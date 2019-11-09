#!/bin/bash

clear

platformio run --target clean
platformio run
platformio run --target buildfs

path=$(ls -d .pio/*/ | cut -d'/' -f 2)

echo "path: $path"
echo -n "get Version: "

cat src/config.h | grep __FIRMWARE__ | tr -s " " | cut -d' ' -f4 | cut -d'"' -f2 > powermeter.version.txt ; cat powermeter.version.txt
cp .pio/$path/esp32dev/firmware.bin powermeter.ino.bin
cp .pio/$path/esp32dev/spiffs.bin powermeter.spiffs.bin

echo "File to send:"
echo "-------------------------------"
ls | grep powermeter
echo "-------------------------------"

echo "copy firmware to Server"
sshpass -p RK.5-xQ23fram scp powermeter* root@192.168.22.1:/var/www/
echo "done."
