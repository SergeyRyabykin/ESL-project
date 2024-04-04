#include <stdint.h>
#include "nrfx_nvmc.h"
#include "nrf_dfu_types.h"
#include "app_util.h"
#include "custom_nvm.h"

#include "nrf_soc.h"

#include "custom_log.h"

typedef uint16_t custom_nvm_record_validity_t;
typedef uint8_t custom_nvm_payload_size_t; // CUSTOM_PAYLOAD_MAX_SIZE is maximum payload_size

#define CUSTOM_APP_DATA_AREA_START_ADDR ((uint32_t)(BOOTLOADER_ADDRESS - NRF_DFU_APP_DATA_AREA_SIZE))
#define CUSTOM_NVM_NUMBER_APP_DATA_PAGES (NRF_DFU_APP_DATA_AREA_SIZE / CODE_PAGE_SIZE)
#define CUSTOM_NVM_APP_DATA_PAGE_ADDR(page) (CUSTOM_APP_DATA_AREA_START_ADDR + page * CODE_PAGE_SIZE)
#define CUSTOM_NVM_GET_NEXT_PAGE(page) ((page + 1 == CUSTOM_NVM_NUMBER_APP_DATA_PAGES) ? 0 : (page + 1))


#define CUSTOM_NVM_RECORD_VALID   ((custom_nvm_record_validity_t)0x5AA5)
#define CUSTOM_NVM_RECORD_INVALID ((custom_nvm_record_validity_t)0x1881)

typedef struct {
    custom_nvm_record_validity_t validity;
    custom_nvm_payload_size_t payload_size;
    custom_nvm_payload_id_t payload_id;
} custom_nvm_record_info_t;

#define CUSTOM_NVM_RECORD_SIZE_IN_WORDS(payload_size) (CUSTOM_NVM_SIZE_IN_WORDS(sizeof(custom_nvm_record_info_t)) + CUSTOM_NVM_SIZE_IN_WORDS(payload_size))
#define CUSTOM_NVM_RECORD_SIZE_IN_BYTES(payload_size) (CUSTOM_NVM_RECORD_SIZE_IN_WORDS(payload_size) * sizeof(uint32_t))

static int custom_nvm_get_actual_page(void)
{
    int page = 0;

    for(unsigned int i = 0; i < CUSTOM_NVM_NUMBER_APP_DATA_PAGES; i++) {
        custom_nvm_record_info_t *record = (custom_nvm_record_info_t *)CUSTOM_NVM_APP_DATA_PAGE_ADDR(i);

        if(CUSTOM_NVM_RECORD_VALID == record->validity || CUSTOM_NVM_RECORD_INVALID == record->validity) {
            page = i;
            break;
        }
    }

    return page;
}

static uintptr_t custom_nvm_find_empty_record_addr(void)
{
    unsigned int current_page = custom_nvm_get_actual_page();
    custom_nvm_record_info_t *record = (custom_nvm_record_info_t *)CUSTOM_NVM_APP_DATA_PAGE_ADDR(current_page);

    while(CUSTOM_NVM_RECORD_VALID == record->validity || CUSTOM_NVM_RECORD_INVALID == record->validity) {
        record = (custom_nvm_record_info_t *)((uintptr_t)record + CUSTOM_NVM_RECORD_SIZE_IN_BYTES(record->payload_size));
    }

    return (uintptr_t)record;
}

static bool custom_nvm_is_addr_valid_for_page(const uintptr_t addr, const unsigned int page, const size_t object_size)
{
    uintptr_t page_addr = CUSTOM_NVM_APP_DATA_PAGE_ADDR(page);

    if(page_addr > addr || (page_addr + CODE_PAGE_SIZE) < (addr + CUSTOM_NVM_RECORD_SIZE_IN_BYTES(object_size))) {
        return false;
    }

    return true;
}

static ret_code_t custom_nvm_write_record(const void *object, const size_t object_size, const custom_nvm_payload_id_t id, const uintptr_t addr)
{
    unsigned int record_size_w = CUSTOM_NVM_RECORD_SIZE_IN_WORDS(object_size);
    uint32_t record_array[record_size_w];

    custom_nvm_record_info_t info = {
        .validity = CUSTOM_NVM_RECORD_VALID,
        .payload_size = object_size,
        .payload_id = id
    };

    memcpy(record_array, &info, sizeof(info));
    memcpy(&record_array[CUSTOM_NVM_SIZE_IN_WORDS(sizeof(info))], object, object_size);

    NRF_LOG_INFO("Before writing:");
    uintptr_t my_addr = CUSTOM_APP_DATA_AREA_START_ADDR;
    for(int i = 0; i < 10; i++) {
        NRF_LOG_INFO("%x : %x", my_addr + i*4, *((uint32_t *)my_addr + i));
    }

    NRF_LOG_INFO("Must be written:");
    for(int i = 0; i < record_size_w; i++) {
        NRF_LOG_INFO("%x : %X", addr + i*4, record_array[i]);
    }

    return sd_flash_write((uint32_t *)addr, record_array, record_size_w);
    // for(int i = record_size_w - 1; i >= 0 ; i--){
    //     uintptr_t current_word_addr = addr + i * sizeof(uint32_t);
    //     if(nrfx_nvmc_word_writable_check(current_word_addr, record_array[i])) {
    //         while(!nrfx_nvmc_write_done_check()) {
    //             ;
    //         }
    //         nrfx_nvmc_word_write(current_word_addr, record_array[i]);
    //     }
    //     else {
    //         return NRF_ERROR_INVALID_DATA;
    //     }
    // }

    // return NRF_SUCCESS;
}

