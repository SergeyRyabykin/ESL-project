#ifndef CUSTOM_BUTTONS_H__
#define CUSTOM_BUTTONS_H__

#include <stdbool.h>
#include "nrf_gpio.h"

#define BUTTON NRF_GPIO_PIN_MAP(1, 6)

enum button_state {
    DEFAULT_UNKNOWN,
    SINGLE_CLICK_PRESSED,
    DOUBLE_CLICK_PRESSED,
    SINGLE_CLICK_RELEASED,
    DOUBLE_CLICK_RELEASED
};

typedef struct {
    uint32_t pin;
    volatile enum button_state button_state;
    volatile unsigned int current_click_num;
} custom_button_context_t;

void config_pin_as_button(uint32_t pin);
bool button_is_pressed(uint32_t pin);
bool button_is_released(uint32_t pin);
bool button_te_is_pressed(uint32_t pin);
bool button_te_is_released(uint32_t pin);

#endif // CUSTOM_BUTTONS_H__