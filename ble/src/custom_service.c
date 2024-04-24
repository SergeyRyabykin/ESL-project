#include "custom_service.h"

#include "app_error.h"
#include "nrf_log.h"

#include "ble.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"

#include <string.h>

#define CCCD_VALUE_LENGTH 2

static ret_code_t estc_ble_add_characteristics(ble_custom_service_t *service);

ret_code_t custom_ble_service_init(ble_custom_service_t *service)
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


ret_code_t custom_ble_get_cccd(uint16_t conn_handle, ble_custom_characteristic_t *characteristic, uint16_t *cccd_value)
{
    ret_code_t ret;

    ble_gatts_value_t cccd = {
        .len = CCCD_VALUE_LENGTH,
        .offset = 0,
        .p_value = (uint8_t *)cccd_value
    };

    // Get client characteristic configuration descriptor
    ret = sd_ble_gatts_value_get(conn_handle, characteristic->char_handles.cccd_handle, &cccd);

    return ret;
}

ret_code_t custom_ble_send_characteristic_value(uint16_t conn_handle, ble_custom_characteristic_t *characteristic, uint16_t type)
{
    ret_code_t ret;
   
    ble_gatts_hvx_params_t hvx_params = {
        .handle = characteristic->char_handles.value_handle,
        .type = type,
        .offset = 0,
        .p_len = (uint16_t*)&characteristic->val_len,
        .p_data = characteristic->value
    };

    ret = sd_ble_gatts_hvx(conn_handle, &hvx_params);

    return ret;
}

