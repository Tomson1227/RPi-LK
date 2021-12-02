#! /bin/bash

BOOT_DIR=/mnt/RPi_boot
ROOT_DIR=/mnt/RPi_root
LINUX_DIR=~/repos/linux
BIN_FILES_DIR=~/repos/RPi_LK/Create_Boot_SD

SD1=`find /dev/ -name "sd?"`1
SD2=`find /dev/ -name "sd?"`2

sudo umount $SD1
sudo umount $SD2

sudo mkdir -p $BOOT_DIR $ROOT_DIR
sudo mount $SD1 $BOOT_DIR
sudo mount $SD2 $ROOT_DIR
sudo mkdir $BOOT_DIR/overlays

export ARCH=arm
export KERNEL=kernel7
export CROSS_COMPILE=arm-linux-gnueabihf-
export INSTALL_MOD_PATH=$ROOT_DIR

cd $LINUX_DIR

sudo make -j$((`nproc` -1)) bcm2709_defconfig
sudo make -j$((`nproc` -1)) zImage modules dtbs

sudo mkdir $BOOT_DIR/overlays
sudo cp $BIN_FILES_DIR/bootcode.bin $BOOT_DIR
sudo cp $BIN_FILES_DIR/fixud.sh     $BOOT_DIR
sudo cp $BIN_FILES_DIR/start.elf    $BOOT_DIR
sudo cp arch/arm/boot/zImage        $BOOT_DIR
sudo cp arch/arm/boot/dts/bcm2710-rpi-3-b.dtb $BOOT_DIR
sudo cp arch/arm/boot/dts/overlays/disable-bt.dtbo $BOOT_DIR/overlays

sudo touch $BOOT_DIR/config.txt
printf "# Use the Linux Kernel we compiled earlier.
kernel=zImage

# Enable UART so we can use a TTL cable.
enable_uart=1

# Use the appropriate DTB for our device.
device_tree=bcm2710-rpi-3-b.dtb

# Disable Bluetooth via device tree overlay.
# It's a complicated explanation that you
# can read about here: https://youtu.be/68jbiuf27AY?t=431
# IF YOU SKIP THIS STEP, your serial connection will not
# work correctly.
dtoverlay=disable-bt\n" | sudo tee $BOOT_DIR/config.txt > /dev/null 

sudo touch $BOOT_DIR/cmdline.txt
printf "console=tty1 console=serial0,115200 root=/dev/mmcblk0p2 rootfstype=ext4 rootwait\n" | sudo tee $BOOT_DIR/cmdline.txt > /dev/null 

sudo make -j$((`nproc` -1)) modules_install

sudo umount $BOOT_DIR
sudo umount $ROOT_DIR
