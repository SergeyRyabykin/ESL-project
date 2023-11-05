#ifndef CUSTOM_PINS_H__
#define CUSTOM_PINS_H__

#include "nrf_gpio.h"

#define LED_Y NRF_GPIO_PIN_MAP(0, 6)
#define LED_R NRF_GPIO_PIN_MAP(0, 8)
#define LED_G NRF_GPIO_PIN_MAP(1, 9)
#define LED_B NRF_GPIO_PIN_MAP(0, 12)

#define CUSTOM_LEDS_LIST {LED_Y, LED_R, LED_G, LED_B}

void config_pin_as_led(uint32_t pin);
void led_on(uint32_t pin);
void led_off(uint32_t pin);
void all_leds_off(unsigned int num, const uint32_t leds[num]);

#endif // CUSTOM_PINS_H__