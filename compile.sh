#!/bin/sh
cd zloader.ls2k/
sed -i "2c TARGETEL=ls2k_lspi" ./Makefile.ls2k
make cfg all tgt=rom CROSS_COMPILE=/opt/loongarch64-linux-gnu-2021-12-10-vector/bin/loongarch64-linux-gnu- DEBUG=-g
make dtb CROSS_COMPILE=/opt/loongarch64-linux-gnu-2021-12-10-vector/bin/loongarch64-linux-gnu-
sed -i "2c TARGETEL=ls2k" ./Makefile.ls2k
