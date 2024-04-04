#ifndef CUSTOM_RECORD_H
#define CUSTOM_RECORD_H

#include <stddef.h>
#include "sdk_errors.h"

#define FILE_ID 0x1

ret_code_t custom_record_storage_init(void);
ret_code_t custom_record_save(uint16_t key, void const *src_ptr, size_t size_bytes);
ret_code_t custom_record_read(uint16_t key, void *dest_ptr);
ret_code_t custom_record_read_iterate(uint16_t key, void *dest_ptr);
ret_code_t custom_record_update(uint16_t key, void const *src_ptr, size_t size_bytes);
ret_code_t custom_record_delete(void);
ret_code_t custom_record_erase(uint16_t file_id);


#endif // CUSTOM_RECORD_H