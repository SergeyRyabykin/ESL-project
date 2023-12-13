
#include <stdbool.h>
#include <string.h>
#include "custom_cmd.h"

#define CUSTOM_CMD_DELIMITER ' '

static ret_code_t custom_cmd_get_name(char *name, char *cmd)
{
    size_t i = 0;

    if(CUSTOM_CMD_DELIMITER == *cmd || '\0' ==  *cmd) {
        return NRF_ERROR_INVALID_PARAM;
    }

    for(i = 0; CUSTOM_CMD_NAME_LENGTH > i; i++) {
        if(CUSTOM_CMD_DELIMITER != *(cmd + i) && '\0' !=  *(cmd + i)) {
            name[i] = cmd[i];
        }
        else {
            break;
        }
    }

    if(CUSTOM_CMD_NAME_LENGTH == i) {
        return NRF_ERROR_INVALID_PARAM;
    }

    return NRF_SUCCESS;
}

static const custom_cmd_t *custom_cmd_get_cmd(const char *name, const custom_cmd_ctx_t *context)
{
    for(unsigned int i = 0; context->number_commands > i; i++) {
        if(!strcmp(context->commands[i].cmd_name, name)) {
            return &context->commands[i];
        }
    }

    return NULL;
}

ret_code_t custom_cmd_init(const char *name, custom_cmd_executor_t execute, const char * description, custom_cmd_ctx_t *context)
{
    ret_code_t ret = NRF_ERROR_NO_MEM;

    if(CUSTOM_CMD_MAX_NUM > context->number_commands) {
        strcpy(context->commands[context->number_commands].cmd_name, name);
        context->commands[context->number_commands].cmd_execute = execute;
        context->commands[context->number_commands].cmd_description = description;
        context->number_commands++;
        ret = NRF_SUCCESS;
    }

    return ret;
}

ret_code_t custom_cmd_init_all(const unsigned int size, custom_cmd_t commands[size], custom_cmd_ctx_t *context)
{
    ret_code_t ret = NRF_SUCCESS;
    for(unsigned int i = 0; size > i; i++) {
        ret = custom_cmd_init(context->commands[i].cmd_name, context->commands[i].cmd_execute, context->commands[i].cmd_description, context);
        if(NRF_SUCCESS != ret) {
            break;
        }

    }

    return ret;
}

ret_code_t custom_cmd_execute(char *cmd_str, const custom_cmd_ctx_t *context)
{
    ret_code_t ret;
    char name[CUSTOM_CMD_NAME_LENGTH] = "\0";

    ret = custom_cmd_get_name(name, cmd_str);
    if(NRF_SUCCESS != ret) {
        return ret;
    }

    // TODO: This function damages the cmd string. Resolve this problem for the future.
    const custom_cmd_t *cmd = custom_cmd_get_cmd(name, context);

    if(NULL != cmd) {
        ret = cmd->cmd_execute(cmd_str);
    }
    else {
        ret = NRF_ERROR_NOT_FOUND;
    }

    return ret;
}