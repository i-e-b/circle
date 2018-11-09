#!/bin/sh

# No external storage (for checking error handling)
#~/code/piemu/build/arm-softmmu/qemu-system-arm -M raspi2 -bios ./kernel7.img -usbdevice keyboard

# SD Card plugged in:
#~/code/piemu/build/arm-softmmu/qemu-system-arm -M raspi2 -bios ./kernel7.img -usbdevice keyboard -sd sd-image.iso
~/code/piemu/build/arm-softmmu/qemu-system-arm -M raspi2 -bios ./kernel7.img -usbdevice keyboard -drive if=sd,format=raw,file=sd-image.iso

# USB Storage plugged in
#~/code/piemu/build/arm-softmmu/qemu-system-arm -M raspi2 -bios ./kernel7.img -usbdevice keyboard -usbdevice disk:sd-image.iso