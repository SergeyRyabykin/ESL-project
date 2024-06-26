#ifndef ESTC_SERVICE_H__
#define ESTC_SERVICE_H__

#include <stdint.h>

#include "ble.h"
#include "sdk_errors.h"

// #define CUSTOM_BASE_UUID { 0x8a, 0x01, 0x02, 0x27, 0x42, 0x8e, 0xb9, 0xa5, 0x6d, 0x4e, 0xa7, 0x75, 0x7f, 0x6c, 0xdd, 0xc3 }
// #define CUSTOM_SERVICE_1_UUID 0x6c7f

#define CUSTOM_BASE_UUID { 0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } 
#define CUSTOM_SERVICE_1_UUID BLE_APPEARANCE_GENERIC_HID

#define CUSTOM_GATT_CHAR_1_UUID 0x2bde
#define CUSTOM_GATT_CHAR_2_UUID 0x2bde

typedef struct {
    ble_uuid_t char_uuid;
    ble_gatts_char_handles_t char_handles;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t attr_md;
    uint8_t *value;
    unsigned int val_len;
} ble_custom_characteristic_t;

typedef struct {
    ble_uuid128_t base_service_uuid;
    ble_uuid_t service_uuid;
    uint16_t service_handle;
    ble_custom_characteristic_t **p_characteristics;
    unsigned int char_num;
} ble_custom_service_t;

ret_code_t custom_ble_service_init(ble_custom_service_t *service);
// void estc_ble_service_on_ble_event(const ble_evt_t *ble_evt, void *ctx);

/**
 * @brief Function to get client characteristic configuration descriptor
 * 
 * @param [in] conn_handle Connection handle 
 * @param [in] characteristic Pointer to characteristic to get the CCCD value
 * @param [out] cccd_value Pointer to variable to save CCCD value
 * @return ret_code_t NRF return values
 */
ret_code_t custom_ble_get_cccd(uint16_t conn_handle, ble_custom_characteristic_t *characteristic, uint16_t *cccd_value);
ret_code_t custom_ble_send_characteristic_value(uint16_t conn_handle, ble_custom_characteristic_t *characteristic, uint16_t type);

#endif /* ESTC_SERVICE_H__ */