#include "custom_record.h"
#include <string.h>

#define SIZE_IN_WORDS(size_in_bytes) (size_in_bytes / sizeof(uint32_t) + ((size_in_bytes % sizeof(uint32_t)) ? 1 : 0))

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

    return ret;
}

ret_code_t custom_record_save(custom_record_t *record, void const *src_ptr, size_t size_bytes)
{
    uint32_t payload[SIZE_IN_WORDS(size_bytes)];

    memcpy(payload, src_ptr, size_bytes);

    record->record.data.p_data = payload;
    record->record.data.length_words = SIZE_IN_WORDS(size_bytes);

    ret_code_t ret = fds_record_write(&record->record_desc, &record->record);

    if(NRF_SUCCESS == ret) {
        wait_for_complete();
    }
    else if (FDS_ERR_NO_SPACE_IN_FLASH == ret) {
        fds_gc();
        wait_for_complete();
        ret = fds_record_write(&record->record_desc, &record->record);
        wait_for_complete();
    }

    return ret;
}

ret_code_t custom_record_read(custom_record_t * const record, void *dest_ptr)
{
    memset(&record->ftok, 0x00, sizeof(fds_find_token_t));

    ret_code_t ret = fds_record_find(record->record.file_id, record->record.key, &record->record_desc, &record->ftok);

    if(NRF_SUCCESS == ret) {
        ret = fds_record_open(&record->record_desc, &record->flash_record);
        if(NRF_SUCCESS == ret) {
            memcpy(dest_ptr, record->flash_record.p_data, record->flash_record.p_header->length_words * sizeof(uint32_t));
            ret = fds_record_close(&record->record_desc);
        }
    }

    return ret;
}

ret_code_t custom_record_read_iterate(custom_record_t * const record, void *dest_ptr)
{
    ret_code_t ret = fds_record_find(record->record.file_id, record->record.key, &record->record_desc, &record->ftok);

    if(NRF_SUCCESS == ret) {
        ret = fds_record_open(&record->record_desc, &record->flash_record);
        if(NRF_SUCCESS == ret) {
            memcpy(dest_ptr, record->flash_record.p_data, record->flash_record.p_header->length_words * sizeof(uint32_t));
            ret = fds_record_close(&record->record_desc);
        }
    }

    return ret;
}

ret_code_t custom_record_update(custom_record_t * const record, void const *src_ptr, size_t size_bytes)
{
    memset(&record->ftok, 0x00, sizeof(fds_find_token_t));

    ret_code_t ret = fds_record_find(record->record.file_id, record->record.key, &record->record_desc, &record->ftok);

    uint32_t payload[SIZE_IN_WORDS(size_bytes)];

    memcpy(payload, src_ptr, size_bytes);

    if(NRF_SUCCESS == ret) {
        record->record.data.p_data = payload;
        record->record.data.length_words = SIZE_IN_WORDS(size_bytes);

        ret = fds_record_update(&record->record_desc, &record->record);

        if(NRF_SUCCESS == ret) {
            wait_for_complete();
        }
        else if (FDS_ERR_NO_SPACE_IN_FLASH == ret) {
            fds_gc();
            wait_for_complete();
            ret = fds_record_write(&record->record_desc, &record->record);
            wait_for_complete();
        }
    }

    return ret;
}

ret_code_t custom_record_delete(custom_record_t * const record)
{
    ret_code_t ret = fds_record_delete(&record->record_desc);

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