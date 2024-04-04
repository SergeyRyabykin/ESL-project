#include "custom_record.h"
#include "fds.h"
#include <string.h>

#include "custom_log.h"

#define SIZE_IN_WORDS(size_in_bytes) (size_in_bytes / sizeof(uint32_t) + ((size_in_bytes % sizeof(uint32_t)) ? 1 : 0))

static fds_record_t g_record;
static fds_record_desc_t g_record_desc;
static fds_flash_record_t g_flash_record;
static fds_find_token_t g_ftok;

static volatile bool is_complete = false;

static inline void wait_for_complete(void)
{
    while(false == is_complete) {
        ;
    }
    is_complete = false;
}

static void custom_fds_evt_handler(fds_evt_t const *p_fds_evt)
{
    switch (p_fds_evt->id) {
        case FDS_EVT_INIT:
        case FDS_EVT_WRITE:
        case FDS_EVT_UPDATE:
        case FDS_EVT_DEL_RECORD:
        case FDS_EVT_DEL_FILE:
        case FDS_EVT_GC:
            is_complete = true;
        default:
            break;
    }
}

ret_code_t custom_record_storage_init(void)
{
    ret_code_t ret = fds_register(custom_fds_evt_handler);

    if(NRF_SUCCESS != ret) {
        return ret;
    }
    ret = fds_init();

    if(NRF_SUCCESS == ret) {
        wait_for_complete();
    }

    memset(&g_ftok, 0x00, sizeof(fds_find_token_t));

    return ret;
}

ret_code_t custom_record_save(uint16_t key, void const *src_ptr, size_t size_bytes)
{
    uint32_t payload[SIZE_IN_WORDS(size_bytes)];

    memcpy(payload, src_ptr, size_bytes);

    g_record.file_id = FILE_ID;
    g_record.key = key;
    g_record.data.p_data = payload;
    g_record.data.length_words = SIZE_IN_WORDS(size_bytes);

    ret_code_t ret = fds_record_write(&g_record_desc, &g_record);

    if(NRF_SUCCESS == ret) {
        wait_for_complete();
    }

    return ret;
}

ret_code_t custom_record_read(uint16_t key, void *dest_ptr)
{
    memset(&g_ftok, 0x00, sizeof(fds_find_token_t));

    ret_code_t ret = fds_record_find(FILE_ID, key, &g_record_desc, &g_ftok);

    if(NRF_SUCCESS == ret) {
        ret = fds_record_open(&g_record_desc, &g_flash_record);
        if(NRF_SUCCESS == ret) {
            memcpy(dest_ptr, g_flash_record.p_data, g_flash_record.p_header->length_words * sizeof(uint32_t));
            ret = fds_record_close(&g_record_desc);
        }
    }

    return ret;
}

ret_code_t custom_record_read_iterate(uint16_t key, void *dest_ptr)
{
    ret_code_t ret = fds_record_find(FILE_ID, key, &g_record_desc, &g_ftok);

    if(NRF_SUCCESS == ret) {
        ret = fds_record_open(&g_record_desc, &g_flash_record);
        if(NRF_SUCCESS == ret) {
            memcpy(dest_ptr, g_flash_record.p_data, g_flash_record.p_header->length_words * sizeof(uint32_t));
            ret = fds_record_close(&g_record_desc);
        }
    }

    return ret;
}

ret_code_t custom_record_update(uint16_t key, void const *src_ptr, size_t size_bytes)
{
    memset(&g_ftok, 0x00, sizeof(fds_find_token_t));

    ret_code_t ret = fds_record_find(FILE_ID, key, &g_record_desc, &g_ftok);

    uint32_t payload[SIZE_IN_WORDS(size_bytes)];

    memcpy(payload, src_ptr, size_bytes);

    if(NRF_SUCCESS == ret) {
        g_record.file_id = FILE_ID;
        g_record.key = key;
        g_record.data.p_data = payload;
        g_record.data.length_words = SIZE_IN_WORDS(size_bytes);

        ret = fds_record_update(&g_record_desc, &g_record);

        if(NRF_SUCCESS == ret) {
            wait_for_complete();
        }
    }

    return ret;
}

ret_code_t custom_record_delete(void)
{
    ret_code_t ret = fds_record_delete(&g_record_desc);

    if(NRF_SUCCESS == ret) {
        wait_for_complete();
    }

    return ret;
}

ret_code_t custom_record_erase(uint16_t file_id)
{
    ret_code_t ret = fds_file_delete(file_id);

    if(NRF_SUCCESS == ret) {
        wait_for_complete();
    }

    return ret;
}