#
# Copyright (C) 2010 Samsung Electronics Co., Ltd.
#              http://www.samsung.com/
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
####################################
reader_type1="/dev/sdb"
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

bl1_position=1
uboot_position=65

####################################
#<BL1 fusing>
echo "---------------"
echo "BL1 fusing"
#split -b 30720 ../u-boot.bin temp.bin_
#./add_checksum temp.bin_aa BL2.bin
#rm temp.bin*
./size_adjust ../u-boot.bin BL2-16K.bin 16384 
dd iflag=dsync oflag=dsync if=BL2-16K.bin of=$1 seek=$bl1_position
rm BL2-16K.bin

####################################
#<u-boot fusing>
echo "---------------"
echo "u-boot fusing"
dd iflag=dsync oflag=dsync if=../u-boot.bin of=$1 seek=$uboot_position

####################################
#<Message Display>
echo "---------------"
echo "U-boot image for secure boot is created successfully."
echo "Eject SD card and insert it again."

