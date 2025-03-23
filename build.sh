#!/bin/bash

# Path to the GCC cross compiler
#
# This one uses the tools from STM32CubeIDE, but you can of course
# also download the toolchain from the ARM website
# https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
TOOLCHAIN_PATH=/opt/st/stm32cubeide_1.18.0/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.13.3.rel1.linux64_1.0.0.202410170706/tools/bin

export PATH=${TOOLCHAIN_PATH}:${PATH}

cmake -DCMAKE_TOOLCHAIN_FILE=cubeide-gcc.cmake  -S ./ -B Release -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
make -C Release VERBOSE=1 -j

# cmake -DCMAKE_TOOLCHAIN_FILE=cubeide-gcc.cmake  -S ./ -B Debug -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
# make -C Debug VERBOSE=1 -j
