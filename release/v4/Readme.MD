Warning:
If you are running an older verion, you will need to do a USB based flash first, as I had to change the partitioning scheme.
Details on how to do this will follow.

Short version:

Windows:
- Get Flash Download Tools from https://www.espressif.com/en/support/download/other-tools
- Connect board using microUSB cable
- In tool select esp32, develop, ok.
- In first line tick checkbox, open USB-flash.bin, enter 0x0 (that's zeroxzero) into last field.
- Select virtual com port in bottom right
- Click Start