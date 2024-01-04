#ifndef CUSTOM_NVM_H__
#define CUSTOM_NVM_H__

#include <stddef.h>
#include <limits.h>

#include "sdk_errors.h"

#define CUSTOM_NVM_SIZE_IN_WORDS(size_in_bytes) (size_in_bytes / sizeof(uint32_t) + ((size_in_bytes % sizeof(uint32_t)) ? 1 : 0))
#define CUSTOM_NVM_SIZE_IN_BYTES(size_in_bytes) (CUSTOM_NVM_SIZE_IN_WORDS(size_in_bytes) * sizeof(uint32_t))
#define CUSTOM_PAYLOAD_MAX_SIZE UCHAR_MAX 
typedef uint8_t custom_nvm_payload_id_t;

void custom_nvm_erase(void);
uintptr_t custom_nvm_find(const custom_nvm_payload_id_t id);
uintptr_t custom_nvm_find_next(const uintptr_t current_object_addr, const custom_nvm_payload_id_t id);
ret_code_t custom_nvm_discard_by_id(const custom_nvm_payload_id_t id);
ret_code_t custom_nvm_discard(const uintptr_t object_addr);
ret_code_t custom_nvm_save(const void *object, const size_t object_size, const custom_nvm_payload_id_t id);

#endif // CUSTOM_NVM_H__