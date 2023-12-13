#include "custom_queue.h"
#include "custom_cmd.h"

#include <stdint.h>

#include "sdk_config.h"
#include "nrfx_pwm.h"

#include "custom_buttons.h"
#include "custom_app.h"
#include "custom_hsv.h"
#include "custom_log.h"
#include "custom_nvm.h"

#define PWM_PLAYBACK_COUNT 1

#include "app_usbd.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_cdc_acm.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define CUSTOM_RGB_STEP (NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE / 255.0)

#define READ_SIZE 1

static char m_rx_buffer[READ_SIZE];

static void usb_ev_handler(app_usbd_class_inst_t const * p_inst,
                           app_usbd_cdc_acm_user_event_t event);

/* Make sure that they don't intersect with LOG_BACKEND_USB_CDC_ACM */
#define CUSTOM_CDC_ACM_COMM_INTERFACE  2
#define CUSTOM_CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN3

#define CUSTOM_CDC_ACM_DATA_INTERFACE  3
#define CUSTOM_CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN4
#define CUSTOM_CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT4

APP_USBD_CDC_ACM_GLOBAL_DEF(custom_usb_cdc_acm,
                            usb_ev_handler,
                            CUSTOM_CDC_ACM_COMM_INTERFACE,
                            CUSTOM_CDC_ACM_DATA_INTERFACE,
                            CUSTOM_CDC_ACM_COMM_EPIN,
                            CUSTOM_CDC_ACM_DATA_EPIN,
                            CUSTOM_CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250);

static nrf_pwm_values_individual_t g_pwm_values;

nrfx_gpiote_in_config_t g_gpiote_cfg = {
    .sense = NRF_GPIOTE_POLARITY_TOGGLE,
    .pull = NRF_GPIO_PIN_PULLUP,
    .is_watcher = false,
    .hi_accuracy = false,
    .skip_gpio_setup = false
};

static custom_app_pwm_indicator_ctx_t g_app_pwm_ind_ctx = {
    .pwm_channel = &g_pwm_values.channel_0
};

// TODO: Save settings to this stucture after color changing through cli
static custom_hsv_ctx_t g_custom_hsv_ctx = {
    .color = {
        .hue = 277,
        .saturation = MAX_SATURATION,
        .value = MAX_BRIGHTNESS
    }
};

static nrfx_pwm_config_t g_pwm_config = NRFX_PWM_DEFAULT_CONFIG;
static nrfx_pwm_t g_pwm_inst = NRFX_PWM_INSTANCE(0);
static nrf_pwm_sequence_t g_pwm_sequence = {
    .values = (nrf_pwm_values_t){.p_individual = (nrf_pwm_values_individual_t *)&g_pwm_values},
    .length = NRF_PWM_VALUES_LENGTH(g_pwm_values),
    .repeats = 100,
};


CUSTOM_QUEUE_INIT(g_custom_queue_output);
static custom_cmd_ctx_t g_custom_cmd_ctx;

// TODO: Find out if there is required any attribute to save strings to Flash.
// TODO: STATIC_ASSERT to test size of all the strings again queie size to avoid print losing
static const char g_cmd_rgb_description[] = "RGB <red> <green> <blue> - sets RGB color. Max value is 255\r\n";
static const char g_cmd_hsv_description[] = "HSV <hue> <saturation> <value> - sets HSV color. Hue in degrees others in percents\r\n";
static const char g_cmd_help_description[] = "help - shows this information\r\n";
static const char g_cmd_error_message[] = "Unknown command\r\n";

static char g_cmd_str[CUSTOM_CMD_STR_LENGTH];
static char g_message[CUSTOM_QUEUE_ROOM_SIZE];

static bool is_number(const char *str)
{
    for(size_t idx = 0; '\0' != str[idx]; idx++) {
        if(!isdigit((int)str[idx])) {
            return false;
        }
    }
    return true;
}

