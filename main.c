
#include <stdint.h>
#include <assert.h>
#include "nrf_delay.h"
#include "custom_leds.h"
#include "custom_buttons.h"


static const unsigned int device_id[] = {6, 5, 7, 7};
static const uint32_t leds_list[] = CUSTOM_LEDS_LIST;

/**
 * @brief Function to blink led set number of times with required width
 * 
 * @param led_idx Index of led
 * @param num  Number of times
 * @param width Half period value in ms
 */
static void blink_num_times(uint32_t led_idx , uint32_t num, uint32_t width)
{
    for(unsigned int i = 0; i < num; i++){
        while(button_is_released(BUTTON)) {
            ;
        }

        led_on(leds_list[led_idx]);
        nrf_delay_ms(width);

        while(button_is_released(BUTTON)) {
            ;
        }

        led_off(leds_list[led_idx]);
        nrf_delay_ms(width);
    }
}


int main(void)
{
    _Static_assert(sizeof(device_id)/sizeof(*device_id) <= sizeof(leds_list)/sizeof(*leds_list), "The number of digits in ID exceeds the number of leds!");

    config_pin_as_led(LED_Y);
    config_pin_as_led(LED_R);
    config_pin_as_led(LED_G);
    config_pin_as_led(LED_B);
    config_pin_as_button(BUTTON);

    all_leds_off(sizeof(leds_list)/sizeof(*leds_list), leds_list);



    while(true) {

        for (int i = 0; i < sizeof(device_id)/sizeof(*device_id); i++)
        {
            blink_num_times(i, device_id[i], 500);
            nrf_delay_ms(2000);
        }
    }
}

