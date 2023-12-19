#ifndef CUSTOM_CLI_CMD_LIST_H__
#define CUSTOM_CLI_CMD_LIST_H__

#include "custom_cmd.h"
#include "sdk_errors.h"


ret_code_t custom_cmd_rgb_handler(char *cmd, void *context);
ret_code_t custom_cmd_hsv_handler(char *cmd, void *context);
ret_code_t custom_cmd_help_handler(char *cmd, void *context);

static const custom_cmd_t custom_cli_commands[] = {
    {"RGB", custom_cmd_rgb_handler, "RGB <red> <green> <blue> - sets RGB color. Max value is 255\r\n"},
    {"HSV", custom_cmd_hsv_handler, "HSV <hue> <saturation> <value> - sets HSV color. Hue in degrees others in percents\r\n"},
    {"help", custom_cmd_help_handler, "help - shows this information\r\n"},
};

#endif // CUSTOM_CLI_CMD_LIST_H__