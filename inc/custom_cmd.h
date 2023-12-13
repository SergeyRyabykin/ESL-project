#ifndef CUSTOM_CMD_H__
#define CUSTOM_CMD_H__

#include <stdint.h>
#include "sdk_errors.h"

#define CUSTOM_CMD_NAME_LENGTH 20
#define CUSTOM_CMD_MAX_NUM 10
#define CUSTOM_CMD_STR_LENGTH 64

typedef ret_code_t (*custom_cmd_executor_t)(char *cmd);

typedef struct {
    char cmd_name[CUSTOM_CMD_NAME_LENGTH];
    custom_cmd_executor_t cmd_execute;
    const char * cmd_description;
} custom_cmd_t;

typedef struct {
    custom_cmd_t commands[CUSTOM_CMD_MAX_NUM];
    unsigned int number_commands;
} custom_cmd_ctx_t;

ret_code_t custom_cmd_init(const char *name, custom_cmd_executor_t execute, const char * description, custom_cmd_ctx_t *context);
ret_code_t custom_cmd_init_all(const unsigned int size, custom_cmd_t commands[size], custom_cmd_ctx_t *context);
ret_code_t custom_cmd_execute(char *cmd_str, const custom_cmd_ctx_t *context);


#endif // CUSTOM_CMD_H__