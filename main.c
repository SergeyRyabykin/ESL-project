#include <stdint.h>

#include "sdk_config.h"
#include "nrfx_pwm.h"

#include "custom_buttons.h"
#include "custom_app.h"
#include "custom_hsv.h"
#include "custom_log.h"
#include "custom_nvm.h"
#include "custom_app_defines.h"

#include "custom_ble.h"
#include "nrf_sdh_soc.h"

#include "app_util.h"
#include "nrf_dfu_types.h"


// #include "custom_leds.h"
// #include "nrf_delay.h"

#define PWM_PLAYBACK_COUNT 1
#define APP_SOC_OBSERVER_PRIO           1                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define START_ADDR ((uint32_t)(BOOTLOADER_ADDRESS - NRF_DFU_APP_DATA_AREA_SIZE))


static volatile bool g_flash_is_busy = false;
static volatile bool g_color_must_be_saved = false;

static nrf_pwm_values_individual_t g_pwm_values;
static custom_hsv_ctx_t g_custom_hsv_ctx = {
    .color = {
        .hue = 277,
        .saturation = MAX_SATURATION,
        .value = MAX_BRIGHTNESS
    }
};

#ifdef ESTC_USB_CLI_ENABLED
#include "custom_cli.h"
#include "custom_app_types.h"
#include "custom_cli_cmd_list.h"
#include "custom_cmd.h"

static const custom_cmd_ctx_t custom_cmd_ctx = CUSTOM_CMD_INIT_LIST(custom_cli_commands);
static custom_cmd_executor_ctx_t executor_ctx = {
    .cmd = NULL
};

static custom_app_ctx_t g_custom_app_ctx = {
    .custom_hsv_ctx = &g_custom_hsv_ctx,
    .pwm_values = &g_pwm_values,
    .custom_cmd_ctx = &custom_cmd_ctx,
    .custom_print_output = custom_cli_print,
    .executor_ctx = &executor_ctx,
};
#endif

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

// static void led_on(uint8_t led)
// {
//     switch (led) {
//         case 1:
//             g_pwm_values.channel_1 = CUSTOM_RGB_STEP * 255;
//             g_pwm_values.channel_2 = 0;
//             g_pwm_values.channel_3 = 0;
//             break;
//         case 2:
//             g_pwm_values.channel_1 = 0;
//             g_pwm_values.channel_2 = CUSTOM_RGB_STEP * 255;
//             g_pwm_values.channel_3 = 0;
//             break;
//         case 3:
//             g_pwm_values.channel_1 = 0;
//             g_pwm_values.channel_2 = 0;
//             g_pwm_values.channel_3 = CUSTOM_RGB_STEP * 255;
//             break;
//         default:
//             break;
//     }
// }

// Function to process call if APP_ERROR_CHECK macros failed
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    NRF_LOG_INFO("ERROR: %x", id);
}

ret_code_t custom_default_color_rewrite(void)
{
    while(g_flash_is_busy) {
        // Nop
    }

    ret_code_t ret = custom_nvm_save(&g_custom_hsv_ctx.color, sizeof(g_custom_hsv_ctx.color), DEFAULT_HSV_COLOR_ID);

    if(NRF_SUCCESS == ret) {
        g_flash_is_busy = true;
    }
    else {
        return ret;
    }

    // while(g_flash_is_busy){
    //     // Nop
    // }

    // ret = custom_nvm_discard_by_id(DEFAULT_HSV_COLOR_ID);

    // if(NRF_SUCCESS == ret) {
    //     g_flash_is_busy = true;
    // }

    NRF_LOG_INFO("Default color saved!");
    // uintptr_t addr = START_ADDR;
    // for(int i = 0; i < 24; i++) {
    //     NRF_LOG_INFO("%d : %x", i, *((uint8_t *)addr + i));
    // }

    return ret;
}

