#include "custom_leds.h"

void config_pin_as_led(uint32_t pin)
{
    nrf_gpio_cfg(pin, 
                 GPIO_PIN_CNF_DIR_Output, 
                 GPIO_PIN_CNF_INPUT_Disconnect,
                 GPIO_PIN_CNF_PULL_Pullup,
                 GPIO_PIN_CNF_DRIVE_S0S1,
                 GPIO_PIN_CNF_SENSE_Disabled
                );
}

void led_on(uint32_t pin)
{
    nrf_gpio_pin_clear(pin);
}

void led_off(uint32_t pin)
{
    nrf_gpio_pin_set(pin);
}

void all_leds_off(unsigned int num, const uint32_t leds[num])
{
    for(unsigned int i = 0; i < num; i++) {
        led_off(leds[i]);
    }
}