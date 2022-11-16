cmake_minimum_required(VERSION 3.10)

project (libmpeg_static LANGUAGES C)
add_compile_options(-Wall -Os -g)
set(CMAKE_BUILD_TYPE "Debug")

add_compile_definitions(
    # Define if you have the <immintrin.h> header file.
    # HAVE_IMMINTRIN_H
)

list(APPEND LIBMPEG_SOURCE
    source/mpeg-crc32.c
    # source/mpeg-element-descriptor.c
    # source/mpeg-pack-header.c
    # source/mpeg-packet.c
    source/mpeg-pat.c
    source/mpeg-pes.c
    source/mpeg-pmt.c
    # source/mpeg-ps-dec.c
    # source/mpeg-ps-enc.c
    # source/mpeg-psd.c
    # source/mpeg-psm.c
    source/mpeg-sdt.c
    # source/mpeg-system-header.c
    # source/mpeg-ts-dec.c
    source/mpeg-ts-enc.c
    # source/mpeg-ts-h264.c
    # source/mpeg-ts-h265.c
    # source/mpeg-ts-opus.h
    source/mpeg-util.c
    # source/mpeg-util.h
)

include_directories(./include)

# 生成静态库
add_library(mpeg STATIC ${LIBMPEG_SOURCE})
