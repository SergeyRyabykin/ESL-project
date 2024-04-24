The purpose of the application is to control parameters of RGB led such as color, saturation and value. The application was developed for PCA10059 board by Nordic Semiconductor which as based on NRF52840 chip.
The color parameters might be changed in three ways:
1. Button
  - Double click changes the parameter. The yellow auxiliary led indicates the current mode: slow blinking - color, fast blinking - saturation, always on - value and always off - default mode(no parameter).
  - Keeping the button pressed changes the parameter value.
When the application enters default mode the color is saved to non volatile memory and will be implemented as default color (will be set after power on or reset)
2. BLE
  There is three characteristics:
  - The first is write only. The led state might be changed through writing appropriate command: RGB <red> <green> <blue> or HSV <hue> <saturation> <value>. To save the led state to NVM the 'save' command might be implemented. The commands must be in the text format.
  - The second is read only. The led state in HSV format is shown in this characteristic.
  - The third is also read only. The applied command result is shown in this one (Success, Unknown command or Arguments error).
3. USB CLI
  This extension can be included into the application with 'ESTC_USB_CLI_ENABLED=1' parameter added to make call. 
  The easiest way to learn all the available commands is to implement 'help' command in the CLI terminal.
