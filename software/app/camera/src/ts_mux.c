#include "ts_mux.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "mpeg-ts-proto.h"
#include "mpeg-ts.h"

// 一般TS流包长度为188字节或204字节
#define TS_PACK_SIZE 204

typedef struct {
    FILE *fp;
    void *mpeg_ts_handle;
    int video_stream_id; // 必须初始化为-1，因为初始化失败也可能是0，导致音频与视频id重叠
    int audio_stream_id; // 必须初始化为-1，因为初始化失败也可能是0，导致音频与视频id重叠
    uint8_t ts_pkt_buf[TS_PACK_SIZE];
    pthread_mutex_t lock; // TODO
} ts_base_handle;

ts_err_code ts_init(void **ts_handle)
{
    if (ts_handle == NULL) {
        return TS_PARAM_NULL_PTR;
    }

    ts_base_handle *base_handle = malloc(sizeof(ts_base_handle));
    if (!base_handle) {
        goto end;
    }
    memset(base_handle, 0, sizeof(ts_base_handle));
    pthread_mutex_init(&base_handle->lock, NULL);

    base_handle->audio_stream_id = -1;
    base_handle->video_stream_id = -1;

    *ts_handle = (void *)base_handle;

    return TS_OK;

end:
    *ts_handle = NULL;
    ts_exit((void *)&base_handle);
    return TS_OTHER_ERR;
}

ts_err_code ts_exit(void **ts_handle)
{
    ts_base_handle *base_handle = (ts_base_handle *)*ts_handle;

    if (base_handle) {
        if (base_handle->fp) {
            return TS_FILE_NOT_CLOSE;
        }
        if (base_handle->mpeg_ts_handle) {
            mpeg_ts_destroy(base_handle->mpeg_ts_handle);
        }
        pthread_mutex_destroy(&base_handle->lock);
        free(base_handle);
        *ts_handle = NULL;
    }

    return TS_OK;
}

static void *_ts_func_alloc(void *param, size_t bytes)
{
    ts_base_handle *base_handle = (ts_base_handle *)param;

    if (bytes <= TS_PACK_SIZE) {
        return base_handle->ts_pkt_buf;
    } else {
        // printf("alloc ts buffer failed, size(%d) larger than %d", bytes, TS_PACK_SIZE);
        return NULL;
    }
}

static void _ts_func_free(void *param, void *packet)
{
    return;
}

static int _ts_func_write(void *param, const void *packet, size_t bytes)
{
    ts_base_handle *base_handle = (ts_base_handle *)param;
    int ret = 0;

    ret = fwrite(packet, 1, bytes, base_handle->fp);
    if (ret == bytes) {
        return 0;
    }
    else {
        // printf("write ts file error");
        return 1;
    }
}

static struct mpeg_ts_func_t ts_func = {
    .alloc = _ts_func_alloc,
    .write = _ts_func_write,
    .free  = _ts_func_free,
};

ts_err_code ts_open_file(void *ts_handle, char venc_type, char *file_name)
{
    ts_base_handle *base_handle = (ts_base_handle *)ts_handle;

    if (ts_handle == NULL) {
        // printf("ts_handle not create");
        return TS_PARAM_NULL_PTR;
    }

    if (access(file_name, F_OK) == 0) {
        // printf("file %s exist\n", file_names);
        return TS_FILE_OTHER_ERROR;
    }
    base_handle->fp = fopen(file_name, "wb");
    if (base_handle->fp == NULL) {
        // printf("open file %s error", DUMP_TS_PATH);
        return TS_FILE_NOT_OPEN;
    }

    base_handle->mpeg_ts_handle = mpeg_ts_create(&ts_func, ts_handle);
    base_handle->audio_stream_id = mpeg_ts_add_stream(base_handle->mpeg_ts_handle, PSI_STREAM_AAC, NULL, 0);
    if (venc_type == '4') {
        base_handle->video_stream_id = mpeg_ts_add_stream(base_handle->mpeg_ts_handle, PSI_STREAM_H264, NULL, 0);
    } else if (venc_type == '5') {
        base_handle->video_stream_id = mpeg_ts_add_stream(base_handle->mpeg_ts_handle, PSI_STREAM_H265, NULL, 0);
    } else {
        // printf("unknown venc type %d", venc_type);
        return TS_PARAM_INVALID;
    }

    return TS_OK;
}

ts_err_code ts_close_file(void *ts_handle)
{
    ts_base_handle *base_handle = (ts_base_handle *)ts_handle;

    if (ts_handle == NULL) {
        // printf("ts_handle not create");
        return TS_PARAM_NULL_PTR;
    }

    if (base_handle->mpeg_ts_handle == NULL) {
        printf("ts_handle is closed");
        return TS_OK;
    }

    mpeg_ts_destroy(base_handle->mpeg_ts_handle);
    base_handle->mpeg_ts_handle = NULL;

    if (base_handle->fp) {
        fclose(base_handle->fp);
        base_handle->fp = NULL;
    }

    return TS_OK;
}

ts_err_code ts_write_file(void *ts_handle, uint8_t *data, uint32_t len, int64_t pts, char type, int is_i_frame)
{
    ts_base_handle *base_handle = (ts_base_handle *)ts_handle;
    int32_t ts_mpeg_flag          = 0;

    if (ts_handle == NULL) {
        // printf("ts_handle not create");
        return TS_PARAM_NULL_PTR;
    }

    if (base_handle->mpeg_ts_handle == NULL) {
        return TS_FILE_NOT_OPEN;
    }

    pthread_mutex_lock(&base_handle->lock);
    if (type == 'a') {
        mpeg_ts_write(base_handle->mpeg_ts_handle, base_handle->audio_stream_id, 0,
                      pts * 90, pts * 90, data, len);
    }
    if (type == 'v') {
        if (is_i_frame) {
            ts_mpeg_flag = MPEG_FLAG_IDR_FRAME;
        } else {
            ts_mpeg_flag = 0;
        }

        mpeg_ts_write(base_handle->mpeg_ts_handle, base_handle->video_stream_id, ts_mpeg_flag,
                      pts * 90, pts * 90, data, len);
    }
    pthread_mutex_unlock(&base_handle->lock);

    return TS_OK;
}

int32_t ts_is_file_open(void *ts_handle)
{
    ts_base_handle *base_handle = (ts_base_handle *)ts_handle;

    return base_handle ? (base_handle->fp != NULL) : 0;
}