static ret_code_t cmd_hsv_handler(char *str)
{
    unsigned int arg_cnt = 0;
    char *token = strtok(str, " ");
    uint16_t cmd_args[3] = {0};

    while((token = strtok(NULL, " "))) {
        if(is_number(token) && 3 > arg_cnt) {
            int arg = atoi(token);
            switch (arg_cnt) {
                case 0: {
                    if(0 <= arg && 360 > arg) {
                        cmd_args[arg_cnt] = arg;
                    }
                    else {
                        return NRF_ERROR_INVALID_PARAM;
                    }
                }; 
                    break;
                case 1:
                case 2: {
                    if(0 <= arg && 100 >= arg) {
                        cmd_args[arg_cnt] = arg;
                    }
                    else {
                        return NRF_ERROR_INVALID_PARAM;
                    }
                }
                    break;
                
                default:
                    break;
                }
        }
        else {
            return NRF_ERROR_INVALID_PARAM;
        }

        arg_cnt++;
    }
   

    if(3 != arg_cnt) {
        return NRF_ERROR_INVALID_PARAM;
    }

    g_custom_hsv_ctx.color.hue = cmd_args[0];
    g_custom_hsv_ctx.color.saturation = cmd_args[1];
    g_custom_hsv_ctx.color.value = cmd_args[2];

    uint8_t r, g, b;
    custom_hsv_to_rgb(&g_custom_hsv_ctx.color, &r, &g, &b);
    g_pwm_values.channel_1 = CUSTOM_RGB_STEP * r;
    g_pwm_values.channel_2 = CUSTOM_RGB_STEP * g;
    g_pwm_values.channel_3 = CUSTOM_RGB_STEP * b;


    return NRF_SUCCESS;
}

static ret_code_t cmd_rgb_handler(char *str)
{
    unsigned int arg_cnt = 0;
    char *token = strtok(str, " ");
    uint8_t cmd_args[3] = {0};

    while((token = strtok(NULL, " "))) {
        if(is_number(token) && 3 > arg_cnt) {
            int arg = atoi(token);
            if(0 <= arg && 255 >= arg) {
                cmd_args[arg_cnt] = arg;
            }
            else {
                return NRF_ERROR_INVALID_PARAM;
            }
        }
        else {
            return NRF_ERROR_INVALID_PARAM;
        }

        arg_cnt++;
    }
   

    if(3 != arg_cnt) {
        return NRF_ERROR_INVALID_PARAM;
    }


    g_pwm_values.channel_1 = CUSTOM_RGB_STEP * cmd_args[0];
    g_pwm_values.channel_2 = CUSTOM_RGB_STEP * cmd_args[1];
    g_pwm_values.channel_3 = CUSTOM_RGB_STEP * cmd_args[2];

    custom_rgb_to_hsv( &g_custom_hsv_ctx.color, cmd_args[0], cmd_args[1], cmd_args[2]);

    return NRF_SUCCESS;
}

static ret_code_t cmd_help_handler(char *str)
{
    //  TODO: There might be overflow of queue
    for(unsigned int i = 0; i < g_custom_cmd_ctx.number_commands; i++) {
        custom_queue_add(&g_custom_queue_output, (char *)g_custom_cmd_ctx.commands[i].cmd_description);
    }

    return NRF_SUCCESS;
}


// nrfx_err_t custom_pwm_init(unsigned int id, nrf_pwm_values_individual_t values, void (*custom_pwm_handler)(void));

// Function to process call if APP_ERROR_CHECK macros failed
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    LOG("ERROR: %d", info);

    while(true) {
        // Infinite loop
    }
}

void custom_pwm_event_handler(nrfx_pwm_evt_type_t event_type)
{
    if(DOUBLE_CLICK_RELEASED == custom_button_get_state(CUSTOM_BUTTON) && !custom_button_is_processed(CUSTOM_BUTTON)) {
        custom_app_set_pwm_indicator(custom_app_change_state(), &g_app_pwm_ind_ctx);

        // TODO: Find out why the app is hanged up here when loggin is done
        // LOG("Double click");

        if(DEFAULT_MODE == custom_app_get_state()) {
            custom_nvm_save_obj(&g_custom_hsv_ctx.color, sizeof(g_custom_hsv_ctx.color));
        }

        custom_button_process(CUSTOM_BUTTON);
    }

    if(SINGLE_CLICK_PRESSED == custom_button_get_state(CUSTOM_BUTTON) && DEFAULT_MODE != custom_app_get_state()) {
        switch(custom_app_get_state()) {
            case HUE_MODE: custom_hsv_hue_change_by_one(&g_custom_hsv_ctx.color); break;
            case SATURATION_MODE: custom_hsv_saturation_change_by_one(&g_custom_hsv_ctx); break;
            case VALUE_MODE: custom_hsv_value_change_by_one(&g_custom_hsv_ctx); break;
            default: break;
        }

        uint8_t r, g, b;
        custom_hsv_to_rgb(&g_custom_hsv_ctx.color, &r, &g, &b);
        g_pwm_values.channel_1 = CUSTOM_RGB_STEP * r;
        g_pwm_values.channel_2 = CUSTOM_RGB_STEP * g;
        g_pwm_values.channel_3 = CUSTOM_RGB_STEP * b;
    }
    
    custom_app_process_pwm_indicator(&g_app_pwm_ind_ctx);
}