void custom_pwm_event_handler(nrfx_pwm_evt_type_t event_type)
{
    if(DOUBLE_CLICK_RELEASED == custom_button_get_state(CUSTOM_BUTTON) && !custom_button_is_processed(CUSTOM_BUTTON)) {
        custom_app_set_pwm_indicator(custom_app_change_state(), &g_app_pwm_ind_ctx);

        if(DEFAULT_MODE == custom_app_get_state()) {
            NRF_LOG_INFO("Default mode.");
            g_color_must_be_saved = true;
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




/**@brief Function for initializing power management.
 */
// static void power_management_init(void)
// {
//     ret_code_t err_code;
//     err_code = nrf_pwr_mgmt_init();
//     APP_ERROR_CHECK(err_code);
// }


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
// static void idle_state_handle(void)
// {
//     if (NRF_LOG_PROCESS() == false)
//     {
//         nrf_pwr_mgmt_run();
//     }
// 	LOG_BACKEND_USB_PROCESS();
// }

/**
 * @brief SoftDevice SoC event handler.
 *
 * @param[in] evt_id    SoC event.
 * @param[in] p_context Context.
 */
static void soc_evt_handler(uint32_t evt_id, void * p_context)
{
    // volatile uint8_t *is_busy = (volatile uint8_t *)p_context;
    switch (evt_id)
    {
        case NRF_EVT_FLASH_OPERATION_SUCCESS:
            NRF_LOG_INFO("NVM was written");
            uintptr_t addr = START_ADDR;
            for(int i = 0; i < 10; i++) {
                NRF_LOG_INFO("%x : %x", addr + i*4, *((uint32_t *)addr + i));
            }
            g_flash_is_busy = false;
            break;
        case NRF_EVT_FLASH_OPERATION_ERROR:
            NRF_LOG_INFO("NVM error");
            g_flash_is_busy = false;
            break;
        default:
            break;
    }
}

// static uint32_t leds[] = CUSTOM_LEDS_LIST;

int main(void)
{
    // custom_led_all_pins_config(ARRAY_SIZE(leds), leds);

    // To erase user app non-volatile memory
    custom_button_pin_config(CUSTOM_BUTTON);
    if(custom_button_is_pressed(CUSTOM_BUTTON)) {
        custom_nvm_erase();
        while(custom_button_is_pressed(CUSTOM_BUTTON)) {
            ;
        }
    }

    uint32_t ret = 0;
    // Log initialization
    ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

#ifdef ESTC_USB_CLI_ENABLED
    ret = custom_cli_init(&g_custom_app_ctx);
    APP_ERROR_CHECK(ret);
#endif

    ret = custom_button_events_init();
    APP_ERROR_CHECK(ret);

    ret = custom_button_event_enable(CUSTOM_BUTTON, &g_gpiote_cfg);
    APP_ERROR_CHECK(ret);

    custom_ble_init();
    NRF_SDH_SOC_OBSERVER(m_soc_observer, APP_SOC_OBSERVER_PRIO, soc_evt_handler, NULL); // Move to custom_nvm module
    // (void)soc_evt_handler;

    uintptr_t saved_object = custom_nvm_find(DEFAULT_HSV_COLOR_ID);
    if(saved_object) {
        g_custom_hsv_ctx.color.hue = ((custom_hsv_t *)saved_object)->hue;
        g_custom_hsv_ctx.color.saturation = ((custom_hsv_t *)saved_object)->saturation;
        g_custom_hsv_ctx.color.value = ((custom_hsv_t *)saved_object)->value;
    }
    // else {
    //     while(g_flash_is_busy) {
    //         // Nop
    //     }
    //     ret = custom_nvm_save(&g_custom_hsv_ctx.color, sizeof(g_custom_hsv_ctx.color), DEFAULT_HSV_COLOR_ID);
    //     APP_ERROR_CHECK(ret);
    //     if(NRF_SUCCESS == ret) {
    //         g_flash_is_busy = true;
    //     }
    // }

    uint8_t r, g, b;
    custom_hsv_to_rgb(&g_custom_hsv_ctx.color, &r, &g, &b);
    g_pwm_values.channel_1 = CUSTOM_RGB_STEP * r;
    g_pwm_values.channel_2 = CUSTOM_RGB_STEP * g;
    g_pwm_values.channel_3 = CUSTOM_RGB_STEP * b;

    ret = nrfx_pwm_init(&g_pwm_inst, &g_pwm_config, custom_pwm_event_handler);
    APP_ERROR_CHECK(ret);
    nrfx_pwm_simple_playback(&g_pwm_inst, &g_pwm_sequence, PWM_PLAYBACK_COUNT, NRFX_PWM_FLAG_LOOP);

    // power_management_init();
    

    while(true) {

#ifdef ESTC_USB_CLI_ENABLED
        if(executor_ctx.cmd) {
            ret = executor_ctx.cmd->cmd_execute(executor_ctx.cmd_str, executor_ctx.context);
            if(NRF_SUCCESS != ret) {
                custom_cli_print("Arguments error!\n\r");
            }
            executor_ctx.cmd = NULL;
        }
#endif        
        if(g_color_must_be_saved) {
            ret = custom_default_color_rewrite();
            APP_ERROR_CHECK(ret);
            g_color_must_be_saved = false;
        }

        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
        // idle_state_handle();
    }   
}

