#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := chaufeeEau

#EXTRA_COMPONENT_DIRS = $(IDF_PATH)/examples/common_components/protocol_examples_common

include $(IDF_PATH)/make/project.mk

spiffs_web:ressources/web/*
	mkspiffs -c ressources/web/ -p 256 -b 8192 -s 0xFC000 web_spiffs.bin
	esptool --chip esp8266 --port /dev/ttyUSB0 --baud 115200 write_flash 0x304000 web_spiffs.bin

