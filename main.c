
#include <stdint.h>
#include <assert.h>

#include "sdk_config.h"
#include "nrf_delay.h"
#include "nrfx_systick.h"
#include "app_timer.h"
#include "nrfx_gpiote.h"

#include "custom_leds.h"
#include "custom_buttons.h"
#include "custom_blink.h"


#define PERIOD 1000

static const unsigned int device_id[] = {6, 5, 7, 7};
static const uint32_t leds_list[] = CUSTOM_LEDS_LIST;

volatile bool blink_enable = true;
APP_TIMER_DEF(custom_timer);


void custom_timer_timeout_handler(void *context)
{
    led_on(LED_Y);
}

void custom_buttom_toggle_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    blink_enable = !blink_enable;
}

int main(void)
{
    _Static_assert(sizeof(device_id)/sizeof(*device_id) <= sizeof(leds_list)/sizeof(*leds_list), "The number of digits in ID exceeds the number of leds!");
    
    config_pins_as_leds(sizeof(leds_list)/sizeof(*leds_list), leds_list);
    all_leds_off(sizeof(leds_list)/sizeof(*leds_list), leds_list);

    config_pin_as_button(BUTTON);

    // Systick initialization
    nrfx_systick_init();

    // RTC initialization
    if(NRF_SUCCESS != app_timer_init()) {
        // TODO: Organize LOG out of error
        led_on(LED_R);
    }


    if(NRF_SUCCESS != app_timer_create(&custom_timer, APP_TIMER_MODE_REPEATED, custom_timer_timeout_handler)) {
    // if(NRF_SUCCESS != app_timer_create(&custom_timer, APP_TIMER_MODE_SINGLE_SHOT, custom_timer_timeout_handler)) {
        // TODO: Organize LOG out of error
        led_on(LED_R);
    }

    if(NRF_SUCCESS != app_timer_start(custom_timer, APP_TIMER_TICKS(700), &(void *){NULL})) {
        // TODO: Organize LOG out of error
        led_on(LED_R);
    }

    // GPIOTE initialization
    if(!nrfx_gpiote_is_init()) {
        if(NRFX_SUCCESS != nrfx_gpiote_init()) {
            ; // TODO: Organize LOG out of error
        }
    }

    nrfx_gpiote_in_init(BUTTON, &(nrfx_gpiote_in_config_t)NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true), custom_buttom_toggle_handler);
    nrfx_gpiote_in_event_enable(BUTTON, true);

    
    while(true) {
        for (int i = 0; i < sizeof(device_id)/sizeof(*device_id); i++)
        {
            led_blocked_multiple_smooth_blink(leds_list[i], device_id[i], PERIOD, &blink_enable);
            nrf_delay_ms(PERIOD * 2);
        }
    }
}