static ret_code_t custom_nvm_copy_valid_records(unsigned int dest_page, unsigned int src_page)
{
    custom_nvm_record_info_t *record = (custom_nvm_record_info_t *)CUSTOM_NVM_APP_DATA_PAGE_ADDR(src_page);
    uintptr_t dest_addr = CUSTOM_NVM_APP_DATA_PAGE_ADDR(dest_page);

    while(CUSTOM_NVM_RECORD_VALID == record->validity || CUSTOM_NVM_RECORD_INVALID == record->validity) {
        if(CUSTOM_NVM_RECORD_VALID == record->validity) {
            if(custom_nvm_is_addr_valid_for_page(dest_addr, dest_page, record->payload_size)) {
                unsigned int record_size_w = CUSTOM_NVM_RECORD_SIZE_IN_WORDS(record->payload_size);

                for(int i = record_size_w - 1; i >= 0 ; i--){
                    uintptr_t current_word_addr = dest_addr + i * sizeof(uint32_t);

                    if(nrfx_nvmc_word_writable_check(current_word_addr, *((uint32_t *)record + i))) {
                        while(!nrfx_nvmc_write_done_check()) {
                            ;
                        }
                        nrfx_nvmc_word_write(current_word_addr, *((uint32_t *)record + i));
                    }
                    else {
                        return NRF_ERROR_INVALID_DATA;
                    }
                }

                dest_addr = dest_addr + CUSTOM_NVM_RECORD_SIZE_IN_BYTES(record->payload_size);
            }
            else {
                return NRF_ERROR_INVALID_ADDR;
            }
        }

        record = (custom_nvm_record_info_t *)((char *)record + CUSTOM_NVM_RECORD_SIZE_IN_BYTES(record->payload_size));
    }

    return NRF_SUCCESS;
}

static ret_code_t custom_nvm_discard_record(const uintptr_t record_addr)
{
    uint32_t record_array[CUSTOM_NVM_RECORD_SIZE_IN_WORDS(sizeof(custom_nvm_record_info_t))] = {0};

    custom_nvm_record_info_t invalid = {
        .validity = CUSTOM_NVM_RECORD_INVALID,
        .payload_size = ((custom_nvm_record_info_t *)record_addr)->payload_size,
        .payload_id = ((custom_nvm_record_info_t *)record_addr)->payload_id
    };

    memcpy(record_array, &invalid, sizeof(invalid));

    NRF_LOG_INFO("To be discarded:");
    for(int i = 0; i < CUSTOM_NVM_RECORD_SIZE_IN_WORDS(sizeof(custom_nvm_record_info_t)); i++) {
        NRF_LOG_INFO("%x : %X", record_addr + i*4, record_array[i]);
    }
    
    return sd_flash_write((uint32_t *)record_addr, record_array, CUSTOM_NVM_RECORD_SIZE_IN_WORDS(sizeof(custom_nvm_record_info_t)));
    // for(int i = 0; i < CUSTOM_NVM_RECORD_SIZE_IN_WORDS(sizeof(custom_nvm_record_info_t)); i++) {
    //     uintptr_t current_word_addr = record_addr + i * sizeof(uint32_t);

    //     if(nrfx_nvmc_word_writable_check(current_word_addr, record_array[i])) {
    //         while(!nrfx_nvmc_write_done_check()) {
    //             ;
    //         }
    //         nrfx_nvmc_word_write(current_word_addr, record_array[i]);
    //     }
    //     else {
    //         return NRF_ERROR_INVALID_ADDR;
    //     }
    // }

    // return NRF_SUCCESS;
}

static uintptr_t custom_nvm_find_record_by_id(const custom_nvm_payload_id_t id)
{
    unsigned int current_page = custom_nvm_get_actual_page();
    custom_nvm_record_info_t *record = (custom_nvm_record_info_t *)CUSTOM_NVM_APP_DATA_PAGE_ADDR(current_page);

    while(CUSTOM_NVM_RECORD_VALID == record->validity || CUSTOM_NVM_RECORD_INVALID == record->validity) {

        if(CUSTOM_NVM_RECORD_VALID == record->validity && id == record->payload_id) {
            return (uintptr_t)record;
        }

        record = (custom_nvm_record_info_t *)((char *)record + CUSTOM_NVM_RECORD_SIZE_IN_BYTES(record->payload_size));
    }

    return 0;
}

