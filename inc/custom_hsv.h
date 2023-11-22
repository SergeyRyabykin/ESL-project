#ifndef CUSTOM_HSV_H__
#define CUSTOM_HSV_H__

#include <stdint.h>

#define MAX_BRIGHTNESS 100
#define MAX_SATURATION 100

typedef uint16_t hue_t;

struct hsv {
    hue_t hue;          // degree
    uint8_t saturation; // percent
    uint8_t brightness; // percent
};

union rgb {
    uint16_t components[3];
    struct {
        uint16_t red;
        uint16_t green;
        uint16_t blue;
    };
};

void hsv_to_rgb(struct hsv color, union rgb *rgb_color);

#endif // CUSTOM_HSV_H__