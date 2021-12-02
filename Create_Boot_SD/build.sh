#! /bin/bash

BOOT_DIR=/mnt/RPi_boot
ROOT_DIR=/mnt/RPi_root
LINUX_DIR=~/repos/linux
BUSYBOX_DIR=~/repos/busybox
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

cd $BUSYBOX_DIR

read -p "Use default BusyBox setings [Y/n]: " $OPTION
if [[ $OPTION == "n" || $OPTION == "N" ]]; then
    sudo make menuconfig
    # Setting Location -> Setting Value
    # Settings -> Build static binary (no shared libraries)	Enable
    # Settings -> Cross compiler prefix	(arm-Linux-gnueabihf-)
    # Settings -> Destination path for ‘make install’	Same as INSTALL_MOD_PATH from kernel modules step
fi

sudo make -j$((`nproc` -1))
sudo make -j$((`nproc` -1)) install

sudo mkdir $ROOT_DIR/proc 
sudo mkdir $ROOT_DIR/sys 
sudo mkdir $ROOT_DIR/dev 
sudo mkdir $ROOT_DIR/etc
sudo mkdir $ROOT_DIR/etc/init.d
sudo touch $ROOT_DIR/etc/init.d/rcS
sudo chmod +x $ROOT_DIR/etc/init.d/rcS

printf "#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys

echo /sbin/mdev > /proc/sys/kernel/hotplug
mdev -s\n" | sudo tee $ROOT_DIR/etc/init.d/rcS > /dev/null

sudo umount $BOOT_DIR
sudo umount $ROOT_DIR
