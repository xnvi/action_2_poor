#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "file_opt.h"

file_info sg_file_list[MAX_FILE_NUM] = {0};
disk_info sg_disk_info = {0};

static char *sg_ext_names[3] = {
    "ts",
    "gcsv",
    PV_IMG_EXT_NAME,
};

static void _del_space(char *s)
{
    int i = 0;
    char *p1, *p2;
    
    p1 = s;
    p2 = s + strlen(s) - 1;
    while (p1 <= p2 && *p1 == ' '){
        p1++;
    }
    while (p2 >= p1 && *p2 == ' '){
        p2--;
    }
    *(++p2) = '\0';

    p2 = s;
    while(*p1 != '\0') {
        *p2++ = *p1++;
    }

    p1 = s;
    for (i = 0; s[i] != '\0'; i++)
    {
        if (s[i] != ' ')
        {
            *p1++ = s[i];
        }
        if ((s[i] == ' ') && !(s[i + 1] == ' ')) 
        {
            *p1++ = s[i];
        }
    }
    *p1 = '\0';

    return;
}

static int32_t tell_file_num(char *path)
{
    int32_t num = 0;
    // TODO
    // char *cmd = "ls -l | wc -l";
    return num;
}

// 换行符替换为\0
static void replace_crlf(char *s)
{
    while (*s != '\0') {
        if (*s == '\r' || *s == '\n') {
            *s = '\0';
            break;
        }
        s++;
    }
}

// 删除所有不可打印字符
static void del_ctrl_char(char *s)
{
    char *d = s;
    while (*s != '\0') {
        if (*s >= ' ' && *s <= '~') {
            *d = *s;
            d++;
        }
        else {
            printf("char %d\n", *s);
        }
        s++;
    }
    *d = '\0';
}

// 返回以空格分隔的第n个单词的下标，n从1计
static int32_t tell_column(char *s, int32_t n)
{
    int32_t column = 0;
    int32_t id = 0;

    for (column = 1; column <= n; column++) {
        // 跳过前导空格
        while (s[id] == ' ') {
            id++;
        }
        if (column == n) {
            break;
        }

        // 跳过连续字符
        while (s[id] != ' ') {
            id++;
        }
    }

    return id;
}

// 输入一个长度不小于256的字符串，返回系统信息
// 凑合用，自己小心点别越界
int32_t read_top_cmd(char *info)
{
    char line[256];
    FILE *fp = NULL;
    char *tmp;
    int32_t count = 0;

    fp = popen("top -b -n 1", "r");
    if (fp == NULL) {
        return 1;
    }

    while(1) {
        tmp = fgets(line, sizeof(line), fp);
        if (tmp == NULL) {
            break;
        }
        // del_ctrl_char(line);

        strcat(info, tmp);

        // 只要前两行，即CPU信息和内存信息
        count += 1;
        if (count >= 2) {
            break;
        }
    }

    fclose(fp);
    return 0;
}

// 只统计ts文件
int32_t read_ls_cmd()
{
    int32_t i = 0;
    int32_t n = 0;
    int32_t ls_file_num = 0;
    char line[256];
    FILE *fp = NULL;
    char *tmp;

    ls_file_num = 0;
    memset(sg_file_list, 0, sizeof(sg_file_list));

    // fp = fopen("ls.txt", "r");
    // if (fp == NULL) {
    //     return 1;
    // }
    fp = popen("ls -lh " FULL_REC_PATH, "r");
    // fp = popen("ls -lh ./", "r");
    if (fp == NULL) {
        return 0;
    }
    
    for (i = 0; i < MAX_FILE_NUM; i++)
    {
        tmp = fgets(line, sizeof(line), fp);
        if (tmp == NULL) {
            break;
        }
        replace_crlf(line);

        // 获取文件属性
        n = tell_column(line, 1);
        if (line[n] == '-') {
            sg_file_list[ls_file_num].type[0] = line[n];
        }
        // else if (line[n] == 'd') {
        //     // do nothing
        // }
        else {
            continue;
        }

        // 获取文件大小字符串
        n = tell_column(line, 5);
        sscanf(&line[n], "%10s", sg_file_list[ls_file_num].size);

        // 获取文件名字符串
        n = tell_column(line, 9);
        // n += tell_column(&line[n], 5);
        strncpy(sg_file_list[ls_file_num].name, &line[n], 46);

        // printf("%s %s %s\n", sg_file_list[ls_file_num].type, sg_file_list[ls_file_num].size, sg_file_list[ls_file_num].name);

#ifndef ENABLE_SIMULATE_SYSTEM_CALL
        // 只统计*.ts文件
        if (strstr(sg_file_list[ls_file_num].name, ".ts") != NULL) {
            ls_file_num += 1;
        }
        else {
            memset(&sg_file_list[ls_file_num], 0, sizeof(file_info));
        }
#else
        ls_file_num += 1;
#endif

    }

    fclose(fp);
    return ls_file_num;
}

