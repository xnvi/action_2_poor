cmake_minimum_required(VERSION 3.10)

project (udp_video LANGUAGES C)
add_compile_options(-Wall -O2 -g)
set(CMAKE_BUILD_TYPE "Debug")

# 添加工程头文件
include_directories(./)
include_directories(./src/)
include_directories(../../utils/)

# 添加源文件
list(APPEND SOURCE_CODE
    ./main.c
    ./src/media_buf.c
    ./src/camera_config.c
    ./src/audio_aac_adp.c
    ./src/audio_dl_adp.c
    ./src/himpp_audio.c
    ./src/himpp_video.c
    ./src/himpp_isp.c
    ./src/himpp_sys.c
    ./src/himpp_venc.c
    ./src/himpp_vi.c
    ./src/himpp_vpss.c
    ./src/socket_udp.c
    ../../utils/himm.c
)

# 添加 hisi sdk 头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/include)

# hisi sdk 单独的动态库和静态库
list(APPEND HISI_SINGLE_LIB
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libqr.a
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhive_common.so
)

# hisi sdk 动态库，可与静态库替换
list(APPEND HISI_SO_LIB
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libaaccomm.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libaacdec.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libaacenc.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libaacsbrdec.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libaacsbrenc.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libbcd.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libdnvqe.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/lib_hiae.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/lib_hiawb.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/lib_hiawb_natura.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhi_cipher.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/lib_hidehaze.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/lib_hidrc.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/lib_hiir_auto.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/lib_hildci.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhive_AEC.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhive_AGC.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhive_ANR.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhive_EQ.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhive_HPF.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhive_record.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhive_RES.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libisp.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libive.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libivp.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libmd.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libmpi.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libsecurec.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libtde.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libupvqe.so
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libVoiceEngine.so
)

# hisi sdk 静态库，可与动态库替换
list(APPEND HISI_A_LIB
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libaaccomm.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libaacdec.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libaacenc.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libaacsbrdec.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libaacsbrenc.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libbcd.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libdnvqe.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/lib_hiae.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/lib_hiawb.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/lib_hiawb_natura.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhi_cipher.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/lib_hidehaze.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/lib_hidrc.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/lib_hiir_auto.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/lib_hildci.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhive_AEC.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhive_AGC.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhive_ANR.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhive_EQ.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhive_HPF.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhive_record.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libhive_RES.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libisp.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libive.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libivp.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libmd.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libmpi.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libsecurec.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libtde.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libupvqe.a
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../hisi_sdk/lib/libVoiceEngine.a
)

# 生成程序
add_executable(udp_video ${SOURCE_CODE})

# 链接hisi库
target_link_libraries(udp_video ${HISI_SINGLE_LIB} ${HISI_SO_LIB} ${HISI_A_LIB})

# 链接其他库
target_link_libraries(udp_video sns_ov5640 m pthread dl)
