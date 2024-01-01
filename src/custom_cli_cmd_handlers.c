#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>

#include "sdk_errors.h"
#include "custom_app_defines.h"
#include "custom_app_types.h"
#include "custom_nvm.h"

#include "custom_log.h"

typedef struct {
    custom_hsv_t color;
    char name[CUSTOM_PAYLOAD_MAX_SIZE - CUSTOM_NVM_SIZE_IN_BYTES(sizeof(custom_hsv_t))];
} custom_saved_color_t;

static bool is_number(const char *str)
{
    for(size_t idx = 0; '\0' != str[idx]; idx++) {
        if(!isdigit((int)str[idx])) {
            return false;
        }
    }
    return true;
}

ret_code_t custom_cmd_hsv_handler(char *str,  void *context)
{
    custom_app_ctx_t *app_ctx = (custom_app_ctx_t *)context;
    unsigned int arg_cnt = 0;
    char *token = strtok(str, " ");
    uint16_t cmd_args[3] = {0};

    while((token = strtok(NULL, " "))) {
        if(is_number(token) && 3 > arg_cnt) {
            int arg = atoi(token);
            switch (arg_cnt) {
                case 0: {
                    if(0 <= arg && 360 > arg) {
                        cmd_args[arg_cnt] = arg;
                    }
                    else {
                        return NRF_ERROR_INVALID_PARAM;
                    }
                }; 
                    break;
                case 1:
                case 2: {
                    if(0 <= arg && 100 >= arg) {
                        cmd_args[arg_cnt] = arg;
                    }
                    else {
                        return NRF_ERROR_INVALID_PARAM;
                    }
                }
                    break;
                
                default:
                    break;
                }
        }
        else {
            return NRF_ERROR_INVALID_PARAM;
        }

        arg_cnt++;
    }
   

    if(3 != arg_cnt) {
        return NRF_ERROR_INVALID_PARAM;
    }

    app_ctx->custom_hsv_ctx->color.hue = cmd_args[0];
    app_ctx->custom_hsv_ctx->color.saturation = cmd_args[1];
    app_ctx->custom_hsv_ctx->color.value = cmd_args[2];

    uint8_t r, g, b;
    custom_hsv_to_rgb(&app_ctx->custom_hsv_ctx->color, &r, &g, &b);
    app_ctx->pwm_values->channel_1 = CUSTOM_RGB_STEP * r;
    app_ctx->pwm_values->channel_2 = CUSTOM_RGB_STEP * g;
    app_ctx->pwm_values->channel_3 = CUSTOM_RGB_STEP * b;

    return NRF_SUCCESS;
}

ret_code_t custom_cmd_rgb_handler(char *str, void *context)
{
    custom_app_ctx_t *app_ctx = (custom_app_ctx_t *)context;
    unsigned int arg_cnt = 0;
    char *token = strtok(str, " ");
    uint8_t cmd_args[3] = {0};

    while((token = strtok(NULL, " "))) {
        if(is_number(token) && 3 > arg_cnt) {
            int arg = atoi(token);
            if(0 <= arg && 255 >= arg) {
                cmd_args[arg_cnt] = arg;
            }
            else {
                return NRF_ERROR_INVALID_PARAM;
            }
        }
        else {
            return NRF_ERROR_INVALID_PARAM;
        }

        arg_cnt++;
    }
   

    if(3 != arg_cnt) {
        return NRF_ERROR_INVALID_PARAM;
    }


    app_ctx->pwm_values->channel_1 = CUSTOM_RGB_STEP * cmd_args[0];
    app_ctx->pwm_values->channel_2 = CUSTOM_RGB_STEP * cmd_args[1];
    app_ctx->pwm_values->channel_3 = CUSTOM_RGB_STEP * cmd_args[2];

    custom_rgb_to_hsv( &app_ctx->custom_hsv_ctx->color, cmd_args[0], cmd_args[1], cmd_args[2]);

    return NRF_SUCCESS;
}

ret_code_t custom_cmd_help_handler(char *str, void *context)
{
    custom_app_ctx_t *app_ctx = (custom_app_ctx_t *)context;

    for(unsigned int i = 0; i < app_ctx->custom_cmd_ctx->number_commands; i++) {
        while(NRF_SUCCESS != app_ctx->custom_print_output((char *)app_ctx->custom_cmd_ctx->commands[i].cmd_description)) {
            ;
        }
    }

    return NRF_SUCCESS;
}

ret_code_t custom_cmd_add_rgb_color_handler(char *str, void *context)
{
    custom_saved_color_t object = {.color.hue = 0};
    unsigned int arg_cnt = 0;
    char *token = strtok(str, " ");
    uint8_t cmd_args[3] = {0};

    while((token = strtok(NULL, " "))) {
        if(is_number(token) && 3 > arg_cnt) {
            int arg = atoi(token);
            if(0 <= arg && 255 >= arg) {
                cmd_args[arg_cnt] = arg;
            }
            else {
                return NRF_ERROR_INVALID_PARAM;
            }
        }
        else if(3 == arg_cnt) {
            strcpy(object.name, token);
        }
        else {
            return NRF_ERROR_INVALID_PARAM;
        }

        arg_cnt++;
    }
   

    if(4 != arg_cnt) {
        return NRF_ERROR_INVALID_PARAM;
    }

    custom_rgb_to_hsv( &object.color, cmd_args[0], cmd_args[1], cmd_args[2]);

    return custom_nvm_save(&object, sizeof(object), SAVED_COLOR_ID);
}

