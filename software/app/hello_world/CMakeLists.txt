cmake_minimum_required(VERSION 3.10)

project (udp_ts LANGUAGES C)
add_compile_options(-Wall -O2 -g)
set(CMAKE_BUILD_TYPE "Debug")

# 添加工程头文件
include_directories(./)

# 添加源文件
list(APPEND SOURCE_CODE
    ./main.c
)

# 生成可执行程序
add_executable(hello_world ${SOURCE_CODE})

# 链接其他库
target_link_libraries(hello_world m pthread)
