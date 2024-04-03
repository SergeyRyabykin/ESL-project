#include "custom_storage.h"
#include "fds.h"

#define SIZE_IN_WORDS(size_in_bytes) (size_in_bytes / sizeof(uint32_t) + ((size_in_bytes % sizeof(uint32_t)) ? 1 : 0))

#define FILE_ID 0x1

static fds_record_t record;
static fds_record_desc_t record_desc;

static void custom_fds_evt_handler(fds_evt_t const *p_fds_evt)
{
    switch (p_fds_evt->id) {
        case FDS_EVT_INIT:
            break;
        case FDS_EVT_WRITE:
            break;
        case FDS_EVT_UPDATE:
            break;
        case FDS_EVT_DEL_RECORD:
            break;
        case FDS_EVT_DEL_FILE:
            break;
        case FDS_EVT_GC:
            break;
        default:
            break;
    }
}

ret_code_t custom_storage_init(void)
{
    ret_code_t ret = fds_register(custom_fds_evt_handler);

    if(NRF_SUCCESS != ret) {
        return ret;
    }
    ret = fds_init();

    return ret;
}

ret_code_t custom_storage_save(uint16_t key, void *data_ptr, size_t size_bytes)
{
    record.file_id = FILE_ID;
    record.key = key;
    record.data.p_data = data_ptr;
    record.data.length_words = SIZE_IN_WORDS(size_bytes);

    ret_code_t ret = fds_record_write(&record_desc, &record);

    return ret;
}

// ret_code_t custom_storage_find(uint16_t key, )