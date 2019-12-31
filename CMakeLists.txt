cmake_minimum_required(VERSION 3.8)
project("chino")

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.14/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

# include(${CMAKE_BINARY_DIR}/conan.cmake)
  
# conan_check()
# conan_add_remote(NAME bincrafts URL https://api.bintray.com/conan/bincrafters/public-conan)
# conan_cmake_run(CONANFILE conanfile.txt
#                 BASIC_SETUP CMAKE_TARGETS
#                 BUILD missing)
include(ExternalProject)

set(THIRD_PARTY ${CMAKE_CURRENT_LIST_DIR}/third_party)
set(CMAKE_CXX_STANDARD 17)

if (MSVC)
    add_definitions(/D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS /DNOMINMAX /DUNICODE)
else()
    add_compile_options(-Wno-multichar)
endif()

add_subdirectory(src/Native)