static void usb_ev_handler(app_usbd_class_inst_t const * p_inst,
                           app_usbd_cdc_acm_user_event_t event)
{
    switch (event) {
    case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN: {
        ret_code_t ret;
        ret = app_usbd_cdc_acm_read(&custom_usb_cdc_acm, m_rx_buffer, READ_SIZE);
        UNUSED_VARIABLE(ret);
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE: {
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_TX_DONE: {
        if(!custom_queue_is_empty(&g_custom_queue_output)) {
            custom_queue_get(g_message, &g_custom_queue_output);
            app_usbd_cdc_acm_write(&custom_usb_cdc_acm,
                                    g_message,
                                    strlen(g_message));
        }
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_RX_DONE: {
        ret_code_t ret;
        do {
            /*Get amount of data transfered*/
            size_t size = app_usbd_cdc_acm_rx_size(&custom_usb_cdc_acm);

            (void)size;

            /* It's the simple version of an echo. Note that writing doesn't
             * block execution, and if we have a lot of characters to read and
             * write, some characters can be missed.
             */
            if (m_rx_buffer[0] == '\r' || m_rx_buffer[0] == '\n') {
                // TODO: The problem is that the queue must be used alwyas to get real state of the transmitter.
                if(!custom_queue_is_empty(&g_custom_queue_output)) {
                    custom_queue_add(&g_custom_queue_output, "\r\n");
                }
                else {
                    ret = app_usbd_cdc_acm_write(&custom_usb_cdc_acm, "\r\n", 2);
                }

                if(strlen(g_cmd_str)) {
                    ret = custom_cmd_execute(g_cmd_str, &g_custom_cmd_ctx);
                    if(NRF_SUCCESS != ret) {
                        custom_queue_add(&g_custom_queue_output, g_cmd_error_message);
                    }
                }
                g_cmd_str[0] = '\0';

            }
            else {
                if(!custom_queue_is_empty(&g_custom_queue_output)) {
                    custom_queue_add(&g_custom_queue_output, m_rx_buffer);
                }
                else {
                    ret = app_usbd_cdc_acm_write(&custom_usb_cdc_acm,
                                                m_rx_buffer,
                                                READ_SIZE);
                }

                strncat(g_cmd_str, m_rx_buffer, sizeof(m_rx_buffer));
            }

            /* Fetch data until internal buffer is empty */
            ret = app_usbd_cdc_acm_read(&custom_usb_cdc_acm,
                                        m_rx_buffer,
                                        READ_SIZE);
        } while (ret == NRF_SUCCESS);

        break;
    }
    default:
        break;
    }
}

int main(void)
{
    uint32_t ret = 0;
    // Log initialization
    ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    ret = custom_button_events_init();
    APP_ERROR_CHECK(ret);

    ret = custom_button_event_enable(CUSTOM_BUTTON, &g_gpiote_cfg);
    APP_ERROR_CHECK(ret);

    ret = custom_nvm_init(&g_custom_hsv_ctx.color, sizeof(g_custom_hsv_ctx.color));
    APP_ERROR_CHECK(ret);

    uint8_t r, g, b;
    custom_hsv_to_rgb(&g_custom_hsv_ctx.color, &r, &g, &b);
    g_pwm_values.channel_1 = CUSTOM_RGB_STEP * r;
    g_pwm_values.channel_2 = CUSTOM_RGB_STEP * g;
    g_pwm_values.channel_3 = CUSTOM_RGB_STEP * b;

    ret = nrfx_pwm_init(&g_pwm_inst, &g_pwm_config, custom_pwm_event_handler);
    APP_ERROR_CHECK(ret);
    nrfx_pwm_simple_playback(&g_pwm_inst, &g_pwm_sequence, PWM_PLAYBACK_COUNT, NRFX_PWM_FLAG_LOOP);

    app_usbd_class_inst_t const * custom_class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&custom_usb_cdc_acm);
    ret = app_usbd_class_append(custom_class_cdc_acm);
    APP_ERROR_CHECK(ret);

    ret = custom_cmd_init("RGB", cmd_rgb_handler, g_cmd_rgb_description, &g_custom_cmd_ctx);
    APP_ERROR_CHECK(ret);
    ret = custom_cmd_init("HSV", cmd_hsv_handler, g_cmd_hsv_description, &g_custom_cmd_ctx);
    APP_ERROR_CHECK(ret);
    ret = custom_cmd_init("help", cmd_help_handler, g_cmd_help_description, &g_custom_cmd_ctx);
    APP_ERROR_CHECK(ret);

    while(true) {
        while (app_usbd_event_queue_process()) {
            /* Nothing to do */
        }
        
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }   
}

