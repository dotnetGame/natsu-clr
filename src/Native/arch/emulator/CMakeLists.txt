set(SRCS crt.cpp
         environment.cpp
         threading.cpp
         schedule.cpp
         hal/io/console.cpp
         ${GENERATED_DIR}/Chino.Chip.Emulator.cpp)

add_library(arch STATIC ${SRCS} ${ASM_SRCS})