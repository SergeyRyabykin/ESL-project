#ifndef CUSTOM_HSV_H__
#define CUSTOM_HSV_H__

#include <stdint.h>
#include <stdbool.h>

#define MAX_BRIGHTNESS 100
#define MAX_SATURATION 100

typedef uint16_t hue_t;

typedef struct {
    hue_t hue;          // degree
    uint8_t saturation;  // percent
    uint8_t value;       // percent
} custom_hsv_t;

typedef struct {
    custom_hsv_t color;
    bool satur_is_forward;
    bool value_is_forward;
} custom_hsv_ctx_t;

void custom_hsv_to_rgb(const custom_hsv_t color, uint16_t *red, uint16_t *green, uint16_t *blue);
void custom_hsv_hue_change_by_one(custom_hsv_t *color);
void custom_hsv_saturation_change_by_one(custom_hsv_ctx_t *context);
void custom_hsv_value_change_by_one(custom_hsv_ctx_t *context);


#endif // CUSTOM_HSV_H__