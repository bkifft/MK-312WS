# MK-312WS
ESP32 based webserver as replacement for the HC05/06 inside a MK-312BT.

Can join a WPA2 encrypted WLAN, or be it's own AP.
For now does the knobs, program, ramp and battery level. 
![mk-312ws-gui](https://user-images.githubusercontent.com/6365508/127098461-b83a2a7a-fb84-4c39-a2a6-4bc5771fbe6b.png)

Planned features:
- Ramp parameters.
- Other advanced parameters, like freq and width.
- Powerlevel.
- User program upload.
- MA ramp.

Requirements:
- ESP32 devboard. I recommend the "ESP32 mini", 2x2x10 pins. Like https://www.az-delivery.de/en/products/esp32-d1-mini, does get sold under different brands.
- levelshifter, 5V to 3.3V. Already part of the pcb.
- Arduino IDE with the ESP32 board package and the filesystem uploader (https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/).


Howto:
- Flash the sketch.
- Flash the filesystem.
- Replace the BT module in the mk312. Remember to levelshift RX. Hookup according to plan. [todo]
- Connect to the new AP, pw is 12345678.
- Open http://192.168.4.1/config. Change the SSID if you want to continue AP mode, SSID and password to connect to a preexisting network.
- Open http://mk-312ws.local and start zapping.
