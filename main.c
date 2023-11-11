
#include <stdint.h>
#include <assert.h>

#include "sdk_config.h"
#include "nrf_delay.h"
#include "nrfx_systick.h"
#include "app_timer.h"
#include "nrfx_gpiote.h"


#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"

#include "custom_leds.h"
#include "custom_buttons.h"
#include "custom_blink.h"

#define PERIOD 1000
#define DEBOUNCE_TIME_MS 10
#define DOUBLE_CLICK_TIME_MS 500

static const unsigned int device_id[] = {6, 5, 7, 7};
static const uint32_t leds_list[] = CUSTOM_LEDS_LIST;

_Static_assert(ARRAY_SIZE(device_id) <= ARRAY_SIZE(leds_list), "The number of digits in ID exceeds the number of leds!");

static volatile unsigned int click_num = 0;
static volatile bool blink_enable = true;
APP_TIMER_DEF(debounce_timer);
APP_TIMER_DEF(double_click_timer);


void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    led_on(LED_R);
    NRF_LOG_INFO("ERROR: %d", info);
    while(true) {
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }
}

void custom_double_click_timer_timeout_handler(void *context)
{
    click_num = 0;
}

void custom_debounce_timer_timeout_handler(void *context)
{
    if(!nrfx_gpiote_in_is_set(BUTTON)) {
        click_num++;
    }

    if(2 == click_num) {
        blink_enable = !blink_enable;
        click_num = 0;
        NRF_LOG_INFO("%s", blink_enable ? "Running":"Stopped");
    }
}

void custom_button_toggle_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    uint32_t ret = 0;

    ret = app_timer_stop(debounce_timer);
    APP_ERROR_CHECK(ret);
    ret = app_timer_start(debounce_timer, APP_TIMER_TICKS(DEBOUNCE_TIME_MS), NULL);
    APP_ERROR_CHECK(ret);
    ret = app_timer_start(double_click_timer, APP_TIMER_TICKS(DOUBLE_CLICK_TIME_MS), NULL);
    APP_ERROR_CHECK(ret);
}


void blink_leds_according_to_id(uint32_t period, bool enable)
{
    static bool is_blinking = true;

    if(is_blinking) {
        static unsigned int led_idx = 0;
        if(!led_multiple_smooth_blink(leds_list[led_idx], device_id[led_idx], period, enable)) {
            is_blinking = false;
            led_idx++;

            if(led_idx >= ARRAY_SIZE(leds_list)) {
                led_idx = 0;
            }
        }
    }
    else {
        is_blinking = is_time_expired(period * 4);
    }
}


int main(void)
{
    uint32_t ret = 0;

     // Log initialization
    NRF_LOG_INIT(NULL);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    config_pins_as_leds(ARRAY_SIZE(leds_list), leds_list);
    all_leds_off(ARRAY_SIZE(leds_list), leds_list);

    config_pin_as_button(BUTTON);

    // Systick initialization
    nrfx_systick_init();

    // RTC initialization
    ret = app_timer_init();
    APP_ERROR_CHECK(ret);

    // Debounce timer creation
    ret = app_timer_create(&debounce_timer, APP_TIMER_MODE_SINGLE_SHOT, custom_debounce_timer_timeout_handler);
    APP_ERROR_CHECK(ret);

    // Debounce timer creation
    ret = app_timer_create(&double_click_timer, APP_TIMER_MODE_SINGLE_SHOT, custom_double_click_timer_timeout_handler);
    APP_ERROR_CHECK(ret);


    // GPIOTE initialization
    if(!nrfx_gpiote_is_init()) {
        ret = nrfx_gpiote_init();
        APP_ERROR_CHECK(ret);
    }

    nrfx_gpiote_in_config_t user_gpiote_cfg = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    user_gpiote_cfg.pull = NRF_GPIO_PIN_PULLUP;
    ret = nrfx_gpiote_in_init(BUTTON, &user_gpiote_cfg, custom_button_toggle_event_handler);
    APP_ERROR_CHECK(ret);

    nrfx_gpiote_in_event_enable(BUTTON, true);

    while(true) {
        blink_leds_according_to_id(PERIOD, blink_enable);

        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }   
}

