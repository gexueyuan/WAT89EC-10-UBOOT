#!/bin/sh


make clean
make smdk6450_config
make

cd secure_boot
sudo ./gxysd.sh /dev/sdc
cd ..
