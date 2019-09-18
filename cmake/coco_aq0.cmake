set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR armv7-m)

set(tools /opt/chino-toolchain)
set(CMAKE_C_COMPILER ${tools}/bin/arm-unknown-eabi-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-unknown-eabi-g++)
set(CMAKE_OBJCOPY ${tools}/bin/arm-unknown-eabi-objcopy)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
SET(CMAKE_C_COMPILER_WORKS 1)
SET(CMAKE_CXX_COMPILER_WORKS 1)
SET(CHINO_ARCH armv7-m)
SET(CHINO_BOARD coco_aq0)

add_compile_options(
    -Wno-multichar
    -fno-common
    -std=c++17
    -ffast-math
    -Os
    -flto
    -ffunction-sections
    -fdata-sections
    -mcpu=cortex-m3)
    