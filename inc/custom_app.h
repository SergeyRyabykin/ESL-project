#ifndef CUSTOM_APP_H__
#define CUSTOM_APP_H__

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "sdk_config.h"


#define PERIOD_MS 1000

#define PWM_BASE_CLOCK_HZ (16000000 / (1 << NRFX_PWM_DEFAULT_CONFIG_BASE_CLOCK)) // Have not to be more then 1 MHz to get correct outcomes. See value NRFX_PWM_DEFAULT_CONFIG_BASE_CLOCK in sdk_config.h

#define PWM_BASE_PERIOD_US (1000000 / PWM_BASE_CLOCK_HZ)
#define PWM_TIME_BETWEEN_HANDLERS_US (NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE * PWM_BASE_PERIOD_US)

#define APP_IND_DEFAULT_INC 0
#define APP_IND_HUE_INC     (PERIOD_MS / 2 * 1000 / PWM_TIME_BETWEEN_HANDLERS_US)
#define APP_IND_SATUR_INC   (PERIOD_MS * 1000 / PWM_TIME_BETWEEN_HANDLERS_US)
#define APP_IND_VALUE_INC   NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE

_Static_assert(PWM_BASE_CLOCK_HZ <= 1000000 , "The PWM base clock have to be less or equal of 1 MHz frequency!");

typedef enum {
    DEFAULT_MODE,
    HUE_MODE,
    SATURATION_MODE,
    VALUE_MODE,
    MODE_NUMBER
} custom_app_state_t;

typedef struct {
    volatile bool forward_direct;
    volatile uint16_t pwm_duty_step;
    uint16_t *pwm_channel;
} custom_app_pwm_indicator_ctx_t;

custom_app_state_t custom_app_change_state(void);
custom_app_state_t custom_app_get_state(void);
void custom_app_set_pwm_indicator(const custom_app_state_t state, custom_app_pwm_indicator_ctx_t *context);
void custom_app_process_pwm_indicator(custom_app_pwm_indicator_ctx_t *context);

#endif // CUSTOM_APP_H__