#ifndef CUSTOM_BLE_H
#define CUSTOM_BLE_H

#include "custom_hsv.h"

void custom_ble_init(custom_hsv_t *color);
void custom_ble_notify_color_changed(void);


#endif // CUSTOM_BLE_H