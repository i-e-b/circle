cmake_minimum_required (VERSION 2.6)
project (UsbKeyboard)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES kernel.cpp kernel.h main.cpp)
include_directories( ${UsbKeyboard_SOURCE_DIR}/../../include/circle )

add_custom_target(UsbKeyboard COMMAND make -C ${UsbKeyboard_SOURCE_DIR}
        CLION_EXE_DIR=${PROJECT_BINARY_DIR})