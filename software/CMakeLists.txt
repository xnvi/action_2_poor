cmake_minimum_required(VERSION 3.10)

# 设置交叉编译器
# set(CMAKE_C_COMPILER   /opt/hisi-linux/x86-arm/arm-himix200-linux/bin/arm-himix200-linux-gcc)
# set(CMAKE_CXX_COMPILER /opt/hisi-linux/x86-arm/arm-himix200-linux/bin/arm-himix200-linux-g++)
set(CMAKE_C_COMPILER   /opt/hisi-linux/x86-arm/arm-himix100-linux/bin/arm-himix100-linux-gcc)
set(CMAKE_CXX_COMPILER /opt/hisi-linux/x86-arm/arm-himix100-linux/bin/arm-himix100-linux-g++)

project (hisi_project LANGUAGES C)

add_compile_options(-Wall -O2 -g)
set(CMAKE_BUILD_TYPE "Debug")

# 添加编译器宏
add_compile_definitions(
)

# hello_world 测试程序
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/app/hello_world)
