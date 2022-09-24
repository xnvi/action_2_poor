#include "media_buf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

mbuf_err_code mbuf_init_sta(mbuf_handle *p, uint8_t *buf, uint32_t block_len, uint32_t block_num)
{
    if (!(p && buf && block_len && block_num)) {
        printf("param error, *p %p, *buf %p, block_len %d, block_num %d\n",
                p, buf, block_len, block_num);
        return MBUF_PARAM_INVALID;
    }
    if (block_num > MAX_BLOCK_NUM) {
        printf("too many block(%d), max is %d\n", block_num, MAX_BLOCK_NUM);
        return MBUF_PARAM_OVERFLOW;
    }
    if (pthread_mutex_init(&p->lock, NULL)) {
        printf("pthread_mutex_init error\n");
        return MBUF_OTHER_ERR;
    }
    p->block_len = block_len;
    p->block_num = block_num;
    p->total_size = block_len * block_num;
    p->used_num = 0;
    p->head = 0;
    p->tail = 0;
    p->buf = buf;
    memset(p->data_size, 0, sizeof(uint32_t) * MAX_BLOCK_NUM);

    return MBUF_OK;
}

mbuf_err_code mbuf_destroy_sta(mbuf_handle *p)
{
    if (p) {
        if (pthread_mutex_destroy(&p->lock)) {
            printf("pthread_mutex_destroy error\n");
            return MBUF_OTHER_ERR;
        }
        memset(p->buf, 0, p->total_size);
        p->head = 0;
        p->tail = 0;
        p->block_len = 0;
        p->block_num = 0;
        p->used_num = 0;
        p->total_size = 0;
        return MBUF_OK;
    }

    return MBUF_PARAM_NULL_PTR;
}

mbuf_handle *mbuf_init_dyn(uint32_t block_len, uint32_t block_num)
{
    mbuf_handle *p = NULL;
    uint8_t *buf = NULL;

    if (block_num > MAX_BLOCK_NUM) {
        printf("too many block(%d), max is %d\n", block_num, MAX_BLOCK_NUM);
        return NULL;
    }

    p = (mbuf_handle *)malloc(sizeof(mbuf_handle));
    if (p == NULL) {
        printf("malloc mbuf_handle error, size %d\n", (int32_t)sizeof(mbuf_handle));
        return NULL;
    }

    buf = (uint8_t *)malloc(block_len * block_num);
    if (buf == NULL) {
        printf("malloc ringbuffer error, size %d\n", block_len * block_num);
        free(p);
        return NULL;
    }

    if (mbuf_init_sta(p, buf, block_len, block_num)) {
        printf("ringbuffer init error\n");
        free(buf);
        free(p);
        return NULL;
    }

    return p;
}

mbuf_err_code mbuf_destroy_dyn(mbuf_handle *p)
{
    if (p) {
        mbuf_err_code ret = MBUF_OK;
        ret = mbuf_destroy_sta(p);
        if (ret != MBUF_OK) {
            printf("ringbuffer destroy error\n");
            return ret;
        }
        free(p->buf);
        free(p);
        return MBUF_OK;
    }

    return MBUF_PARAM_NULL_PTR;
}

mbuf_err_code mbuf_reset(mbuf_handle *p)
{
    if (p) {
        pthread_mutex_lock(&p->lock);
        memset(p->buf, 0, p->total_size);
        p->head = 0;
        p->tail = 0;
        p->used_num = 0;
        pthread_mutex_unlock(&p->lock);
        return MBUF_OK;
    }

    return MBUF_PARAM_NULL_PTR;
}

mbuf_err_code mbuf_write_cp(mbuf_handle *p, const void *data, uint32_t len)
{
    if (!(p && data && len)) {
        printf("param error, *p %p, *data %p, len %d\n", p, data, len);
        return MBUF_PARAM_INVALID;
    }

    if (len > p->block_len) {
        printf("data len(%d) larger than block size(%d)\n", len, p->block_len);
        return MBUF_PARAM_OVERFLOW;
    }

    pthread_mutex_lock(&p->lock);
    if (mbuf_is_full(p)) {
        pthread_mutex_unlock(&p->lock);
        printf("ringbuffer is full, write failed\n");
        return MBUF_IS_FULL;
    }

    memcpy(&p->buf[p->tail * p->block_len], data, len);
    p->data_size[p->tail] = len;
    p->tail = (p->tail + 1) % p->block_num;
    p->used_num += 1;

    pthread_mutex_unlock(&p->lock);

    return MBUF_OK;
}

