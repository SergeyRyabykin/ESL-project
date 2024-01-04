#include <stdint.h>

#include "sdk_config.h"
#include "nrfx_pwm.h"

#include "custom_buttons.h"
#include "custom_app.h"
#include "custom_hsv.h"
#include "custom_log.h"
#include "custom_nvm.h"
#include "custom_app_defines.h"

#define PWM_PLAYBACK_COUNT 1

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
static custom_app_ctx_t g_custom_app_ctx = {
    .custom_hsv_ctx = &g_custom_hsv_ctx,
    .pwm_values = &g_pwm_values,
    .custom_cmd_ctx = &custom_cmd_ctx,
    .custom_print_output = custom_cli_print,
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

        if(DEFAULT_MODE == custom_app_get_state()) {
            ret_code_t ret = custom_nvm_save(&g_custom_hsv_ctx.color, sizeof(g_custom_hsv_ctx.color), DEFAULT_HSV_COLOR_ID);
            APP_ERROR_CHECK(ret);
            ret = custom_nvm_discard_by_id(DEFAULT_HSV_COLOR_ID);
            APP_ERROR_CHECK(ret);
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

int main(void)
{
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

    // To erase user app non-volatile memory
    if(custom_button_te_is_pressed(CUSTOM_BUTTON)) {
        custom_nvm_erase();
        while(custom_button_te_is_pressed(CUSTOM_BUTTON)) {
            ;
        }
    }

    uintptr_t saved_object = custom_nvm_find(DEFAULT_HSV_COLOR_ID);
    if(saved_object) {
        g_custom_hsv_ctx.color.hue = ((custom_hsv_t *)saved_object)->hue;
        g_custom_hsv_ctx.color.saturation = ((custom_hsv_t *)saved_object)->saturation;
        g_custom_hsv_ctx.color.value = ((custom_hsv_t *)saved_object)->value;
    }
    else {
        ret = custom_nvm_save(&g_custom_hsv_ctx.color, sizeof(g_custom_hsv_ctx.color), DEFAULT_HSV_COLOR_ID);
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
    
    while(true) {
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }   
}

