#ifndef CUSTOM_RECORD_H
#define CUSTOM_RECORD_H

#include <stddef.h>
#include "sdk_errors.h"
#include "fds.h"

#define FILE_ID 0x1

typedef struct {
    fds_record_t record;
    fds_record_desc_t record_desc;
    fds_flash_record_t flash_record;
    fds_find_token_t ftok;
} custom_record_t;

ret_code_t custom_record_storage_init(void);
ret_code_t custom_record_save(custom_record_t *record, void const *src_ptr, size_t size_bytes);
ret_code_t custom_record_read(custom_record_t * const record, void *dest_ptr);
ret_code_t custom_record_read_iterate(custom_record_t * const record, void *dest_ptr);
ret_code_t custom_record_update(custom_record_t * const record, void const *src_ptr, size_t size_bytes);
ret_code_t custom_record_delete(custom_record_t * const record);
ret_code_t custom_record_erase(uint16_t file_id);


#endif // CUSTOM_RECORD_H