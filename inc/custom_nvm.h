#ifndef CUSTOM_NVM_H__
#define CUSTOM_NVM_H__

#include <stddef.h>
#include "sdk_errors.h"

ret_code_t custom_nvm_init(void *obj, size_t obj_size);
ret_code_t custom_nvm_save_obj(const void *obj, size_t obj_size);

#endif // CUSTOM_NVM_H__