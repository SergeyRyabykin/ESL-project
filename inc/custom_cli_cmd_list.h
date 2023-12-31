#ifndef CUSTOM_CLI_CMD_LIST_H__
#define CUSTOM_CLI_CMD_LIST_H__

#include "custom_cmd.h"
#include "sdk_errors.h"

ret_code_t custom_cmd_rgb_handler(char *cmd, void *context);
ret_code_t custom_cmd_hsv_handler(char *cmd, void *context);
ret_code_t custom_cmd_help_handler(char *cmd, void *context);
ret_code_t custom_cmd_add_rgb_color_handler(char *cmd, void *context);
ret_code_t custom_cmd_add_hsv_color_handler(char *cmd, void *context);
ret_code_t custom_cmd_add_current_color_handler(char *cmd, void *context);
ret_code_t custom_cmd_del_color_handler(char *cmd, void *context);
ret_code_t custom_cmd_apply_color_handler(char *cmd, void *context);
ret_code_t custom_cmd_list_colors_handler(char *cmd, void *context);


static const custom_cmd_t custom_cli_commands[] = {
    {"RGB",               custom_cmd_rgb_handler,               "RGB <red> <green> <blue> - sets RGB color. Max value is 255\r\n"},
    {"HSV",               custom_cmd_hsv_handler,               "HSV <hue> <saturation> <value> - sets HSV color. Hue in degrees others in percents\r\n"},
    {"help",              custom_cmd_help_handler,              "help - shows this information\r\n"},
    {"add_rgb_color",     custom_cmd_add_rgb_color_handler,     "add_rgb_color <r> <g> <b> <color_name> - saves color typed in RGB format into NVM \r\n"},
    {"add_hsv_color",     custom_cmd_add_hsv_color_handler,     "add_hsv_color <h> <s> <v> <color_name> - saves color typed in HSV format into NVM \r\n"},
    // {"add_current_color", custom_cmd_add_current_color_handler, "add_current_color <color_name> - saves curent color into NVM\r\n"},
    {"a", custom_cmd_add_current_color_handler, "add_current_color <color_name> - saves curent color into NVM\r\n"},
    {"del_color",         custom_cmd_del_color_handler,         "del_color <color_name> - removes color from NVM\r\n"},
    {"apply_color",       custom_cmd_apply_color_handler,       "apply_color <color_name> - applys named color if it exists\r\n"},
    {"list_colors",       custom_cmd_list_colors_handler,       "list_colors - shows the list of available colors\r\n"},
};

#endif // CUSTOM_CLI_CMD_LIST_H__