#ifndef CUSTOM_PINS_H__
#define CUSTOM_PINS_H__

#include "nrf_gpio.h"

#define LED_Y NRF_GPIO_PIN_MAP(0, 6)
#define LED_R NRF_GPIO_PIN_MAP(0, 8)
#define LED_G NRF_GPIO_PIN_MAP(1, 9)
#define LED_B NRF_GPIO_PIN_MAP(0, 12)

#define CUSTOM_LEDS_LIST {LED_Y, LED_R, LED_G, LED_B}

void custom_led_pin_config(uint32_t pin);
void custom_led_all_pins_config(unsigned int num, const uint32_t pins[num]);
void custom_led_on(uint32_t pin);
void custom_led_off(uint32_t pin);
void custom_led_toggle(uint32_t pin);
void custom_leds_off_all(unsigned int num, const uint32_t leds[num]);
bool custom_led_is_on(uint32_t pin);

#endif // CUSTOM_PINS_H__