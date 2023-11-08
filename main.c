
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

static const unsigned int device_id[] = {6, 5, 7, 7};
static const uint32_t leds_list[] = CUSTOM_LEDS_LIST;

_Static_assert(sizeof(device_id)/sizeof(*device_id) <= sizeof(leds_list)/sizeof(*leds_list), "The number of digits in ID exceeds the number of leds!");


static volatile bool blink_enable = true;
APP_TIMER_DEF(debounce_timer);

static volatile unsigned int click_num = 0;


void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    led_on(LED_R);
}


void custom_debounce_timer_timeout_handler(void *context)
{
    if(!nrfx_gpiote_in_is_set(BUTTON)) {
        click_num++;
    }

    if(2 <= click_num) {
        blink_enable = !blink_enable;
        click_num = 0;
    }
}

void custom_buttom_toggle_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    APP_ERROR_CHECK(app_timer_start(debounce_timer, APP_TIMER_TICKS(DEBOUNCE_TIME_MS), NULL));
}

int main(void)
{
    uint32_t ret = 0;

    config_pins_as_leds(sizeof(leds_list)/sizeof(*leds_list), leds_list);
    all_leds_off(sizeof(leds_list)/sizeof(*leds_list), leds_list);

    config_pin_as_button(BUTTON);

    // Systick initialization
    nrfx_systick_init();

    // RTC initialization
    ret = app_timer_init();
    APP_ERROR_CHECK(ret);

    // Debounce timer creation
    ret = app_timer_create(&debounce_timer, APP_TIMER_MODE_SINGLE_SHOT, custom_debounce_timer_timeout_handler);
    APP_ERROR_CHECK(ret);

    // GPIOTE initialization
    if(!nrfx_gpiote_is_init()) {
        ret = nrfx_gpiote_init();
        APP_ERROR_CHECK(ret);
    }

    ret = nrfx_gpiote_in_init(BUTTON, &(nrfx_gpiote_in_config_t)NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true), custom_buttom_toggle_handler);
    APP_ERROR_CHECK(ret);

    nrfx_gpiote_in_event_enable(BUTTON, true);

    // Log initialization
    NRF_LOG_INIT(NULL);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    while(true) {
        for (int i = 0; i < sizeof(device_id)/sizeof(*device_id); i++)
        {
            led_blocked_multiple_smooth_blink(leds_list[i], device_id[i], PERIOD, &blink_enable);
            nrf_delay_ms(PERIOD * 2);
        }
    }
}

