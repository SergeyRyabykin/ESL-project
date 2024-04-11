#include <stdint.h>
#include <assert.h>

#include "sdk_config.h"
#include "nrfx_pwm.h"

#include "custom_buttons.h"
#include "custom_app.h"
#include "custom_hsv.h"
#include "custom_log.h"
#include "custom_record.h"
#include "custom_app_defines.h"
#include "custom_app_types.h"
#include "custom_cmd_list.h"
#include "custom_cmd.h"

#include "custom_ble.h"
#include "nrf_delay.h"


#define PWM_PLAYBACK_COUNT 1
static void notify_color_changed(void * context);

static custom_record_t default_record = {
    .record.file_id = FILE_ID,
    .record.key = DEFAULT_HSV_COLOR_ID
};

static volatile bool g_default_must_be_updated = false;

static nrf_pwm_values_individual_t g_pwm_values;
static custom_hsv_ctx_t g_custom_hsv_ctx = {
    .color = {
        .hue = 277,
        .saturation = MAX_SATURATION,
        .value = MAX_BRIGHTNESS
    }
};

static const custom_cmd_t custom_ble_cmd_set[] = {
    custom_commands[0],
    custom_commands[1],
    custom_commands[2],
};

static const custom_cmd_ctx_t g_custom_ble_cmd_ctx = CUSTOM_CMD_INIT_LIST(custom_ble_cmd_set);
static custom_cmd_executor_ctx_t g_executor_ctx = {
    .cmd = NULL
};

#ifdef ESTC_USB_CLI_ENABLED
#include "custom_cli.h"
static const custom_cmd_ctx_t g_custom_cli_cmd_ctx = CUSTOM_CMD_INIT_LIST(custom_commands);
static custom_app_ctx_t g_custom_app_cli_ctx = {
    .custom_app_callback = notify_color_changed,
    .default_record = &default_record,
    .custom_hsv_ctx = &g_custom_hsv_ctx,
    .pwm_values = &g_pwm_values,
    .custom_cmd_ctx = &g_custom_cli_cmd_ctx,
    .custom_print_output = custom_cli_print,
    .executor_ctx = &g_executor_ctx,
};
#endif

static custom_app_ctx_t g_custom_app_ble_ctx = {
    .custom_app_callback = notify_color_changed,
    .default_record = &default_record,
    .custom_hsv_ctx = &g_custom_hsv_ctx,
    .pwm_values = &g_pwm_values,
    .custom_cmd_ctx = &g_custom_ble_cmd_ctx,
    .custom_print_output = custom_ble_notify_message,
    .executor_ctx = &g_executor_ctx,
};

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

static nrfx_pwm_config_t g_pwm_config = NRFX_PWM_DEFAULT_CONFIG;
static nrfx_pwm_t g_pwm_inst = NRFX_PWM_INSTANCE(0);
static nrf_pwm_sequence_t g_pwm_sequence = {
    .values = (nrf_pwm_values_t){.p_individual = (nrf_pwm_values_individual_t *)&g_pwm_values},
    .length = NRF_PWM_VALUES_LENGTH(g_pwm_values),
    .repeats = 100,
};

static void notify_color_changed(void * context)
{
    char message[20] = "\0";

    NRF_STATIC_ASSERT(sizeof(message) >= sizeof("H:360 S:100 V:100"), "The array for notification too short");

    sprintf(message, "H:%d S:%d V:%d", g_custom_hsv_ctx.color.hue, g_custom_hsv_ctx.color.saturation, g_custom_hsv_ctx.color.value);
    custom_ble_notify_color_changed(message, sizeof(message));
}

// Function to process call if APP_ERROR_CHECK macros failed
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    NRF_LOG_INFO("ERROR: %x", id);
}

