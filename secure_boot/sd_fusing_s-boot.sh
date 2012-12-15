#
# Copyright (C) 2010 Samsung Electronics Co., Ltd.
#              http://www.samsung.com/
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
####################################
#if [ -z $1 ]
#then
#    echo "usage: ./sd_fusing_s-boot.sh"
#    exit 0
#fi


####################################
#<BL2 signing>
echo "---------------"
echo "BL2 signing"
split -b 30720 ../u-boot.bin temp.bin_
./add_checksum temp.bin_aa BL2.bin
rm temp.bin*
./codesigner_v20 -v2.0 BL2.bin BL2.bin.signed -STAGE2
./size_adjust BL2.bin.signed BL2-32K.bin.signed 32768 
rm BL2.bin
rm BL2.bin.signed

####################################
#<u-boot signing>
echo "---------------"
echo "u-boot signing"
./size_adjust ../u-boot.bin u-boot-524032.bin 524032 
./codesigner_v20 -v2.0 u-boot-524032.bin u-boot-512K.bin.signed -STAGE2  #mkh: this adds 256 bytes signed data.
rm u-boot-524032.bin

####################################
#< signedBL1 image copy & u-boot image concatenation>
rm ./sdfuse/*
cp 6450_EVT1.sbl1.bin ./sdfuse/BL1.bin.signed
cat BL2-32K.bin.signed u-boot-512K.bin.signed > ./sdfuse/u-boot.bin.sd.signed
rm BL2-32K.bin.signed
#rm u-boot-512K.bin.signed

####################################
#<zImage image signing>
echo "---------------"
echo "zImage signing"
./size_adjust zImage zImage-3MB 3145472 
./codesigner_v20 -v2.0 zImage-3MB zImage.signed -STAGE2
rm zImage-3MB
cp zImage.signed ./sdfuse
rm zImage.signed

####################################
#<ramdisk-uboot image signing>
echo "---------------"
echo "ramdisk-uboot signing"
./size_adjust ramdisk-uboot.img ramdisk-uboot-1.5MB.img 1572608
./codesigner_v20 -v2.0 ramdisk-uboot-1.5MB.img  ramdisk-uboot.img.signed -STAGE2
rm ramdisk-uboot-1.5MB.img 
cp ramdisk-uboot.img.signed ./sdfuse
rm ramdisk-uboot.img.signed 
 
####################################
#<system image copy>
echo "---------------"
echo "system image copy"
cp system.img ./sdfuse

####################################
#<Message Display>
echo "---------------"
echo "U-boot image for secure boot is created successfully."
echo "Eject SD card and insert it again."

