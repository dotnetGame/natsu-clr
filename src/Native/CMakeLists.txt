cmake_minimum_required (VERSION 3.8)

include_directories(${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/Generated)

set(SRCS natsu.fcall.cpp
         natsu.runtime.cpp
         main.cpp
         Generated/System.Private.CorLib.cpp
         Generated/Chino.Kernel.cpp)

add_executable(chino ${SRCS})
target_link_libraries(chino)