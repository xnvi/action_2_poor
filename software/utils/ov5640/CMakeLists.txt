cmake_minimum_required(VERSION 3.10)

# libsns_ov5640.so
project (libsns_ov5640_shared LANGUAGES C)
add_compile_options(-Os -g -Wall -fPIC)
set(CMAKE_BUILD_TYPE "Debug")

add_compile_definitions(
    # SENSOR_OV5640
)

list(APPEND SNS_OV5640_SOURCE
    # ./ov5640_cmos_ex.h
    ./ov5640_cmos.c
    ./ov5640_sensor_ctl.c
)

# 添加 hisi sdk 头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/include)

# 添加头文件
include_directories(./)

# 生成动态库
add_library(sns_ov5640 SHARED ${SNS_OV5640_SOURCE})