void custom_nvm_erase(void) {
    nrfx_nvmc_page_erase(CUSTOM_NVM_APP_DATA_PAGE_ADDR(0));
    nrfx_nvmc_page_erase(CUSTOM_NVM_APP_DATA_PAGE_ADDR(1));
    nrfx_nvmc_page_erase(CUSTOM_NVM_APP_DATA_PAGE_ADDR(2));
}

uintptr_t custom_nvm_find(const custom_nvm_payload_id_t id)
{
    uintptr_t record = custom_nvm_find_record_by_id(id);

    if(record) {
        return record + CUSTOM_NVM_SIZE_IN_BYTES(sizeof(custom_nvm_record_info_t));
    }

    return 0;
}

uintptr_t custom_nvm_find_next(const uintptr_t current_object_addr, const custom_nvm_payload_id_t id)
{
    unsigned int current_page = custom_nvm_get_actual_page();

    custom_nvm_record_info_t *record = (custom_nvm_record_info_t *)(current_object_addr - CUSTOM_NVM_SIZE_IN_BYTES(sizeof(custom_nvm_record_info_t)));

    if(!custom_nvm_is_addr_valid_for_page((uintptr_t)record, current_page, 0)) {
        return 0;
    }

    record = (custom_nvm_record_info_t *)((uintptr_t)record + CUSTOM_NVM_RECORD_SIZE_IN_BYTES(record->payload_size));

    while(CUSTOM_NVM_RECORD_VALID == record->validity || CUSTOM_NVM_RECORD_INVALID == record->validity) {

        if(CUSTOM_NVM_RECORD_VALID == record->validity && id == record->payload_id) {
            return (uintptr_t)record + CUSTOM_NVM_SIZE_IN_BYTES(sizeof(custom_nvm_record_info_t));
        }

        record = (custom_nvm_record_info_t *)((char *)record + CUSTOM_NVM_RECORD_SIZE_IN_BYTES(record->payload_size));
    }

    return 0;
}


ret_code_t custom_nvm_discard_by_id(const custom_nvm_payload_id_t id)
{
    ret_code_t ret = NRF_ERROR_NOT_FOUND;

    uintptr_t record = custom_nvm_find_record_by_id(id);

    if(record) {
        ret = custom_nvm_discard_record(record);
    }

    return ret;
}

ret_code_t custom_nvm_discard(const uintptr_t object_addr)
{
    ret_code_t ret = NRF_ERROR_NOT_FOUND;

    unsigned int current_page = custom_nvm_get_actual_page();
    uintptr_t record = object_addr - CUSTOM_NVM_SIZE_IN_BYTES(sizeof(custom_nvm_record_info_t));

     if(!custom_nvm_is_addr_valid_for_page(record, current_page, 0)) {
        return ret;
    }

    if(record && CUSTOM_NVM_RECORD_VALID == ((custom_nvm_record_info_t *)record)->validity) {
        ret = custom_nvm_discard_record(record);
    }

    return ret;
}

ret_code_t custom_nvm_save(const void *object, const size_t object_size, const custom_nvm_payload_id_t id)
{
    ret_code_t ret = NRF_ERROR_DATA_SIZE;

    if(CUSTOM_PAYLOAD_MAX_SIZE < object_size) {
        return ret;
    }

    // // Object saving attempt
    unsigned int page = custom_nvm_get_actual_page();
    uintptr_t addr = custom_nvm_find_empty_record_addr();

    if(custom_nvm_is_addr_valid_for_page(addr, page, object_size)) {
        ret = custom_nvm_write_record(object, object_size, id, addr);
    }
    else {
        ret = NRF_ERROR_INVALID_ADDR;
    }

    // Copy all the valid records to the convenient page, erase current page and then save desired object
    if(NRF_SUCCESS != ret) {
        unsigned int next_page = CUSTOM_NVM_GET_NEXT_PAGE(page);

        while(NRF_SUCCESS != ret && page != next_page) {
            ret = custom_nvm_copy_valid_records(next_page, page);

            if(NRF_SUCCESS == ret) {

                nrfx_nvmc_page_erase(CUSTOM_NVM_APP_DATA_PAGE_ADDR(page));

                addr = custom_nvm_find_empty_record_addr();
                if(custom_nvm_is_addr_valid_for_page(addr, next_page, object_size)) {
                    ret = custom_nvm_write_record(object, object_size, id, addr);
                }
                else {
                    ret = NRF_ERROR_INVALID_ADDR;
                }
            }
            next_page = CUSTOM_NVM_GET_NEXT_PAGE(next_page);
        }
    }

    return ret;
}

