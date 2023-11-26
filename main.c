
#include <stdint.h>

#include "sdk_config.h"
#include "nrf_delay.h"
#include "nrfx_pwm.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"

#include "custom_buttons.h"
#include "custom_app.h"
#include "custom_hsv.h"

#define PWM_PLAYBACK_COUNT 1

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


// Function to process call if APP_ERROR_CHECK macros failed
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    NRF_LOG_INFO("ERROR: %d", info);

    LOG_BACKEND_USB_PROCESS();
    NRF_LOG_PROCESS();

    while(true) {
        // Infinite loop
    }
}

void custom_pwm_event_handler(nrfx_pwm_evt_type_t event_type)
{
    if(DOUBLE_CLICK_RELEASED == custom_button_get_state(CUSTOM_BUTTON) && !custom_button_is_processed(CUSTOM_BUTTON)) {
        custom_app_set_pwm_indicator(custom_app_change_state(), &g_app_pwm_ind_ctx);
        custom_button_processed(CUSTOM_BUTTON, true);
    }

    if(SINGLE_CLICK_PRESSED == custom_button_get_state(CUSTOM_BUTTON) && DEFAULT_MODE != custom_app_get_state()) {
        switch(custom_app_get_state()) {
            case HUE_MODE: custom_hsv_hue_change_by_one(&g_custom_hsv_ctx.color); break;
            case SATURATION_MODE: custom_hsv_saturation_change_by_one(&g_custom_hsv_ctx); break;
            case VALUE_MODE: custom_hsv_value_change_by_one(&g_custom_hsv_ctx); break;
            default: break;
        }

        custom_hsv_to_rgb(g_custom_hsv_ctx.color, &g_pwm_values.channel_1, &g_pwm_values.channel_2, &g_pwm_values.channel_3);
    }
    
    custom_app_process_pwm_indicator(&g_app_pwm_ind_ctx);


}

int main(void)
{
    uint32_t ret = 0;

    // Log initialization
    NRF_LOG_INIT(NULL);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    ret = custom_button_events_init();
    APP_ERROR_CHECK(ret);

    ret = custom_button_event_enable(CUSTOM_BUTTON, &g_gpiote_cfg);
    APP_ERROR_CHECK(ret);
    
    custom_hsv_to_rgb(g_custom_hsv_ctx.color, &g_pwm_values.channel_1, &g_pwm_values.channel_2, &g_pwm_values.channel_3);

    ret = nrfx_pwm_init(&g_pwm_inst, &g_pwm_config, custom_pwm_event_handler);
    APP_ERROR_CHECK(ret);
    nrfx_pwm_simple_playback(&g_pwm_inst, &g_pwm_sequence, PWM_PLAYBACK_COUNT, NRFX_PWM_FLAG_LOOP);


    while(true) {
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }   
}

