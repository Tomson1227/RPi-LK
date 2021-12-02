#! /bin/bash

BOOT_DIR=/mnt/RPi_boot
ROOT_DIR=/mnt/RPi_root

SD1=`find /dev/ -name "sd?"`1
SD2=`find /dev/ -name "sd?"`2

sudo umount $SD1
sudo umount $SD2

sudo mkdir -p $BOOT_DIR $ROOT_DIR
sudo mount $SD1 $BOOT_DIR
sudo mount $SD2 $ROOT_DIR
sudo mkdir $BOOT_DIR/overlays

sudo umount $BOOT_DIR
sudo umount $ROOT_DIR
