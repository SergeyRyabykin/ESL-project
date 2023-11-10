#ifndef CUSTOM_BLINK_H__
#define CUSTOM_BLINK_H__

#include <stdint.h>
#include <stdbool.h>

#define BLINK_FREQ      100 // Hz
#define BLINK_PERIOD_US (1000000/BLINK_FREQ) // us

bool is_time_expired(uint32_t time_ms);
void led_blocked_single_smooth_blink(uint32_t led, uint32_t duration_ms, volatile bool * enable);
bool led_single_smooth_blink(uint32_t led, uint32_t duration_ms, bool enable);
void led_blocked_multiple_smooth_blink(uint32_t led , uint32_t num, uint32_t half_period_ms, volatile bool *enable);
bool led_multiple_smooth_blink(uint32_t led , uint32_t num, uint32_t half_period_ms, bool enable);


#endif // CUSTOM_BLINK_H__