UPLOAD_PORT=/dev/ttyACM3
BUILDPATH=build1
BOARD=esp32:esp32:esp32c6
OPTIONS_PART=min_spiffs
FLASH_SIZE=4M

all: build

build: arduino.ino types.hpp gui.hpp gui.cpp convert.h credentials.h
	mkdir -p $(BUILDPATH)
	arduino-cli compile -v -j `nproc` --build-path `pwd`/$(BUILDPATH) --fqbn $(BOARD) --board-options PartitionScheme=$(OPTIONS_PART),FlashSize=$(FLASH_SIZE) .

upload: build
	arduino-cli compile -j `nproc` --build-path `pwd`/$(BUILDPATH) --fqbn $(BOARD) --board-options PartitionScheme=$(OPTIONS_PART),FlashSize=$(FLASH_SIZE) -u -p $(UPLOAD_PORT)

uploadnet: build
	arduino-cli compile -j `nproc` --build-path `pwd`/$(BUILDPATH) --fqbn $(BOARD) --board-options PartitionScheme=$(OPTIONS_PART),FlashSize=$(FLASH_SIZE),ip=192.168.10.55 -u --port "192.168.10.55" --protocol network

getdeps:
	arduino-cli core install esp32:esp32
	arduino-cli lib install CircularBuffer U8g2 ArduinoOTA
