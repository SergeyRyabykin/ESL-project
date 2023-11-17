#ifndef CUSTOM_HSV_H__
#define CUSTOM_HSV_H__

#include <stdint.h>

#define MAX_BRIGHTNESS 100
#define MAX_SATURATION 100

typedef uint16_t hue_t;

struct hsv {
    hue_t hue;       // degree
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

union rgb hue_to_rgb(hue_t hue);
void set_brightness(uint8_t brightness, union rgb *rgb);
void set_saturation(uint8_t saturation, union rgb *rgb);


#endif // CUSTOM_HSV_H__