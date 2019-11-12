set(ASM_SRCS crt.S)
set(SRCS crt.cpp)

set_property(SOURCE ${ASM_SRCS} PROPERTY LANGUAGE C)
add_library(arch STATIC ${SRCS} ${ASM_SRCS})