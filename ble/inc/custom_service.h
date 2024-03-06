#ifndef ESTC_SERVICE_H__
#define ESTC_SERVICE_H__

#include <stdint.h>

#include "ble.h"
#include "sdk_errors.h"

#define CUSTOM_BASE_UUID { 0x8a, 0x01, 0x02, 0x27, 0x42, 0x8e, 0xb9, 0xa5, 0x6d, 0x4e, 0xa7, 0x75, 0x7f, 0x6c, 0xdd, 0xc3 }

#define CUSTOM_SERVICE_1_UUID 0x6c7f

#define CUSTOM_GATT_CHAR_1_UUID 0xfff0
#define CUSTOM_GATT_CHAR_2_UUID 0xfff1
#define CUSTOM_GATT_CHAR_3_UUID 0xfff2

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

ret_code_t estc_ble_service_init(ble_custom_service_t *service);
// void estc_ble_service_on_ble_event(const ble_evt_t *ble_evt, void *ctx);
// void estc_update_characteristic_1_value(ble_custom_service_t *service, int32_t *value);

#endif /* ESTC_SERVICE_H__ */