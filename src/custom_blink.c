#include "custom_blink.h"
#include "custom_leds.h"
#include "nrfx_systick.h"
#include "nrf_delay.h"


/**
 * @brief Function to make a single smooth blink
 * 
 * @param[in] led Led to blink
 * @param[in] duration_ms Duration in ms
 * @param[in] enable Flag to enable blink going on
 */
// TODO: Think about defence of the enable pointer from value changing
void led_blocked_single_smooth_blink(uint32_t led, uint32_t duration_ms, volatile bool * enable)
{
    // TODO: Have a closer look on the dut_cycle_step calculation because of deviding on 2 the duration.
    uint32_t num_shots = (duration_ms * 1000) / (2 * BLINK_PERIOD_US); // times
    uint32_t duty_cycle_step = BLINK_PERIOD_US / num_shots; // us
    int32_t current_duty_cycle = duty_cycle_step; //us

    bool forward_direction = true;

    nrfx_systick_state_t state;

    while(0 <= current_duty_cycle) {

        led_on(led);
        nrfx_systick_get(&state);
        while(!nrfx_systick_test(&state, current_duty_cycle)) {
            ;
        }
        led_off(led);
        nrfx_systick_get(&state);
        // TODO: Think about if current_duty_cycle is more then BLINK_PERIOD_US
        while(!nrfx_systick_test(&state, (BLINK_PERIOD_US - current_duty_cycle))) {
            ;
        }

        if(*enable) {
            if(forward_direction) {
                if (current_duty_cycle >= BLINK_PERIOD_US){
                    forward_direction = false;
                }
            }

            if(forward_direction) {
                current_duty_cycle += duty_cycle_step;
            }
            else {
                current_duty_cycle -= duty_cycle_step;
            }
        }
    }
}

/**
 * @brief Function to blink led smoothly set number of times with required duration
 * 
 * @param[in] led_idx Led to blink
 * @param[in] num  Number of times
 * @param[in] half_period_ms Half period value in ms
 * @param[in] enable Flag to enable blink going on
 */
void led_blocked_multiple_smooth_blink(uint32_t led , uint32_t num, uint32_t half_period_ms, volatile bool *enable)
{
    for(unsigned int i = 0; i < num; i++){
        led_blocked_single_smooth_blink(led, half_period_ms, enable);
        nrf_delay_ms(half_period_ms);
    }
}