make ARCH=arm htcleo_defconfig
make -j16 ARCH=arm CROSS_COMPILE=/linaro/android-toolchain-eabi/bin/arm-eabi- zImage 
# make -j16 ARCH=arm CROSS_COMPILE=/arm-2011.03/bin/arm-none-eabi- zImage 
