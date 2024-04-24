The purpose of the application is to control parameters of RGB led such as hue, saturation and value. The application was developed for PCA10059 board which is based on NRF52840 chip.
<br />The color parameters might be changed in three ways:
1. Button
  - Double click changes a parameter. The auxiliary led indicates the current parameter to be changed: slow blinking - hue, fast blinking - saturation, always on - value and always off - default mode(no parameter).
  - Keeping the button pressed changes the parameter value.
<br />When the application enters default mode the color is saved into non volatile memory and will be implemented as default color (This led state will be implemented after power on or reset).
2. BLE
  <br />There are three characteristics:
  - The first is write only. The led state might be changed through writing an appropriate command to this characteristic:
    <br />  RGB \<red> \<green> \<blue>
    <br />  HSV \<hue> \<saturation> \<value>
    <br />  save - to save the led state into NVM and use it as the default state.
    <br />The commands must be in the text format.
  - The second is read only. The led state in HSV format is shown in this characteristic. It updates the actual state info each time the parameters of the color are changed.
  - The third is also read only. The applied command result is shown in this one (Success, Unknown command or Arguments error).
3. USB CLI
  <br />This extension can be included into the application with 'ESTC_USB_CLI_ENABLED=1' parameter added to MAKE invocation. 
  <br />The easiest way to learn all the available commands is to implement 'help' command in the CLI terminal.
