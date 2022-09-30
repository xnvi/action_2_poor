cmake_minimum_required(VERSION 3.10)

# 设置交叉编译器
# set(CMAKE_C_COMPILER   /opt/hisi-linux/x86-arm/arm-himix200-linux/bin/arm-himix200-linux-gcc)
# set(CMAKE_CXX_COMPILER /opt/hisi-linux/x86-arm/arm-himix200-linux/bin/arm-himix200-linux-g++)
set(CMAKE_C_COMPILER   /opt/hisi-linux/x86-arm/arm-himix100-linux/bin/arm-himix100-linux-gcc)
set(CMAKE_CXX_COMPILER /opt/hisi-linux/x86-arm/arm-himix100-linux/bin/arm-himix100-linux-g++)

project (hisi_project LANGUAGES C)

add_compile_options(-Wall -O2 -g)
set(CMAKE_BUILD_TYPE "Debug")

# 开启 NEON 指令加速
set(CMAKE_C_FLAGS "-mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4")

# 添加编译器宏
add_compile_definitions(
    # 摄像头的型号为 imx335
    SENSOR0_TYPE=OV5640_YUV_MIPI_2M_30FPS_8BIT
)

# 编译其他公用库文件
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext_lib/libmpeg)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/utils/ov5640)

# hello_world 测试程序
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/app/hello_world)

# 外设测试程序
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/app/peripheral_test)

# LVGL 测试程序
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/app/lvgl_test)

# UDP 音视频流测试程序
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/app/udp_video)

# 相机程序
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/app/camera)

# 温度监控程序
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/app/temp_monitor)
