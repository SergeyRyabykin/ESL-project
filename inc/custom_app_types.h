#ifndef CUSTOM_APP_TYPES_H__
#define CUSTOM_APP_TYPES_H__

#include "custom_hsv.h"
#include "custom_cmd.h"
#include "custom_queue.h"
#include "nrfx_pwm.h"
#include "custom_record.h"

typedef ret_code_t (*custom_print_output_t)(const char *message);

typedef struct {
    void (*custom_app_callback)(void * context);
    custom_record_t *default_record;
    custom_hsv_ctx_t *custom_hsv_ctx;
    nrf_pwm_values_individual_t *pwm_values;
    const custom_cmd_ctx_t *custom_cmd_ctx;
    custom_print_output_t custom_print_output;
    custom_cmd_executor_ctx_t *executor_ctx;
} custom_app_ctx_t;

#endif // CUSTOM_APP_TYPES_H__