Warning:
If you are running an older version (no version information at bottom of pages), you will need to do a USB based flash first, as I had to change the partitioning scheme.
Details on how to do this will follow.

Short version:

Windows:
- Get Flash Download Tools from https://www.espressif.com/en/support/download/other-tools
- Connect board using microUSB cable
- In tool select esp32, develop, ok.
- In first line tick checkbox, open USB-flash.bin, enter 0x0 (that's zeroxzero) into last field.
- Select virtual com port in bottom right
- Click Start

esptool.py:
- Install esptool.py. Available via apt, pip, homebrew and otheer package managers.
- Connect board using microUSB cable
- run 
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800  --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0x0 USB-flash.bin 

