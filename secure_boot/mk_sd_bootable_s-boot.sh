#
# Copyright (C) 2010 Samsung Electronics Co., Ltd.
#              http://www.samsung.com/
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
####################################
reader_type1="/dev/sdc"
reader_type2="/dev/mmcblk0"

if [ -z $1 ]
then
    echo "usage: ./mk_sd_bootable_s-boot.sh <SD Reader's device file>"
    exit 0
fi

if [ $1 = $reader_type1 ]
then 
    partition1="$11"
    partition2="$12"
    partition3="$13"
    partition4="$14"

elif [ $1 = $reader_type2 ]
then 
    partition1="$1p1"
    partition2="$1p2"
    partition3="$1p3"
    partition4="$1p4"

else
    echo "Unsupported SD reader"
    exit 0
fi

if [ -b $1 ]
then
    echo "$1 reader is identified."
else
    echo "$1 is NOT identified."
    exit 0
fi

####################################
# make partition
echo "make sd card partition"
echo "./sd_fdisk $1" 
./sd_fdisk $1 
dd iflag=dsync oflag=dsync if=sd_mbr.dat of=$1 
rm sd_mbr.dat
 
####################################
# format
umount $partition1 2> /dev/null
umount $partition2 2> /dev/null
umount $partition3 2> /dev/null
umount $partition4 2> /dev/null

echo "mkfs.vfat -F 32 $partition1"
mkfs.vfat -F 32 $partition1

#echo "mkfs.ext2 $partition2"
#mkfs.ext2 $partition2  

#echo "mkfs.ext2 $partition3"
#mkfs.ext2 $partition3  

#echo "mkfs.ext2 $partition4"
#mkfs.ext2 $partition4  

####################################
# mount 
#umount /media/sd 2> /dev/null
#mkdir -p /media/sd
#echo "mount -t vfat $partition1 /media/sd"
#mount -t vfat $partition1 /media/sd

signed_bl1_position=1
signed_bl2_position=33
uboot_position=129

####################################
#<BL1 fusing>
echo "---------------"
echo "BL1 fusing"
dd iflag=dsync oflag=dsync if=6450_EVT1.sbl1.bin of=$1 seek=$signed_bl1_position

####################################
#<BL2 fusing>
echo "---------------"
echo "BL2 signing"
split -b 30720 ../u-boot.bin temp.bin_
./add_checksum temp.bin_aa BL2.bin
rm temp.bin*
./codesigner_v20 -v2.0 BL2.bin BL2.bin.signed -STAGE2
./size_adjust BL2.bin.signed BL2-32K.bin.signed 32768 

echo "---------------"
echo "BL2 fusing"
dd iflag=dsync oflag=dsync if=BL2-32K.bin.signed of=$1 seek=$signed_bl2_position
rm BL2.bin
rm BL2.bin.signed

####################################
#<u-boot fusing>
echo "---------------"
echo "u-boot signing"
./size_adjust ../u-boot.bin u-boot-524032.bin 524032 
./codesigner_v20 -v2.0 u-boot-524032.bin u-boot-512K.bin.signed -STAGE2

echo "---------------"
echo "u-boot fusing"
dd iflag=dsync oflag=dsync if=./u-boot-512K.bin.signed of=$1 seek=$uboot_position
rm u-boot-524032.bin
rm BL2-32K.bin.signed
rm u-boot-512K.bin.signed

####################################
#<Message Display>
echo "---------------"
echo "U-boot image for secure boot is created successfully."
echo "Eject SD card and insert it again."

