#ifndef CUSTOM_BLE_H
#define CUSTOM_BLE_H

#include "custom_hsv.h"

typedef void (*custom_cb_ble_write_data_t)(void *data);

void custom_ble_init(custom_hsv_t *color, custom_cb_ble_write_data_t custom_ble_write_data_cb);
void custom_ble_notify_color_changed(void const *data, uint16_t len);
ret_code_t custom_ble_notify_message(char const *msg);
void custom_ble_advertising_start(void);
void custom_ble_delete_peers(void);




#endif // CUSTOM_BLE_H