#include "custom_blink.h"
#include "custom_leds.h"
#include "nrfx_systick.h"
#include "nrf_delay.h"

#define SYSTICK_PART_TIME_US 1000

bool is_time_expired(uint32_t time_ms)
{
    static bool in_action = false;
    static uint32_t time = 0;
    static nrfx_systick_state_t state = {
        .time = 0
    };

    if(!in_action) {
        time = time_ms;
        in_action = true;
        nrfx_systick_get(&state);
    }

    if(nrfx_systick_test(&state, SYSTICK_PART_TIME_US)) {
        if(!--time){
            in_action = false;
        }
        nrfx_systick_get(&state);
    }

    return (!in_action);
}

/**
 * @brief Function to make a single smooth blink
 * 
 * @param[in] led Led to blink
 * @param[in] duration_ms Duration in ms
 * @param[in] enable Flag to enable blink going on
 */
void led_blocked_single_smooth_blink(uint32_t led, uint32_t duration_ms, volatile bool * enable)
{
    uint32_t num_shots = (duration_ms * 1000) / (2 * BLINK_PERIOD_US); // times
    uint32_t duty_cycle_step = BLINK_PERIOD_US / num_shots; // us
    int32_t current_duty_cycle = 0; //us

    bool forward_direction = true;

    nrfx_systick_state_t state;

    do {

        if(0 < current_duty_cycle){
            led_on(led);
        }
        nrfx_systick_get(&state);
        while(!nrfx_systick_test(&state, current_duty_cycle)) {
            ;
        }
        led_off(led);
        nrfx_systick_get(&state);
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

            if(0 >= current_duty_cycle) {
                break;
            }
        }
    } while(true);
}

/**
 * @brief Function to make single smooth blink with led
 * 
 * @param[in] led Led to blink
 * @param[in] duration_ms Duration of one blink in ms
 * @param[in] enable Enables intensity changing
 * @return true If function in process of blinking
 * @return false If function has finished blinking or not blinked yet 
 */
bool led_single_smooth_blink(uint32_t led, uint32_t duration_ms, bool enable)
{
    uint32_t num_shots = (duration_ms * 1000) / (BLINK_PERIOD_US); // times
    uint32_t duty_cycle_step = BLINK_PERIOD_US / num_shots; // us

    static int32_t current_duty_cycle = 0; //us
    static int32_t time_to_wait = 0; //us

    static bool forward_direction = true;
    static bool in_action = false;

    static nrfx_systick_state_t state = {
        .time = 0
    };

    if(!in_action) {
        nrfx_systick_get(&state);
        in_action = true;
    }

    if(nrfx_systick_test(&state, time_to_wait)) {
        
        if(0 < current_duty_cycle){
            led_toggle(led);
        }

        if(enable) {
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

            if(0 >= current_duty_cycle) {
                led_off(led);
                current_duty_cycle = 0;
                time_to_wait = 0;
                forward_direction = true;
                in_action = false;
                return in_action;
            }
        }

        time_to_wait = (led_is_on(led)) ? current_duty_cycle : (BLINK_PERIOD_US - current_duty_cycle);
        nrfx_systick_get(&state);
    }

    return in_action;
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

/**
 * @brief Function to blink led smoothly set number of times with required duration. Function is blocking.
 * 
 * @param[in] led Led to blink
 * @param[in] num Number of times
 * @param[in] half_period_ms Half period value in ms
 * @param[in] enable Flag to enable blink going on
 * @return true If function in process of blinking
 * @return false If function has finished blinking or not blinked yet
 */
bool led_multiple_smooth_blink(uint32_t led , uint32_t num, uint32_t half_period_ms, bool enable)
{
    static unsigned int current_num = 0;
    static bool in_action = false;
    static bool delay = false;

    static nrfx_systick_state_t state = {
        .time = 0
    };

    if(!in_action) {
        nrfx_systick_get(&state);
        in_action = true;
    }

    if(delay) {
        if(is_time_expired(half_period_ms)) {
            delay = false;
        }
        else {
            return in_action;
        }
    }

    if(num) {
        if(led_single_smooth_blink(led, half_period_ms, enable)) {
            return in_action;
        }
        else {
            current_num++;

            if(current_num < num) {
                delay = true;
            }
            else {
                in_action = false;
                delay = false;
                current_num = 0;
            }
        }
    }

    return in_action;
}
