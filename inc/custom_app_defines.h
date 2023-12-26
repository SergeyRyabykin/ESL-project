#ifndef CUSTOM_APP_DEFINES_H__
#define CUSTOM_APP_DEFINES_H__

#include "sdk_config.h"

#define CUSTOM_RGB_STEP (NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE / 255.0)

enum payload_id {
    DEFAULT_HSV_COLOR_ID
};

#endif // CUSTOM_APP_DEFINES_H__