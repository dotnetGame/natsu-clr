cmake_minimum_required (VERSION 3.8)

set(GENERATED_DIR ${CMAKE_CURRENT_LIST_DIR}/Generated)
include_directories(${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/Generated)
add_subdirectory(arch/${CHINO_ARCH})

if (CHINO_APP)
    string(REPLACE "." "_" CHINO_APP_MODULE ${CHINO_APP})
    string(REPLACE "." "::" CHINO_APP_NAMESPACE ${CHINO_APP})
else()
    message(FATAL_ERROR "Please set CHINO_APP")
endif()

set(SRCS natsu.fcall.cpp
         natsu.runtime.cpp
         natsu.gc.cpp
         natsu.unicode.cpp
         natsu.threading.cpp
         chino.runtime.cpp
         main.cpp
         Generated/${CHINO_APP}.cpp
         Generated/System.Private.CoreLib.cpp
         Generated/System.Collections.cpp
         Generated/System.Diagnostics.Debug.cpp
         Generated/System.Runtime.cpp
         Generated/System.Runtime.Extensions.cpp
         Generated/Chino.Core.cpp
         Generated/Chino.IO.cpp
         Generated/Chino.Threading.cpp
         Generated/Chino.Kernel.cpp
         Generated/BitFields.cpp)

add_executable(chino ${SRCS} ${ASM_SRCS})
target_compile_definitions(chino PUBLIC
    -DCHINO_ARCH=${CHINO_ARCH}
    -DCHINO_APP=${CHINO_APP}
    -DCHINO_APP_MODULE=${CHINO_APP_MODULE}
    -DCHINO_APP_NAMESPACE=${CHINO_APP_NAMESPACE})
target_link_libraries(chino PUBLIC arch)

if (NOT WIN32)
    target_link_libraries(chino PRIVATE -Wl,-gc-sections -Wl,-static -T ${CMAKE_CURRENT_LIST_DIR}/board/${CHINO_ARCH}/${CHINO_BOARD}/chino.ld)
    target_link_libraries(chino PRIVATE stdc++ arch)
    target_compile_options(chino PRIVATE -flto)
endif()

ADD_CUSTOM_COMMAND(OUTPUT chino.bin
		COMMAND rm -f chino.bin
		COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_CURRENT_BINARY_DIR}/chino chino.bin
        DEPENDS chino
        COMMENT "Generating chino.bin ...")

ADD_CUSTOM_TARGET(firmware DEPENDS chino.bin)