mbuf_err_code mbuf_read_cp(mbuf_handle *p, void *data, uint32_t *len)
{
    if (!(p && data && len)) {
        printf("param error, *p %p, *data %p, *len %p\n", p, data, len);
        return MBUF_PARAM_INVALID;
    }

    pthread_mutex_lock(&p->lock);
    if (mbuf_is_empty(p)) {
        pthread_mutex_unlock(&p->lock);
        return MBUF_IS_EMPTY;
    }

    memcpy(data, &p->buf[p->head * p->block_len], p->data_size[p->head]);
    *len = p->data_size[p->head];
    p->head = (p->head + 1) % p->block_num;
    p->used_num -= 1;

    pthread_mutex_unlock(&p->lock);

    return MBUF_OK;
}

void *mbuf_write_no_cp(mbuf_handle *p, uint32_t len)
{
    if (!(p)) {
        printf("param error, *p %p\n", p);
        return NULL;
    }

    if (len > p->block_len) {
        printf("data len(%d) larger than block size(%d)\n", len, p->block_len);
        return NULL;
    }

    pthread_mutex_lock(&p->lock);
    if (mbuf_is_full(p)) {
        pthread_mutex_unlock(&p->lock);
        printf("ringbuffer is full, write failed\n");
        return NULL;
    }

    p->data_size[p->tail] = len;

    return &p->buf[p->tail * p->block_len];
}

mbuf_err_code mbuf_write_end_no_cp(mbuf_handle *p)
{
    if (!(p)) {
        printf("param error, *p %p\n", p);
        return MBUF_PARAM_NULL_PTR;
    }

    p->tail = (p->tail + 1) % p->block_num;
    p->used_num += 1;

    pthread_mutex_unlock(&p->lock);

    return 0;
}

void *mbuf_read_no_cp(mbuf_handle *p, uint32_t *len)
{
    void *ptr;

    if (!(p)) {
        printf("param error, *p %p\n", p);
        return NULL;
    }

    pthread_mutex_lock(&p->lock);
    if (mbuf_is_empty(p)) {
        pthread_mutex_unlock(&p->lock);
        return NULL;
    }

    ptr = &p->buf[p->head * p->block_len];
    *len = p->data_size[p->head];

    pthread_mutex_unlock(&p->lock);

    return ptr;
}

mbuf_err_code mbuf_read_end_no_cp(mbuf_handle *p)
{
    if (!(p)) {
        printf("param error, *p %p\n", p);
        return MBUF_PARAM_NULL_PTR;
    }

    pthread_mutex_lock(&p->lock);
    if (mbuf_is_empty(p)) {
        pthread_mutex_unlock(&p->lock);
        return MBUF_IS_EMPTY;
    }
    
    p->head = (p->head + 1) % p->block_num;
    p->used_num -= 1;

    pthread_mutex_unlock(&p->lock);

    return MBUF_OK;
}

// 无内存复制读，返回一个内存地址和数据长度，失败返回NULL
void *mbuf_read_no_free(mbuf_handle *p, uint32_t *len)
{
    if (!(p)) {
        printf("param error, *p %p\n", p);
        return NULL;
    }

    pthread_mutex_lock(&p->lock);
    if (mbuf_is_empty(p)) {
        pthread_mutex_unlock(&p->lock);
        return NULL;
    }

    *len = p->data_size[p->head];

    return &p->buf[p->head * p->block_len];
}

// 无内存复制读，读后不释放，与 mbuf_read_no_cp() 配合使用
mbuf_err_code mbuf_read_end_no_free(mbuf_handle *p)
{
    if (!(p)) {
        printf("param error, *p %p\n", p);
        return MBUF_PARAM_NULL_PTR;
    }

    pthread_mutex_unlock(&p->lock);

    return MBUF_OK;
}

// 不读取，直接释放一个缓存
mbuf_err_code mbuf_free(mbuf_handle *p)
{
    return mbuf_read_end_no_cp(p);
}

inline int32_t mbuf_get_used_num(const mbuf_handle *p)
{
    return p->used_num;
}

inline int32_t mbuf_get_free_num(const mbuf_handle *p)
{
    return p->block_num - p->used_num;
}

inline int32_t mbuf_is_empty(const mbuf_handle *p)
{
    return p->used_num == 0;
}

inline int32_t mbuf_is_full(const mbuf_handle *p)
{
    return p->used_num == p->block_num;
}

void mbuf_print(mbuf_handle *p, void(*block_print)(void *, uint32_t))
{
    int32_t i;
    uint32_t len;
    void *block;

    if (mbuf_is_empty(p)) {
        printf("block ringbuffer is empty");
        return;
    }

    block = (void *)mbuf_read_no_cp(p, &len);
    for(i = 0; i < p->used_num; i++)
    {
        block_print(block, len);
        // p->block_print(block);
        block += p->block_len;
        if (block == p->buf + p->total_size) {
            block = p->buf;
        }
    }
}
