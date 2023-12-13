#include <stdint.h>
#include "nrfx_nvmc.h"
#include "nrf_dfu_types.h"
#include "app_util.h"
#include "custom_nvm.h"

typedef uint16_t custom_app_data_validity_t;

#define CUSTOM_APP_DATA_AREA_START_ADDR ((uint32_t)(BOOTLOADER_ADDRESS - NRF_DFU_APP_DATA_AREA_SIZE))
#define CUSTOM_NUMBER_APP_DATA_PAGES (NRF_DFU_APP_DATA_AREA_SIZE / CODE_PAGE_SIZE)

#define CUSTOM_APP_DATA_VALID_ID   ((custom_app_data_validity_t)0x5AA5)
#define CUSTOM_APP_DATA_INVALID_ID ((custom_app_data_validity_t)0x1881)

#define CUSTOM_DATA_SIZE_BYTE(obj_size) (obj_size + sizeof(CUSTOM_APP_DATA_VALID_ID))
#define CUSTOM_DATA_SIZE_WORD(obj_size) (CUSTOM_DATA_SIZE_BYTE(obj_size) / sizeof(uint32_t) + ((CUSTOM_DATA_SIZE_BYTE(obj_size) % sizeof(uint32_t)) ? 1 : 0))

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static bool custom_nvm_is_addr_valid(uintptr_t addr, size_t obj_size) 
{
    unsigned int word_num = CUSTOM_DATA_SIZE_WORD(obj_size);
    return (!(addr % sizeof(uint32_t)) && (CUSTOM_APP_DATA_AREA_START_ADDR <= addr) && (BOOTLOADER_ADDRESS > (addr + word_num * sizeof(uint32_t))));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static bool custom_nvm_is_data_valid(uint32_t data[], size_t obj_size) 
{
    custom_app_data_validity_t validity = 0;
    
    memcpy(&validity, (unsigned char *)data + obj_size, sizeof(validity));

    return (CUSTOM_APP_DATA_VALID_ID == validity);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static void custom_nvm_convert_obj(const void *src_obj, size_t obj_size, uint32_t dst_array[])
{
    memcpy(dst_array, src_obj, obj_size);
    memcpy( (unsigned char *)dst_array + obj_size, \
            &(custom_app_data_validity_t){CUSTOM_APP_DATA_VALID_ID}, \
            sizeof(CUSTOM_APP_DATA_VALID_ID) \
          );
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static void custom_nvm_recover_obj(void *obj, uint32_t data[], size_t obj_size)
{
    memcpy(obj, data, obj_size);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static void custom_nvm_load_data(uintptr_t addr, uint32_t dest_array[], size_t obj_size)
{
    for (unsigned int i = 0; i < CUSTOM_DATA_SIZE_WORD(obj_size); i++) {
        dest_array[i] = *((uint32_t *)addr + i);
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static void custom_nvm_write_data(uintptr_t addr, uint32_t num, uint32_t data[num])
{
    for(unsigned int i = 0; i < num; i++){
        while(!nrfx_nvmc_write_done_check()) {
            ;
        }
        nrfx_nvmc_word_write(addr + i * sizeof(uint32_t), data[i]);
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static nrfx_err_t custom_nvm_find_valid_obj(void *obj, size_t obj_size)
{
    uint32_t data_size_w = CUSTOM_DATA_SIZE_WORD(obj_size);
    uint32_t data[data_size_w];
    uintptr_t current_addr = CUSTOM_APP_DATA_AREA_START_ADDR;

    while(custom_nvm_is_addr_valid(current_addr, obj_size)) {
        custom_nvm_load_data(current_addr, data, obj_size);
        if(custom_nvm_is_data_valid(data, obj_size)) {
            custom_nvm_recover_obj(obj, data, obj_size);
            return NRFX_SUCCESS;
        }
        current_addr += data_size_w * (sizeof(uint32_t));
    }

    return NRF_ERROR_NOT_FOUND;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static nrfx_err_t custom_nvm_write_obj(uintptr_t addr, const void *obj, size_t obj_size)
{
    uint32_t data_size_w = CUSTOM_DATA_SIZE_WORD(obj_size);
    uint32_t data[data_size_w];

    custom_nvm_convert_obj(obj, obj_size, data);

    for(unsigned int i = 0; i < data_size_w; i++){
        if(!nrfx_nvmc_word_writable_check(addr + i * sizeof(uint32_t), data[i])) {
            return NRF_ERROR_INVALID_DATA;
        }
    }

    custom_nvm_write_data(addr, data_size_w, data);

    return NRFX_SUCCESS;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static nrfx_err_t custom_nvm_erase_area(void)
{
    nrfx_err_t ret;

    for(unsigned int i = 0; i < CUSTOM_NUMBER_APP_DATA_PAGES; i++) {
        ret = nrfx_nvmc_page_erase(CUSTOM_APP_DATA_AREA_START_ADDR + CODE_PAGE_SIZE * i);
        if(NRFX_SUCCESS != ret) {
            break;
        }
    }

    return ret;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static void custom_nvm_spoil_data(uint32_t data[], size_t obj_size) 
{
    memset(data, 0x0, CUSTOM_DATA_SIZE_WORD(obj_size) * sizeof(uint32_t));
    memcpy( (unsigned char *)data + obj_size, \
        &(custom_app_data_validity_t){CUSTOM_APP_DATA_INVALID_ID}, \
        sizeof(CUSTOM_APP_DATA_INVALID_ID) \
        );
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
nrfx_err_t custom_nvm_init(void *obj, size_t obj_size) {
    nrfx_err_t ret;
    uint32_t data[CUSTOM_DATA_SIZE_WORD(obj_size)];
    custom_app_data_validity_t validity = 0;

    custom_nvm_load_data(CUSTOM_APP_DATA_AREA_START_ADDR, data, obj_size);
    
    memcpy(&validity, (unsigned char *)data + obj_size, sizeof(validity));

    if(CUSTOM_APP_DATA_VALID_ID != validity && CUSTOM_APP_DATA_INVALID_ID != validity) {
        ret = custom_nvm_erase_area();
        if(NRFX_SUCCESS != ret) {
            return ret;
        }

        ret = custom_nvm_write_obj(CUSTOM_APP_DATA_AREA_START_ADDR, obj, obj_size);
    }
    else {
        ret = custom_nvm_find_valid_obj(obj, obj_size);
    }

    return ret;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
nrfx_err_t custom_nvm_save_obj(const void *obj, size_t obj_size)
{
    nrfx_err_t ret;
    uint32_t data_size_w = CUSTOM_DATA_SIZE_WORD(obj_size);
    uint32_t data[data_size_w];
    uintptr_t current_addr = CUSTOM_APP_DATA_AREA_START_ADDR;

    while(custom_nvm_is_addr_valid(current_addr, obj_size)) {
        custom_nvm_load_data(current_addr, data, obj_size);

        if(custom_nvm_is_data_valid(data, obj_size)) {
            custom_nvm_spoil_data(data, obj_size);
            custom_nvm_write_data(current_addr, data_size_w, data);
            // TODO: Avoid of data losing
            if(custom_nvm_is_addr_valid((current_addr += data_size_w * (sizeof(uint32_t))), obj_size)) {
                ret = custom_nvm_write_obj(current_addr, obj, obj_size);
                if(NRFX_SUCCESS == ret) {
                    return ret;
                }
            }

            break;
        }

        current_addr += data_size_w * (sizeof(uint32_t));
    }

    ret = custom_nvm_erase_area();
    if(NRFX_SUCCESS != ret) {
        return ret;
    }
    // TODO: Avoid of data losing
    ret = custom_nvm_write_obj(CUSTOM_APP_DATA_AREA_START_ADDR, obj, obj_size);
    return ret;
}
