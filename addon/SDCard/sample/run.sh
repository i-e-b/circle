#!/bin/sh
#~/code/piemu/build/arm-softmmu/qemu-system-arm -M raspi2 -bios ./kernel7.img -usbdevice keyboard -drive if=sd,format=raw,file=sd-image.iso
#~/code/piemu/build/arm-softmmu/qemu-system-arm -M raspi2 -bios ./kernel7.img -usbdevice keyboard -sd sd-image.iso

~/code/piemu/build/arm-softmmu/qemu-system-arm -M raspi2 -bios ./kernel7.img -usbdevice keyboard -usbdevice disk:sd-image.iso