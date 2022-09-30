#ifndef _TS_PACK_H
#define _TS_PACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    TS_OK,
    TS_PARAM_NULL_PTR,
    TS_PARAM_OVERFLOW,
    TS_PARAM_INVALID,
    TS_FILE_NOT_OPEN,
    TS_FILE_NOT_CLOSE,
    TS_FILE_OTHER_ERROR,
    TS_OTHER_ERR,
} ts_err_code;

// ts_err_code ts_init(void **ts_handle, void *rb_handle);
ts_err_code ts_init(void **ts_handle);
ts_err_code ts_exit(void **ts_handle);
// venc_type: '4'代表h264，'5'代表h265
ts_err_code ts_open_file(void *ts_handle, char venc_type, char *file_name);
ts_err_code ts_close_file(void *ts_handle);
// pts单位毫秒，type: 'a'代表音频数据，'v'代表视频数据
ts_err_code ts_write_file(void *ts_handle, uint8_t *data, uint32_t len, int64_t pts, char type, int32_t is_i_frame);
int32_t ts_is_file_open(void *ts_handle);

#ifdef __cplusplus
}
#endif

#endif
