#ifndef CUSTOM_NVM_H__
#define CUSTOM_NVM_H__

#include "nrfx_nvmc.h"
#include "nrf_dfu_types.h"
#include "app_util.h"

typedef uint16_t custom_app_data_validity_t;

#define CUSTOM_APP_DATA_AREA_START_ADDR ((uint32_t)(BOOTLOADER_ADDRESS - NRF_DFU_APP_DATA_AREA_SIZE))
#define CUSTOM_NUMBER_APP_DATA_PAGES (NRF_DFU_APP_DATA_AREA_SIZE / CODE_PAGE_SIZE)

#define CUSTOM_APP_DATA_VALID_ID   ((custom_app_data_validity_t)0x5AA5)
#define CUSTOM_APP_DATA_INVALID_ID ((custom_app_data_validity_t)0x1881)

#define CUSTOM_DATA_SIZE_BYTE(obj_size) (obj_size + sizeof(CUSTOM_APP_DATA_VALID_ID))
#define CUSTOM_DATA_SIZE_WORD(obj_size) (CUSTOM_DATA_SIZE_BYTE(obj_size) / sizeof(uint32_t) + ((CUSTOM_DATA_SIZE_BYTE(obj_size) % sizeof(uint32_t)) ? 1 : 0))

nrfx_err_t custom_nvm_init(void *obj, size_t obj_size);
nrfx_err_t custom_nvm_save_obj(const void *obj, size_t obj_size);




#endif // CUSTOM_NVM_H__