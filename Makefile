sketch      := MK-312WS.ino
CORE        := esp32:esp32
boardconfig := "${CORE}:d1_mini32:FlashFreq=80,PartitionScheme=min_spiffs,CPUFreq=240,UploadSpeed=460800"

ARDUINO_CLI ?= arduino-cli
MKSPIFFS    ?= mkspiffs
BC          ?= bc

PARTITION_TABLE=~/.arduino15/packages/esp32/hardware/esp32/1.0.6/tools/partitions/min_spiffs.csv

DEVICE :=/dev/ttyACM0
DEVICE :=/dev/ttyUSB0

.PHONY: factory
factory: | erase_flash flash-fs flash

.PHONY: erase_flash
erase_flash:
	python ~/.arduino15/packages/esp32/tools/esptool_py/3.0.0/esptool.py --chip esp32 \
	  --port ${DEVICE} \
	  --baud 460800 \
	erase_flash

.PHONY: build
build: filesystem.bin
	$(ARDUINO_CLI) compile -e -v --fqbn $(boardconfig) $(sketch)

.PHONY: flash
flash:
	$(ARDUINO_CLI) upload -v -p ${DEVICE} --fqbn ${boardconfig} ${sketch} 

.PHONY: filesystem.bin
.ONESHELL:
filesystem.bin:
	PROPS=$$($(ARDUINO_CLI) compile --fqbn $(boardconfig) --show-properties)
	BUILD_SPIFFS_BLOCKSIZE=4096
	BUILD_SPIFFS_PAGESIZE=256
	BUILD_SPIFFS_START_HEX=$$(cat ${PARTITION_TABLE} | grep "^spiffs"|cut -d, -f4 | xargs)
	BUILD_SPIFFS_START=$$(echo "ibase=16;$${BUILD_SPIFFS_START_HEX#??}"|bc -q)
	echo "BUILD_SPIFFS_START $$BUILD_SPIFFS_START_HEX ($$BUILD_SPIFFS_START)"
	BUILD_SPIFFS_SIZE_HEX=$$(cat ${PARTITION_TABLE} | grep "^spiffs"|cut -d, -f5 | xargs)
	BUILD_SPIFFS_SIZE=$$(echo "ibase=16;$${BUILD_SPIFFS_SIZE_HEX#??}"|bc -q)
	echo "BUILD_SPIFFS_SIZE  $$BUILD_SPIFFS_SIZE_HEX ($$BUILD_SPIFFS_SIZE)"
	$(info $$BUILD_SPIFFS_SIZE is [${BUILD_SPIFFS_SIZE}])

	$(MKSPIFFS) -c data -p $$BUILD_SPIFFS_PAGESIZE -b $$BUILD_SPIFFS_BLOCKSIZE -s $$BUILD_SPIFFS_SIZE $@

.PHONY: flash-fs
.ONESHELL:
flash-fs:
	BUILD_SPIFFS_START_HEX=$$(cat ${PARTITION_TABLE} | grep "^spiffs"|cut -d, -f4 | xargs)
	python ~/.arduino15/packages/esp32/tools/esptool_py/3.0.0/esptool.py --chip esp32 \
	  --port ${DEVICE} \
	  --baud 460800 \
	  --before default_reset \
	  --after hard_reset \
	  write_flash $${BUILD_SPIFFS_START_HEX} filesystem.bin


.PHONY: clean
clean:
	rm -rf build
	rm -f filesystem.bin

