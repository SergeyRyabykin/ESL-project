#ifndef CUSTOM_APP_DEFINES_H__
#define CUSTOM_APP_DEFINES_H__

#include "sdk_config.h"
#include "custom_hsv.h"
#include "custom_cmd.h"
#include "custom_queue.h"
#include "nrfx_pwm.h"

#define CUSTOM_RGB_STEP (NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE / 255.0)

typedef struct {
    custom_hsv_ctx_t custom_hsv_ctx;
    nrf_pwm_values_individual_t pwm_values;
    custom_queue_t custom_queue_output;
    const custom_cmd_ctx_t *custom_cmd_ctx;
} custom_app_ctx_t;

#endif // CUSTOM_APP_DEFINES_H__