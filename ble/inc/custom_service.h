#ifndef ESTC_SERVICE_H__
#define ESTC_SERVICE_H__

#include <stdint.h>

#include "ble.h"
#include "sdk_errors.h"

// TODO: 1. Generate random BLE UUID (Version 4 UUID) and define it in the following format:
#define CUSTOM_BASE_UUID { 0x8a, 0x01, 0x02, 0x27, 0x42, 0x8e, 0xb9, 0xa5, 0x6d, 0x4e, 0xa7, 0x75, 0x7f, 0x6c, 0xdd, 0xc3 }

// TODO: 2. Pick a random service 16-bit UUID and define it:
#define CUSTOM_SERVICE_UUID 0x6c7f


// TODO: 3. Pick a characteristic UUID and define it:
#define CUSTOM_GATT_CHAR_1_UUID 0xfff0

typedef struct
{
    uint16_t service_handle;
    // uint16_t connection_handle;
    ble_uuid128_t custom_base_uuid;
    ble_uuid_t service_uuid;
    ble_uuid_t char1_uuid;

    // TODO: 6.3. Add handles for characterstic (type: ble_gatts_char_handles_t)
    ble_gatts_char_handles_t char1_handles;
} ble_estc_service_t;

ret_code_t estc_ble_service_init(ble_estc_service_t *service);
// void estc_ble_service_on_ble_event(const ble_evt_t *ble_evt, void *ctx);
// void estc_update_characteristic_1_value(ble_estc_service_t *service, int32_t *value);

#endif /* ESTC_SERVICE_H__ */