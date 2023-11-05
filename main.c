
#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

#define LED_Y NRF_GPIO_PIN_MAP(0, 6)
#define LED_R NRF_GPIO_PIN_MAP(0, 8)
#define LED_G NRF_GPIO_PIN_MAP(1, 9)
#define LED_B NRF_GPIO_PIN_MAP(0, 12)

#define BUTTON NRF_GPIO_PIN_MAP(1, 6)

static const uint32_t leds_list[LEDS_NUMBER] = {LED_Y, LED_R, LED_G, LED_B};

static const unsigned int device_id[LEDS_NUMBER] = {6, 5, 7, 7};

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

void led_on(uint32_t pin)
{
    nrf_gpio_pin_clear(pin);
}

void led_off(uint32_t pin)
{
    nrf_gpio_pin_set(pin);
}

void all_leds_off(void)
{
    for(unsigned int i = 0; i < LEDS_NUMBER; i++) {
        led_off(leds_list[i]);
    }
}

bool button_is_pressed(uint32_t pin)
{
    return (!nrf_gpio_pin_read(pin));
}

bool button_is_released(uint32_t pin)
{
    return (nrf_gpio_pin_read(pin));
}

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

        bsp_board_led_off(led_idx);
        nrf_delay_ms(width);
    }
}


int main(void)
{
    config_pin_as_led(LED_Y);
    config_pin_as_led(LED_R);
    config_pin_as_led(LED_G);
    config_pin_as_led(LED_B);
    config_pin_as_button(BUTTON);

    all_leds_off();

    while(true) {

        for (int i = 0; i < LEDS_NUMBER; i++)
        {
            blink_num_times(i, device_id[i], 500);
            nrf_delay_ms(2000);
        }
    }
}

