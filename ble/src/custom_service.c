#include "custom_service.h"

#include "app_error.h"
#include "nrf_log.h"

#include "ble.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"

#include <string.h>

static ret_code_t estc_ble_add_characteristics(ble_custom_service_t *service);

ret_code_t estc_ble_service_init(ble_custom_service_t *service)
{
    ret_code_t error_code = NRF_SUCCESS;

    error_code = sd_ble_uuid_vs_add(&service->base_service_uuid, &service->service_uuid.type);
    if(NRF_SUCCESS != error_code) {
        return error_code;
    }

    error_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &service->service_uuid, &service->service_handle);
    if(NRF_SUCCESS != error_code) {
        return error_code;
    }

    error_code = estc_ble_add_characteristics(service);

    // NRF_LOG_DEBUG("%s:%d | Service UUID: 0x%04x", __FUNCTION__, __LINE__, service_uuid.uuid);
    // NRF_LOG_DEBUG("%s:%d | Service UUID type: 0x%02x", __FUNCTION__, __LINE__, service_uuid.type);
    // NRF_LOG_DEBUG("%s:%d | Service handle: 0x%04x", __FUNCTION__, __LINE__, service->service_handle);
    return error_code;
}

static ret_code_t estc_ble_add_characteristics(ble_custom_service_t *service)
{
    ret_code_t error_code = NRF_SUCCESS;

    for(unsigned int i = 0; i < service->char_num; i++) {
        ble_custom_characteristic_t *characteristic = service->p_characteristics[i];

        error_code = sd_ble_uuid_vs_add(&service->base_service_uuid, &characteristic->char_uuid.type);
        if(NRF_SUCCESS != error_code) {
            return error_code;
        }

        ble_gatts_attr_t attr_char_value = { 
            .p_uuid = &characteristic->char_uuid,
            .p_attr_md = &characteristic->attr_md,
            .init_len = characteristic->val_len,
            .max_len = characteristic->val_len,
            .p_value = characteristic->value
        };

        error_code = sd_ble_gatts_characteristic_add(service->service_handle, &characteristic->char_md, &attr_char_value, &characteristic->char_handles);
        if(NRF_SUCCESS != error_code) {
            return error_code;
        }
    }
    
    return error_code;
}