int32_t read_df_cmd()
{
    char line[256];
    FILE *fp = NULL;
    char *tmp;
    int32_t found_disk = 0;

    // fp = fopen("df.txt", "r");
    // if (fp == NULL) {
    //     return 1;
    // }
    fp = popen("df -h", "r");
    if (fp == NULL) {
        return 1;
    }

    memset(&sg_disk_info, 0, sizeof(disk_info));
    strcpy(sg_disk_info.size, "sd-card");
    strcpy(sg_disk_info.used, "error");

    while(1) {
        tmp = fgets(line, sizeof(line), fp);
        if (tmp == NULL) {
            break;
        }
        replace_crlf(line);

        if (strncmp(tmp, DEV_DISK_NAME, sizeof(DEV_DISK_NAME)-1) == 0) {
            found_disk = 1;
            sscanf(tmp, "%*s %10s %10s %10s %3d",
                   sg_disk_info.size, sg_disk_info.used, sg_disk_info.free, &sg_disk_info.percent);
            break;
        }

    }
    // printf("disk size:%s used:%s free:%s percent:%d\n",
    //        sg_disk_info.size, sg_disk_info.used, sg_disk_info.free, sg_disk_info.percent);

    fclose(fp);
    return !found_disk;
}

// 删除同一个目录中文件名前缀相同的所有文件
// 例如有aaa.ts, aaa.log, aaa.bmp
// 输入以上任意一个文件名都会将这三个文件全部删除（文件名中只允许出现一个"."）
int32_t del_same_name_file(char *file_name)
{
    char cmd[128];
    char main_name[64];
    char *s, *d;

    memset(main_name, 0, sizeof(main_name));
    memset(cmd, 0, sizeof(cmd));

    s = file_name;
    d = main_name;
    while (*s != '.' && *s != '\0') {
        *d = *s;
        d++;
        s++;
    }

    strcat(cmd, "rm ");
    strcat(cmd, FULL_REC_PATH);
    strcat(cmd, main_name);
    strcat(cmd, ".*");
    system(cmd);

    return 0;
}

// 生成文件名，已有同名文件自动追加序号到name从，并校验路径是存在
int32_t make_file_name(char *path, int32_t plen, char *name, int32_t nlen)
{
    char fullname[128];
    char tmpname[64];
    int32_t index = 1;
    int32_t i = 0;
    int32_t pos = 0;
    int32_t exist = 0;

    if (plen + nlen > sizeof(fullname) || nlen > sizeof(tmpname)) {
        return 1;
    }

    if (access(path, F_OK | W_OK) != 0) {
        printf("path %s not exist\n", path);
        return 1;
    }

    memset(fullname, 0, sizeof(fullname));
    memset(tmpname, 0, sizeof(tmpname));
    strcat(fullname, path);
    strcat(fullname, name);

    exist = 0;
    pos = strlen(fullname);
    for (i = 0; i < 2; i++) {
        strcpy(&fullname[pos], sg_ext_names[i]);
        if (access(fullname, F_OK) == 0) {
            exist += 1;
        }
    }
    if (exist == 0) {
        // 没有重名
        return 0;
    }

    pos = strlen(path);
    while(1) {
        exist = 0;
        for (i = 0; i < 2; i++) {
            sprintf(tmpname, "%s_%d.%s", name, index, sg_ext_names[i]);
            strcpy(&fullname[pos], tmpname);
            if (access(fullname, F_OK) == 0) {
                exist += 1;
            }
        }
        if (exist == 0) {
            // 找到没有重名的文件名
            sprintf(tmpname, "%s_%d", name, index);
            strcpy(name, tmpname);
            return 0;
        }
        index += 1;

        if (index >= 100) {
            // 有点离谱了，有这么多重名文件？
            return 1;
        }
    }
}

int32_t save_pv_img(char *file_name, uint8_t *data, int32_t len)
{
    FILE *fp = NULL;
    char fullname[128];
    char main_name[64];
    char *s, *d;

    memset(main_name, 0, sizeof(main_name));
    memset(fullname, 0, sizeof(fullname));

    s = file_name;
    d = main_name;
    while (*s != '.' && *s != '\0') {
        *d = *s;
        d++;
        s++;
    }

    strcat(fullname, FULL_REC_PATH);
    strcat(fullname, main_name);
    strcat(fullname, "." PV_IMG_EXT_NAME);

    fp = fopen(fullname, "wb");
    if (fp == NULL) {
        return 1;
    }
    fwrite(data, 1, len, fp);
    fclose(fp);

    return 0;
}

int32_t load_pv_img(char *file_name, uint8_t *data, int32_t len)
{
    FILE *fp = NULL;
    char fullname[128];
    char main_name[64];
    char *s, *d;

    memset(main_name, 0, sizeof(main_name));
    memset(fullname, 0, sizeof(fullname));

    s = file_name;
    d = main_name;
    while (*s != '.' && *s != '\0') {
        *d = *s;
        d++;
        s++;
    }

    strcat(fullname, FULL_REC_PATH);
    strcat(fullname, main_name);
    strcat(fullname, "." PV_IMG_EXT_NAME);

    fp = fopen(fullname, "rb");
    if (fp == NULL) {
        return 1;
    }
    fread(data, 1, len, fp);
    fclose(fp);

    return 0;
}

int32_t search_pv_img(char *file_name)
{
    char fullname[128];
    char main_name[64];
    char *s, *d;

    memset(main_name, 0, sizeof(main_name));
    memset(fullname, 0, sizeof(fullname));

    s = file_name;
    d = main_name;
    while (*s != '.' && *s != '\0') {
        *d = *s;
        d++;
        s++;
    }

    strcat(fullname, FULL_REC_PATH);
    strcat(fullname, main_name);
    strcat(fullname, "." PV_IMG_EXT_NAME);

    if (access(fullname, F_OK) == 0) {
        return 1; // 文件存在
    }
    else {
        return 0; // 文件不存在
    }
}
