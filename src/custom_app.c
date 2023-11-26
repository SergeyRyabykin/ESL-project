#include "custom_app.h"

static custom_app_state_t g_app_state = DEFAULT_MODE;

custom_app_state_t custom_app_change_state(void)
{
    if(MODE_NUMBER <= ++g_app_state) {
        g_app_state = DEFAULT_MODE;
    }

    return g_app_state;
}

custom_app_state_t custom_app_get_state(void)
{
    return g_app_state;
}

void custom_app_set_pwm_indicator(const custom_app_state_t state, custom_app_pwm_indicator_ctx_t *context)
{
    if(!context) {
        return;
    }

    switch (state) {
        case DEFAULT_MODE: 
            context->pwm_duty_step = 0; 
            *context->pwm_channel = APP_IND_DEFAULT_INC; 
            break;
        case HUE_MODE: 
            context->pwm_duty_step = APP_IND_HUE_INC;
            break;
        case SATURATION_MODE: 
            context->pwm_duty_step = APP_IND_SATUR_INC; 
            break;
        case VALUE_MODE: 
            context->pwm_duty_step = 0; 
            *context->pwm_channel = APP_IND_VALUE_INC; 
            break;
        default:
            break;
    }
}

void custom_app_process_pwm_indicator(custom_app_pwm_indicator_ctx_t *context)
{
    if(!context) {
        return;
    }

    if(context->pwm_duty_step) {
        if(context->forward_direct) {
            if(NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE >= (*context->pwm_channel + context->pwm_duty_step)) {
                *context->pwm_channel += context->pwm_duty_step;
            }
            else {
                *context->pwm_channel = NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE; // To prevent overruning of the board during missmatch step values because of increasing in the event handler
                context->forward_direct = false;
            }
        }
        else {
            if(0 <= (*context->pwm_channel - context->pwm_duty_step)) {
                *context->pwm_channel -= context->pwm_duty_step;
            }
            else {
                *context->pwm_channel = 0;  // To prevent overruning of the board during missmatch step values because of increasing in the event handler
                context->forward_direct = true;
            }
        }

    }
}