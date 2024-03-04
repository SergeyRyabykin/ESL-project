#include "custom_service.h"

#include "app_error.h"
#include "nrf_log.h"

#include "ble.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"

// static ret_code_t estc_ble_add_characteristics(ble_estc_service_t *service);

ret_code_t estc_ble_service_init(ble_estc_service_t *service)
{
    ret_code_t error_code = NRF_SUCCESS;

    // TODO: 3. Add service UUIDs to the BLE stack table using `sd_ble_uuid_vs_add`
    error_code = sd_ble_uuid_vs_add(&service->custom_base_uuid, &service->service_uuid.type);
    if(NRF_SUCCESS != error_code) {
        return error_code;
    }

    // TODO: 4. Add service to the BLE stack using `sd_ble_gatts_service_add`
    error_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &service->service_uuid, &service->service_handle);

    // NRF_LOG_DEBUG("%s:%d | Service UUID: 0x%04x", __FUNCTION__, __LINE__, service_uuid.uuid);
    // NRF_LOG_DEBUG("%s:%d | Service UUID type: 0x%02x", __FUNCTION__, __LINE__, service_uuid.type);
    // NRF_LOG_DEBUG("%s:%d | Service handle: 0x%04x", __FUNCTION__, __LINE__, service->service_handle);
    return error_code;
}

// static ret_code_t estc_ble_add_characteristics(ble_estc_service_t *service)
// {
//     ret_code_t error_code = NRF_SUCCESS;

//     ble_uuid128_t custom_char1_uuid = {
//         .uuid128 = CASTOM_CHAR1_BASE_UUID
//     };

//     ble_uuid_t char1_uuid = {
//         .uuid = CUSTOM_GATT_CHAR_1_UUID
//     };

//     // TODO: 6.1. Add custom characteristic UUID using `sd_ble_uuid_vs_add`, same as in step 4
//     error_code = sd_ble_uuid_vs_add(&custom_char1_uuid, &char1_uuid.type);
//     if(NRF_SUCCESS != error_code) {
//         return error_code;
//     }

//     // TODO: 6.5. Configure Characteristic metadata (enable read and write)
//     ble_gatts_char_md_t char_md = { 
//         .char_props = {
//             .read = 1,
//             .write = 1
//         }
//      };


//     // Configures attribute metadata. For now we only specify that the attribute will be stored in the softdevice
//     ble_gatts_attr_md_t attr_md = { 0 };
//     attr_md.vloc = BLE_GATTS_VLOC_STACK;


//     // TODO: 6.6. Set read/write security levels to our attribute metadata using `BLE_GAP_CONN_SEC_MODE_SET_OPEN`
//     BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
//     BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

//     // TODO: 6.2. Configure the characteristic value attribute (set the UUID and metadata)
//     ble_gatts_attr_t attr_char_value = { 
//         .p_uuid = &char1_uuid,
//         .p_attr_md = &attr_md,
//     // TODO: 6.7. Set characteristic length in number of bytes in attr_char_value structure
//         .init_len = 4
//     };

//     // TODO: 6.4. Add new characteristic to the service using `sd_ble_gatts_characteristic_add`
//     error_code = sd_ble_gatts_characteristic_add(service->service_handle, &char_md, &attr_char_value, &service->char1_handles);


//     return error_code;
// }
