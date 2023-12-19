#ifndef CUSTOM_CLI_H__
#define CUSTOM_CLI_H__

#include "sdk_errors.h"
#include "custom_app_types.h"

ret_code_t custom_cli_init(const custom_app_ctx_t *app_ctx);
ret_code_t custom_cli_print(char *str);

#endif // CUSTOM_CLI_H__