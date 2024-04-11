#include <math.h>
#include "custom_log.h"
#include "custom_hsv.h"


// #define MAX(a, b) ((a > b) ? a : b)
// #define MIN(a, b) ((a < b) ? a : b)

void custom_rgb_to_hsv(custom_hsv_t *hsv, const uint8_t red, const uint8_t green, const uint8_t blue)
{
   float r = red / 255.0;
   float g = green / 255.0;
   float b = blue / 255.0;

   float c_max = MAX(r, MAX(g, b));
   float c_min = MIN(r, MIN(g, b));

   float delta = c_max - c_min;

   // Hue
   if(0.0 == delta) {
      hsv->hue = 0;
   }
   else if(c_max == r) {
      int hue = 60 * fmod(((g - b) / delta), 6.0) ;
      hsv->hue = (360 + hue) % 360;
   }
   else if(c_max == g) {
      hsv->hue = 60 * ((b - r) / delta + 2);
   }
   else if(c_max == b) {
      hsv->hue = 60 * ((r - g) / delta + 4);
   }

   // Saturation
   if(0.0 == c_max) {
      hsv->saturation = 0;
   }
   else {
      hsv->saturation = (delta / c_max) * 100;
   }

   // Value
   hsv->value = c_max * 100;
}

void custom_hsv_to_rgb(const custom_hsv_t *color, uint8_t *red, uint8_t *green, uint8_t *blue)
{
    float s = color->saturation / 100.0f;
    float v = color->value / 100.0f;

    float c = s * v;

    float x = c * (1 - fabs(fmod((color->hue / 60.0), 2.0) - 1));

    float m = v - c;

    float r = 0;
    float g = 0;
    float b = 0;

    switch (color->hue / 60) {
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

    *red = (r + m) * 255;
    *green = (g + m) * 255;
    *blue = (b + m) * 255;
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
        if(MAX_SATURATION <= context->color.saturation) {
            context->satur_is_forward = false;
        }
        else {
            ++context->color.saturation;
        }
    }
    else {
        if(0 >= context->color.saturation) {
            context->satur_is_forward = true;
        }
        else {
            --context->color.saturation;
        }
    }
}

void custom_hsv_value_change_by_one(custom_hsv_ctx_t *context)
{    
    if(context->value_is_forward) {
        if(MAX_BRIGHTNESS <= context->color.value) {
            context->value_is_forward = false;
        }
        else {
            ++context->color.value;
        }
    }
    else {
        if(0 >= context->color.value) {
            context->value_is_forward = true;
        }
        else {
            --context->color.value;
        }
    }          
}
