#ifndef __FILE_OPT_H
#define __FILE_OPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "camera_base_config.h"

typedef struct {
    char type[4];
    char size[12];
    char name[48];
} file_info;

typedef struct {
    char size[12];
    char used[12];
    char free[12];
    int32_t percent;
} disk_info;

// 这个文件数基本够用了，更多文件在电脑上看吧
#define MAX_FILE_NUM 64
file_info sg_file_list[MAX_FILE_NUM];


#define MOUNT_NODE "/tmp/sd/"
#define DEV_DISK_NAME "/dev/mmcblk0p1"

#ifdef ENABLE_SIMULATE_SYSTEM_CALL
#define FULL_REC_PATH "./"
#else
#define FULL_REC_PATH "/tmp/sd/video/"
#endif

disk_info sg_disk_info;

// 缩略图文件扩展名
#define PV_IMG_EXT_NAME "565"


int32_t read_top_cmd(char *info);
int32_t read_ls_cmd();
int32_t read_df_cmd();
int32_t del_same_name_file(char *file_name);
int32_t make_file_name(char *path, int32_t plen, char *name, int32_t nlen);

int32_t save_pv_img(char *file_name, uint8_t *data, int32_t len);
int32_t load_pv_img(char *file_name, uint8_t *data, int32_t len);
int32_t search_pv_img(char *file_name);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
