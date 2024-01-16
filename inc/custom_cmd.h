#ifndef CUSTOM_CMD_H__
#define CUSTOM_CMD_H__

#include <stdint.h>
#include "sdk_errors.h"

#define CUSTOM_CMD_NAME_LENGTH 20
#define CUSTOM_CMD_MAX_NUM 10
#define CUSTOM_CMD_STR_LENGTH 64

typedef ret_code_t (*custom_cmd_executor_t)(char *cmd, void *context);

typedef struct {
    char cmd_name[CUSTOM_CMD_NAME_LENGTH];
    custom_cmd_executor_t cmd_execute;
    const char * cmd_description;
} custom_cmd_t;

typedef struct {
    custom_cmd_t *commands;
    unsigned int number_commands;
} custom_cmd_ctx_t;

typedef struct {
    custom_cmd_t *cmd;
    char cmd_str[CUSTOM_CMD_STR_LENGTH];
    void *context;
} custom_cmd_executor_ctx_t;

#define CUSTOM_CMD_INIT_LIST(commands_list) \
{\
    .commands = (custom_cmd_t *)commands_list,\
    .number_commands = ARRAY_SIZE(commands_list)\
}

ret_code_t custom_cmd_init(const char *name, const custom_cmd_executor_t execute, const char * description, custom_cmd_ctx_t *context);
ret_code_t custom_cmd_init_all(const unsigned int size, const custom_cmd_t commands[size], custom_cmd_ctx_t *context);
ret_code_t custom_cmd_execute(char *cmd_str, const custom_cmd_ctx_t *cmd_context, void *app_context);
ret_code_t custom_cmd_get_cmd_executor( custom_cmd_executor_ctx_t *executor_ctx,
                                        char *cmd_str, 
                                        const custom_cmd_ctx_t *cmd_context, 
                                        void *app_context );


#endif // CUSTOM_CMD_H__