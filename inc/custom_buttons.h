#ifndef CUSTOM_BUTTONS_H__
#define CUSTOM_BUTTONS_H__

#include <stdbool.h>
#include "nrf_gpio.h"

#define BUTTON NRF_GPIO_PIN_MAP(1, 6)

void config_pin_as_button(uint32_t pin);
bool button_is_pressed(uint32_t pin);
bool button_is_released(uint32_t pin);


#endif // CUSTOM_BUTTONS_H__