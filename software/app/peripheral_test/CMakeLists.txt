cmake_minimum_required(VERSION 3.10)

project (peripheral_devices_test LANGUAGES C)
add_compile_options(-Wall -O2 -g)
set(CMAKE_BUILD_TYPE "Debug")

# 添加工程头文件
include_directories(./)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../utils)

# 添加 hisi sdk 头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/include)

# 添加源文件
list(APPEND SOURCE_CODE
    ./main.c
    ./ov5640.c
    ./mpu6050.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/himm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/hisi_adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/ft6206.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/st7789.c
)

# 生成可执行程序
add_executable(peripheral_test ${SOURCE_CODE})

# 链接其他库
target_link_libraries(peripheral_test m pthread)
