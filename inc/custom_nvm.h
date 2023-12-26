#ifndef CUSTOM_NVM_H__
#define CUSTOM_NVM_H__

#include <stddef.h>
#include "sdk_errors.h"

typedef uint8_t custom_nvm_payload_id_t;

void custom_nvm_erase(void);
uintptr_t custom_nvm_find(const custom_nvm_payload_id_t id);
ret_code_t custom_nvm_discard(const custom_nvm_payload_id_t id);
ret_code_t custom_nvm_save(const void *object, const size_t object_size, const custom_nvm_payload_id_t id);

#endif // CUSTOM_NVM_H__