#include <math.h>
#include "sdk_config.h"
// #include "nordic_common.h"
#include "custom_hsv.h"

#define PWM_COUNT_TOP NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE

void custom_hsv_to_rgb(const custom_hsv_t color, uint16_t *red, uint16_t *green, uint16_t *blue)
{
    float s = color.saturation / 100.0f;
    float v = color.value / 100.0f;

    float c = s * v;

    float x = c * (1 - fabs(fmod((color.hue / 60.0), 2.0) - 1));

    float m = v - c;

    float r = 0;
    float g = 0;
    float b = 0;

    switch (color.hue / 60) {
        case 0: 
            r = c; g = x; b = 0;
            break;
        case 1: 
            r = x; g = c; b = 0;
            break;
        case 2:
            r = 0; g = c; b = x;
            break;
        case 3:
            r = 0; g = x; b = c;
            break;
        case 4:
            r = x; g = 0; b = c;
            break;
        case 5:
            r = c; g = 0; b = x;
            break;
        default:
            break;
    }

    *red = (r + m) * PWM_COUNT_TOP;
    *green = (g + m) * PWM_COUNT_TOP;
    *blue = (b + m) * PWM_COUNT_TOP;
}

void custom_hsv_hue_change_by_one(custom_hsv_t *color)
{
    if(360 <= ++color->hue) {
        color->hue = 0;
    }
}

void custom_hsv_saturation_change_by_one(custom_hsv_ctx_t *context)
{
    if(context->satur_is_forward) {
        if(MAX_SATURATION <= ++context->color.saturation) {
            context->satur_is_forward = false;
        }
    }
    else {
        if(0 >= --context->color.saturation) {
            context->satur_is_forward = true;
        }
    }
}

void custom_hsv_value_change_by_one(custom_hsv_ctx_t *context)
{    
    if(context->value_is_forward) {
        if(MAX_SATURATION <= ++context->color.value) {
            context->value_is_forward = false;
        }
    }
    else {
        if(0 >= --context->color.value) {
            context->value_is_forward = true;
        }
    }          
}