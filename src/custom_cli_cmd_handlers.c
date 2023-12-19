#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>

#include "sdk_errors.h"
#include "custom_app_defines.h"

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

    app_ctx->custom_hsv_ctx.color.hue = cmd_args[0];
    app_ctx->custom_hsv_ctx.color.saturation = cmd_args[1];
    app_ctx->custom_hsv_ctx.color.value = cmd_args[2];

    uint8_t r, g, b;
    custom_hsv_to_rgb(&app_ctx->custom_hsv_ctx.color, &r, &g, &b);
    app_ctx->pwm_values.channel_1 = CUSTOM_RGB_STEP * r;
    app_ctx->pwm_values.channel_2 = CUSTOM_RGB_STEP * g;
    app_ctx->pwm_values.channel_3 = CUSTOM_RGB_STEP * b;

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


    app_ctx->pwm_values.channel_1 = CUSTOM_RGB_STEP * cmd_args[0];
    app_ctx->pwm_values.channel_2 = CUSTOM_RGB_STEP * cmd_args[1];
    app_ctx->pwm_values.channel_3 = CUSTOM_RGB_STEP * cmd_args[2];

    custom_rgb_to_hsv( &app_ctx->custom_hsv_ctx.color, cmd_args[0], cmd_args[1], cmd_args[2]);

    return NRF_SUCCESS;
}

ret_code_t custom_cmd_help_handler(char *str, void *context)
{
    custom_app_ctx_t *app_ctx = (custom_app_ctx_t *)context;

    //  TODO: There might be overflow of queue
    for(unsigned int i = 0; i < app_ctx->custom_cmd_ctx->number_commands; i++) {
        custom_queue_add(&app_ctx->custom_queue_output, (char *)app_ctx->custom_cmd_ctx->commands[i].cmd_description);
    }

    return NRF_SUCCESS;
}