set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

set(tools /opt/chino-toolchain)
set(CMAKE_C_COMPILER ${tools}/bin/riscv64-unknown-elf-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/riscv64-unknown-elf-g++)
set(CMAKE_OBJCOPY ${tools}/bin/riscv64-unknown-elf-objcopy)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
SET(CMAKE_C_COMPILER_WORKS 1)
SET(CMAKE_CXX_COMPILER_WORKS 1)
SET(CHINO_ARCH riscv64)
SET(CHINO_BOARD kd233)

add_compile_options(
    -Wno-multichar
    -fno-common
    -std=c++17
    -ffast-math
    -Os
    -ffunction-sections
    -fdata-sections
    -mcmodel=medany
    -mrelax)