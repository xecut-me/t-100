# FIXME - this is preliminary version. Needs proper configurations set up

all: build

build: arduino.ino types.hpp gui.hpp gui.cpp convert.h credentials.h
	mkdir -p build1
	arduino-cli compile --build-path `pwd`/build1/ --fqbn esp32:esp32:esp32c6 --board-options PartitionScheme=no_ota,FlashSize=4M .

upload: build
	arduino-cli compile --build-path `pwd`/build1/ --fqbn esp32:esp32:esp32c6 --board-options PartitionScheme=no_ota,FlashSize=4M -u -p /dev/ttyACM0 .

getdeps:
	arduino-cli core install esp32:esp32
	arduino-cli lib install CircularBuffer U8g2
