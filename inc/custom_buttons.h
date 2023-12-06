#ifndef CUSTOM_BUTTONS_H__
#define CUSTOM_BUTTONS_H__

#include <stdbool.h>
#include "nrf_gpio.h"
#include "nrfx_gpiote.h"

#define CUSTOM_BUTTON NRF_GPIO_PIN_MAP(1, 6)
#define CUSTOM_BUTTONS_LIST {CUSTOM_BUTTON}

#define MAX_NUM_CUSTOM_BUTTONS 1

typedef enum {
    DEFAULT_UNKNOWN,
    SINGLE_CLICK_PRESSED,
    DOUBLE_CLICK_PRESSED,
    SINGLE_CLICK_RELEASED,
    DOUBLE_CLICK_RELEASED
} custom_button_state_t;

void custom_button_pin_config(uint32_t pin);
bool custom_button_is_pressed(uint32_t pin);
bool custom_button_is_released(uint32_t pin);
bool custom_button_te_is_pressed(uint32_t pin);
bool custom_button_te_is_released(uint32_t pin);

ret_code_t custom_button_events_init(void);
ret_code_t custom_button_event_enable(const uint32_t pin, const nrfx_gpiote_in_config_t *gpiote_cfg);
bool custom_button_process(const uint32_t pin);
custom_button_state_t custom_button_get_state(const uint32_t pin);
bool custom_button_is_processed(const uint32_t pin);

#endif // CUSTOM_BUTTONS_H__