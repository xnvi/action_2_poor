// 环形缓存队列，适用于1个生产者，多个消费者消费同一个数据的情况
// 未完成，以后有需要再开发

#ifndef _MEDIA_BUF_H
#define _MEDIA_BUF_H

#include <stdint.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_BLOCK_NUM 4

typedef enum {
    MBUF_OK,
    MBUF_PARAM_NULL_PTR,
    MBUF_PARAM_OVERFLOW,
    MBUF_PARAM_INVALID,
    MBUF_IS_FULL,
    MBUF_IS_EMPTY,
    MBUF_OTHER_ERR,
} mbuf_err_code;

typedef struct
{
    uint32_t head;
    uint32_t tail;
    uint32_t block_num; // 块总数
    uint32_t block_len; // 每块的大小
    uint32_t used_num; // 已使用块数
    uint32_t total_size; // block_num * block_len
    uint32_t data_size[MAX_BLOCK_NUM]; // 每块中数据的大小
    uint8_t *buf;
    // int32_t (*block_print)(void *);
    pthread_mutex_t lock;
} mbuf_handle;

mbuf_err_code mbuf_init_sta(mbuf_handle *p, uint8_t *buf, uint32_t block_len, uint32_t block_num);
mbuf_err_code mbuf_destroy_sta(mbuf_handle *p);

mbuf_handle *mbuf_init_dyn(uint32_t block_len, uint32_t block_num);
mbuf_err_code mbuf_destroy_dyn(mbuf_handle *p);

mbuf_err_code mbuf_reset(mbuf_handle *p);

// 有内存复制读写
mbuf_err_code mbuf_write_cp(mbuf_handle *p, const void *data, uint32_t len);
mbuf_err_code mbuf_read_cp(mbuf_handle *p, void *data, uint32_t *len);

// 无内存复制写，返回一个空的内存地址，由用户自行写入
void *mbuf_write_no_cp(mbuf_handle *p, uint32_t len);
// 写入完成后调用，与 mbuf_write_no_cp() 配合使用，len为写入的数据大小
mbuf_err_code mbuf_write_end_no_cp(mbuf_handle *p);

// 无内存复制读，返回一个内存地址和数据长度，失败返回NULL
void *mbuf_read_no_cp(mbuf_handle *p, uint32_t *len);
// 无内存复制读，且读后释放，与 mbuf_read_no_cp() 配合使用
mbuf_err_code mbuf_read_end_no_cp(mbuf_handle *p);

// 无内存复制读，返回一个内存地址和数据长度，失败返回NULL
void *mbuf_read_no_free(mbuf_handle *p, uint32_t *len);
// 无内存复制读，读后不释放，与 mbuf_read_no_free() 配合使用
mbuf_err_code mbuf_read_end_no_free(mbuf_handle *p);
// 不读取，直接释放一个缓存
mbuf_err_code mbuf_free(mbuf_handle *p);

int32_t mbuf_get_used_num(const mbuf_handle *p);
int32_t mbuf_get_free_num(const mbuf_handle *p);
int32_t mbuf_is_empty(const mbuf_handle *p);
int32_t mbuf_is_full(const mbuf_handle *p);

// void mbuf_print(const mbuf_handle *p);
// void mbuf_print(mbuf_handle *p, void(*block_print)(void *, uint32_t));

#ifdef __cplusplus
}
#endif

#endif
