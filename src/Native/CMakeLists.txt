cmake_minimum_required (VERSION 3.8)

include_directories(${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/Generated)

set(SRCS natsu.fcall.cpp
         natsu.runtime.cpp
         natsu.gc.cpp
         main.cpp
         Generated/System.Private.CorLib.cpp
         Generated/Chino.Kernel.cpp
         Generated/BitFields.cpp)
         
if (NOT WIN32)
    set(ASM_SRCS crt.S)
    SET_PROPERTY(SOURCE ${ASM_SRCS} PROPERTY LANGUAGE C)
endif()

add_executable(chino ${SRCS} ${ASM_SRCS})

if (NOT WIN32)
    target_link_libraries(chino -Wl,-gc-sections -nostartfiles -Wl,-static -T ${CMAKE_CURRENT_LIST_DIR}/chino.ld)
endif()

ADD_CUSTOM_COMMAND(OUTPUT chino.bin
		COMMAND rm -f chino.bin
		COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_CURRENT_BINARY_DIR}/chino chino.bin
        DEPENDS chino
        COMMENT "Generating chino.bin ...")

ADD_CUSTOM_TARGET(firmware DEPENDS chino.bin)