
#include <stdint.h>
#include <assert.h>

#include "nrf_delay.h"
#include "nrfx_systick.h"

#include "custom_leds.h"
#include "custom_buttons.h"
#include "custom_blink.h"

// #include "app_timer.h"

#define PERIOD 1000

static const unsigned int device_id[] = {6, 5, 7, 7};
static const uint32_t leds_list[] = CUSTOM_LEDS_LIST;

volatile bool blink_enable = true;

int main(void)
{
    _Static_assert(sizeof(device_id)/sizeof(*device_id) <= sizeof(leds_list)/sizeof(*leds_list), "The number of digits in ID exceeds the number of leds!");

    nrfx_systick_init();

    config_pins_as_leds(sizeof(leds_list)/sizeof(*leds_list), leds_list);
    all_leds_off(sizeof(leds_list)/sizeof(*leds_list), leds_list);

    config_pin_as_button(BUTTON);

    while(true) {
        for (int i = 0; i < sizeof(device_id)/sizeof(*device_id); i++)
        {
            led_blocked_multiple_smooth_blink(leds_list[i], device_id[i], PERIOD, &blink_enable);
            nrf_delay_ms(PERIOD * 2);
        }
    }
}

