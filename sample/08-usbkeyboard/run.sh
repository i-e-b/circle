#!/bin/sh
~/code/piemu/build/arm-softmmu/qemu-system-arm -M raspi2 -bios ~/code/circle/sample/08-usbkeyboard/kernel7.img -usbdevice keyboard
