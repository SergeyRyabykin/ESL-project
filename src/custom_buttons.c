#include "custom_buttons.h"
#include "nrfx_gpiote.h"

void config_pin_as_button(uint32_t pin)
{
    nrf_gpio_cfg(pin, 
                 GPIO_PIN_CNF_DIR_Input, 
                 GPIO_PIN_CNF_INPUT_Connect,
                 GPIO_PIN_CNF_PULL_Pullup,
                 GPIO_PIN_CNF_DRIVE_S0S1,
                 GPIO_PIN_CNF_SENSE_Low
                );
}

bool button_is_pressed(uint32_t pin)
{
    return (!nrf_gpio_pin_read(pin));
}

bool button_is_released(uint32_t pin)
{
    return (nrf_gpio_pin_read(pin));
}

bool button_te_is_pressed(uint32_t pin)
{
    return !nrfx_gpiote_in_is_set(pin);
}

bool button_te_is_released(uint32_t pin)
{
    return nrfx_gpiote_in_is_set(pin);
}