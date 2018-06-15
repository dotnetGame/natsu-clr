cmake_minimum_required(VERSION 3.11)

ADD_COMPILE_OPTIONS("/std:c++latest")

# Add subproject
ADD_SUBDIRECTORY(natsuclr)
ADD_SUBDIRECTORY(tests)
