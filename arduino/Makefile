UPLOAD_PORT=/dev/ttyACM0

all: build

build: arduino.ino types.hpp gui.hpp gui.cpp convert.h credentials.h
	mkdir -p build1
	arduino-cli compile --build-path `pwd`/build1/ --fqbn esp32:esp32:esp32c6 --board-options PartitionScheme=min_spiffs,FlashSize=4M .

upload: build
	arduino-cli compile --build-path `pwd`/build1/ --fqbn esp32:esp32:esp32c6 --board-options PartitionScheme=min_spiffs,FlashSize=4M -u -p $(UPLOAD_PORT)

uploadnet: build
	arduino-cli compile --build-path `pwd`/build1/ --fqbn esp32:esp32:esp32c6 --board-options PartitionScheme=min_spiffs,FlashSize=4M,ip=192.168.10.55 -u --port "192.168.10.55" --protocol network

getdeps:
	arduino-cli core install esp32:esp32
	arduino-cli lib install CircularBuffer U8g2 ArduinoOTA
