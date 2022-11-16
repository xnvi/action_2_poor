cmake_minimum_required(VERSION 3.10)

# 设置交叉编译器
# set(CMAKE_C_COMPILER   /opt/hisi-linux/x86-arm/arm-himix200-linux/bin/arm-himix200-linux-gcc)
# set(CMAKE_CXX_COMPILER /opt/hisi-linux/x86-arm/arm-himix200-linux/bin/arm-himix200-linux-g++)
set(CMAKE_C_COMPILER   /opt/hisi-linux/x86-arm/arm-himix100-linux/bin/arm-himix100-linux-gcc)
set(CMAKE_CXX_COMPILER /opt/hisi-linux/x86-arm/arm-himix100-linux/bin/arm-himix100-linux-g++)

project (hisi_project LANGUAGES C)

add_compile_options(-Wall -O2 -g)
set(CMAKE_BUILD_TYPE "Debug")

#版本号
set(SOFTWARE_VERSION_STR "v0.1")

# 生成git信息，以便版本控制
set(GIT_COMMIT_HASH_STR "")
set(GIT_BRANCH_NAME_STR "")
find_package(Git REQUIRED)                                     # 查找Git，QUIET静默方式不报错，REQUIRED未找到则终止
if(GIT_FOUND)
    # 获取HASH
    execute_process(                                           # 执行一个子进程
        COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%h    # 命令
        OUTPUT_VARIABLE GIT_COMMIT_HASH_STR                    # 输出字符串存入变量
        OUTPUT_STRIP_TRAILING_WHITESPACE                       # 删除字符串尾的换行符
        # ERROR_QUIET                                            # 对执行错误静默
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}          # 执行路径
    )
    # 获取分支名
    execute_process(                                           # 执行一个子进程
        COMMAND ${GIT_EXECUTABLE} symbolic-ref --short -q HEAD # 命令
        OUTPUT_VARIABLE GIT_BRANCH_NAME_STR                    # 输出字符串存入变量
        OUTPUT_STRIP_TRAILING_WHITESPACE                       # 删除字符串尾的换行符
        # ERROR_QUIET                                            # 对执行错误静默
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}          # 执行路径
    )
endif()
if(NOT (GIT_BRANCH_NAME_STR AND GIT_COMMIT_HASH_STR))
    message(FATAL_ERROR "git info not found")
else()
    message(STATUS "git version is ${GIT_BRANCH_NAME}:${GIT_COMMIT_HASH}")
endif()

# 开启 NEON 指令加速
set(CMAKE_C_FLAGS "-mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4")

# 添加编译器宏
add_compile_definitions(
    # 摄像头的全局宏 OV5640_YUV_MIPI_2M_30FPS_8BIT
    SENSOR0_TYPE=OV5640_YUV_MIPI_2M_30FPS_8BIT
    SOFTWARE_VERSION="${SOFTWARE_VERSION_STR}"
    GIT_BRANCH_NAME="${GIT_BRANCH_NAME_STR}"
    GIT_COMMIT_HASH="${GIT_COMMIT_HASH_STR}"
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