void custom_pwm_event_handler(nrfx_pwm_evt_type_t event_type)
{
    if(DOUBLE_CLICK_RELEASED == custom_button_get_state(CUSTOM_BUTTON) && !custom_button_is_processed(CUSTOM_BUTTON)) {
        custom_app_set_pwm_indicator(custom_app_change_state(), &g_app_pwm_ind_ctx);

        if(DEFAULT_MODE == custom_app_get_state()) {
            g_default_must_be_updated = true;
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

    if(SINGLE_CLICK_RELEASED == custom_button_get_state(CUSTOM_BUTTON) && !custom_button_is_processed(CUSTOM_BUTTON)) {
        notify_color_changed(NULL);
        custom_button_process(CUSTOM_BUTTON);
    }
    
    custom_app_process_pwm_indicator(&g_app_pwm_ind_ctx);
}

void custom_ble_change_color(void *cmd_str)
{
    if(strlen(cmd_str)) {
        NRF_LOG_INFO("%s", cmd_str);
        ret_code_t ret = custom_cmd_get_cmd_executor(g_custom_app_ble_ctx.executor_ctx, cmd_str, &g_custom_ble_cmd_ctx, &g_custom_app_ble_ctx);
        if(NRF_SUCCESS != ret) {
            g_custom_app_ble_ctx.custom_print_output("Unknown command\n\r");
            NRF_LOG_INFO("UNKNOWN COMMAND");
        }
    }
}

int main(void)
{
    uint32_t ret = 0;

    // Log initialization
    ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    custom_button_pin_config(CUSTOM_BUTTON);

    custom_ble_init(&g_custom_hsv_ctx.color, custom_ble_change_color);
    ret = custom_record_storage_init();
    APP_ERROR_CHECK(ret);

    // To erase user app non-volatile memory
    if(custom_button_is_pressed(CUSTOM_BUTTON)) {
        custom_ble_delete_peers();
        ret = custom_record_erase(FILE_ID);
        APP_ERROR_CHECK(ret);

        while(custom_button_is_pressed(CUSTOM_BUTTON)) {
            ;
        }

        nrf_delay_ms(10);
    }

#ifdef ESTC_USB_CLI_ENABLED
    ret = custom_cli_init(&g_custom_app_cli_ctx);
    APP_ERROR_CHECK(ret);
#endif

    ret = custom_button_events_init();
    APP_ERROR_CHECK(ret);

    ret = custom_button_event_enable(CUSTOM_BUTTON, &g_gpiote_cfg);
    APP_ERROR_CHECK(ret);

    ret = custom_record_read(&default_record, &g_custom_hsv_ctx.color);
    APP_ERROR_CHECK(ret);

    if(NRF_SUCCESS != ret) {
        ret = custom_record_save(&default_record, &g_custom_hsv_ctx.color, sizeof(g_custom_hsv_ctx.color));
        APP_ERROR_CHECK(ret);
    }

    uint8_t r, g, b;
    custom_hsv_to_rgb(&g_custom_hsv_ctx.color, &r, &g, &b);
    g_pwm_values.channel_1 = CUSTOM_RGB_STEP * r;
    g_pwm_values.channel_2 = CUSTOM_RGB_STEP * g;
    g_pwm_values.channel_3 = CUSTOM_RGB_STEP * b;

    ret = nrfx_pwm_init(&g_pwm_inst, &g_pwm_config, custom_pwm_event_handler);
    APP_ERROR_CHECK(ret);
    nrfx_pwm_simple_playback(&g_pwm_inst, &g_pwm_sequence, PWM_PLAYBACK_COUNT, NRFX_PWM_FLAG_LOOP);

    notify_color_changed(NULL);

    while(true) {

        if(g_executor_ctx.cmd) {
            ret = g_executor_ctx.cmd->cmd_execute(g_executor_ctx.cmd_str, g_executor_ctx.context);
            custom_app_ctx_t *ctx_ptr = (custom_app_ctx_t*)(g_executor_ctx.context);
            if(NRF_SUCCESS != ret) {
                ctx_ptr->custom_print_output("Arguments error\n\r");
                NRF_LOG_INFO("ARGUMENTS_ERROR");
            }
            else {
                ctx_ptr->custom_print_output("Success\n\r");
            }
            g_executor_ctx.cmd = NULL;
        }

        if(g_default_must_be_updated) {
            ret_code_t ret = custom_record_update(&default_record, &g_custom_hsv_ctx.color, sizeof(g_custom_hsv_ctx.color));
            APP_ERROR_CHECK(ret);
            g_default_must_be_updated = false;
        }

        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }   
}

