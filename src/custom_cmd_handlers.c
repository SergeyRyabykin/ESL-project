#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>


#include "sdk_errors.h"
#include "custom_app_defines.h"
#include "custom_app_types.h"

#include "custom_record.h"
#include "custom_log.h"


#define CUSTOM_COLOR_NAME_MAX_LENGTH 50
#define CUSTOM_PAYLOAD_MAX_SIZE UCHAR_MAX 


// TODO: Move this variable to context argument
static custom_record_t g_saved_record = {
    .record.file_id = FILE_ID,
    .record.key = SAVED_COLOR_ID,
};

typedef struct {
    custom_hsv_t color;
    char name[CUSTOM_COLOR_NAME_MAX_LENGTH];
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

    custom_hsv_apply_color(&app_ctx->custom_hsv_ctx->color, app_ctx->pwm_values);

    if(app_ctx->custom_app_callback){
        app_ctx->custom_app_callback(NULL);
    }

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

    if(app_ctx->custom_app_callback){
        app_ctx->custom_app_callback(NULL);
    }

    return NRF_SUCCESS;
}

ret_code_t custom_cmd_save_handler(char *str, void *context)
{
    custom_app_ctx_t *app_ctx = (custom_app_ctx_t *)context;

    strtok(str, " "); // Read the command name
    
    if(strtok(NULL, " ")) {
        return NRF_ERROR_INVALID_PARAM;
    }

    return custom_record_update(app_ctx->default_record, &app_ctx->custom_hsv_ctx->color, sizeof(app_ctx->custom_hsv_ctx->color));
}

ret_code_t custom_cmd_help_handler(char *str, void *context)
{
    custom_app_ctx_t *app_ctx = (custom_app_ctx_t *)context;

    strtok(str, " "); // Read the command name

    if(strtok(NULL, " ")) {
        return NRF_ERROR_INVALID_PARAM;
    }

    for(unsigned int i = 0; i < app_ctx->custom_cmd_ctx->number_commands; i++) {
        while(NRF_SUCCESS != app_ctx->custom_print_output((char *)app_ctx->custom_cmd_ctx->commands[i].cmd_description)) {
            // To process the usb queue if the custom_queue is full
            LOG_BACKEND_USB_PROCESS();
            NRF_LOG_PROCESS();
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

    return custom_record_save(&g_saved_record, &object, sizeof(object));
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

    return custom_record_save(&g_saved_record, &object, sizeof(object));
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

    ret_code_t ret = custom_record_save(&g_saved_record, &object, sizeof(object));

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

    uint8_t object[CUSTOM_PAYLOAD_MAX_SIZE] = {0};
    ret = custom_record_read(&g_saved_record, object);

    while(NRF_SUCCESS == ret && strcmp(((custom_saved_color_t *)object)->name, token)) {
        ret = custom_record_read_iterate(&g_saved_record, object);
    }

    if(NRF_SUCCESS == ret) {
        ret = custom_record_delete(&g_saved_record);
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

    uint8_t object[CUSTOM_PAYLOAD_MAX_SIZE] = {0};
    ret = custom_record_read(&g_saved_record, object);

    while(NRF_SUCCESS == ret && strcmp(((custom_saved_color_t *)object)->name, token)) {
        ret = custom_record_read_iterate(&g_saved_record, object);
    }

    if(NRF_SUCCESS == ret) {
        app_ctx->custom_hsv_ctx->color.hue = ((custom_saved_color_t *)object)->color.hue;
        app_ctx->custom_hsv_ctx->color.saturation = ((custom_saved_color_t *)object)->color.saturation;
        app_ctx->custom_hsv_ctx->color.value = ((custom_saved_color_t *)object)->color.value;
    
        custom_hsv_apply_color(&app_ctx->custom_hsv_ctx->color, app_ctx->pwm_values);

        if(app_ctx->custom_app_callback){
            app_ctx->custom_app_callback(NULL);
        }

        ret = NRF_SUCCESS;
    }

    return ret;
}

ret_code_t custom_cmd_list_colors_handler(char *str, void *context)
{
    custom_app_ctx_t *app_ctx = (custom_app_ctx_t *)context;
    char *token = strtok(str, " "); // Read the command name

    if(!token || strtok(NULL, " ")) {
        return NRF_ERROR_INVALID_PARAM;
    }

    uint8_t object[CUSTOM_PAYLOAD_MAX_SIZE] = {0};
    ret_code_t ret = custom_record_read(&g_saved_record, object);

    while(NRF_SUCCESS == ret) {
        while(NRF_SUCCESS != app_ctx->custom_print_output(((custom_saved_color_t *)object)->name)) {
            ;
        }
        while(NRF_SUCCESS != app_ctx->custom_print_output("\n\r")) {
            ;
        }

        ret = custom_record_read_iterate(&g_saved_record, object);
    }

    return NRF_SUCCESS;
}


