cmake_minimum_required(VERSION 3.10)

project (lvgl_test LANGUAGES C)
add_compile_options(-Wall -Os -g)
set(CMAKE_BUILD_TYPE "Debug")

# 添加工程头文件
include_directories(./)
include_directories(./src/)
include_directories(../../utils/)
include_directories(../../utils/st7789_ko/)
include_directories(../../ext_lib/lvgl/)

# 添加源文件
list(APPEND SOURCE_CODE
    ./main.c
	../../utils/himm.c
	../../utils/ft6206.c
    ../../utils/i2c.c
)

# 添加 LVGL 源文件
file(GLOB_RECURSE LVGL_SOURCE ../../ext_lib/lvgl/src/*.c)

# 添加 hisi sdk 头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/include)

# 生成程序
add_executable(lvgl_test ${SOURCE_CODE} ${LVGL_SOURCE})

# 链接其他库
target_link_libraries(lvgl_test m pthread)