ret_code_t custom_cmd_add_hsv_color_handler(char *str, void *context)
{
    custom_saved_color_t object = {.color.hue = 0};
    unsigned int arg_cnt = 0;
    char *token = strtok(str, " ");
    uint16_t cmd_args[3] = {0};

    while((token = strtok(NULL, " "))) {
        if(is_number(token) && 3 > arg_cnt) {
            int arg = atoi(token);
            switch (arg_cnt) {
                case 0: {
                    if(0 <= arg && 360 > arg) {
                        cmd_args[arg_cnt] = arg;
                    }
                    else {
                        return NRF_ERROR_INVALID_PARAM;
                    }
                }; 
                    break;
                case 1:
                case 2: {
                    if(0 <= arg && 100 >= arg) {
                        cmd_args[arg_cnt] = arg;
                    }
                    else {
                        return NRF_ERROR_INVALID_PARAM;
                    }
                }
                    break;
                
                default:
                    break;
                }
        }
        else if(3 == arg_cnt) {
            strcpy(object.name, token);
        }
        else {
            return NRF_ERROR_INVALID_PARAM;
        }

        arg_cnt++;
    }
   

    if(4 != arg_cnt) {
        return NRF_ERROR_INVALID_PARAM;
    }

    object.color.hue = cmd_args[0];
    object.color.saturation = cmd_args[1];
    object.color.value = cmd_args[2];

    return custom_nvm_save(&object, sizeof(object), SAVED_COLOR_ID);
}

ret_code_t custom_cmd_add_current_color_handler(char *str, void *context)
{
    custom_app_ctx_t *app_ctx = (custom_app_ctx_t *)context;
    custom_saved_color_t object = {.color.hue = 0};
    char *token = strtok(str, " "); // Read the command name
    token = strtok(NULL, " "); // Read the argument

    if(!token || strtok(NULL, " ")) {
        return NRF_ERROR_INVALID_PARAM;
    }

    strcpy(object.name, token);

    object.color.hue = app_ctx->custom_hsv_ctx->color.hue;
    object.color.saturation = app_ctx->custom_hsv_ctx->color.saturation;
    object.color.value = app_ctx->custom_hsv_ctx->color.value;

    NRF_LOG_INFO("Saving...");
    ret_code_t ret = custom_nvm_save(&object, sizeof(object), SAVED_COLOR_ID);

    NRF_LOG_INFO("RET: %x", ret);
    return ret;
}

ret_code_t custom_cmd_del_color_handler(char *str, void *context)
{
    ret_code_t ret = NRF_ERROR_NOT_FOUND;
    char *token = strtok(str, " ");
    token = strtok(NULL, " "); // Read the argument

    if(!token || strtok(NULL, " ")) {
        return NRF_ERROR_INVALID_PARAM;
    }

    uintptr_t object = custom_nvm_find(SAVED_COLOR_ID);

    while(object && strcmp(((custom_saved_color_t *)object)->name, token)) {
        object = custom_nvm_find_next(object, SAVED_COLOR_ID);
    }

    if(object) {
        ret = custom_nvm_discard(object);
    }

    return ret;
}

ret_code_t custom_cmd_apply_color_handler(char *str, void *context)
{
    custom_app_ctx_t *app_ctx = (custom_app_ctx_t *)context;
    ret_code_t ret = NRF_ERROR_NOT_FOUND;
    char *token = strtok(str, " "); // Read the command name
    token = strtok(NULL, " "); // Read the argument

    if(!token || strtok(NULL, " ")) {
        return NRF_ERROR_INVALID_PARAM;
    }

    uintptr_t object = custom_nvm_find(SAVED_COLOR_ID);

    while(object && strcmp(((custom_saved_color_t *)object)->name, token)) {
        object = custom_nvm_find_next(object, SAVED_COLOR_ID);
    }

    if(object) {
        app_ctx->custom_hsv_ctx->color.hue = ((custom_saved_color_t *)object)->color.hue;
        app_ctx->custom_hsv_ctx->color.saturation = ((custom_saved_color_t *)object)->color.saturation;
        app_ctx->custom_hsv_ctx->color.value = ((custom_saved_color_t *)object)->color.value;
    
        uint8_t r, g, b;
        custom_hsv_to_rgb(&app_ctx->custom_hsv_ctx->color, &r, &g, &b);
        app_ctx->pwm_values->channel_1 = CUSTOM_RGB_STEP * r;
        app_ctx->pwm_values->channel_2 = CUSTOM_RGB_STEP * g;
        app_ctx->pwm_values->channel_3 = CUSTOM_RGB_STEP * b;

        ret = NRF_SUCCESS;
    }

    return ret;
}

ret_code_t custom_cmd_list_colors_handler(char *str, void *context)
{
    // custom_app_ctx_t *app_ctx = (custom_app_ctx_t *)context;

    // for(unsigned int i = 0; i < app_ctx->custom_cmd_ctx->number_commands; i++) {
    //     while(NRF_SUCCESS != app_ctx->custom_print_output((char *)app_ctx->custom_cmd_ctx->commands[i].cmd_description)) {
    //         ;
    //     }
    // }

    return NRF_SUCCESS;
}


