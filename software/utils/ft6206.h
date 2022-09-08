#ifndef _FT6206_H
#define _FT6206_H

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#include <stdint.h>

/*
查阅官方手册发现根本没写设备地址，从网上查到的资料发现有两种地址，分别是0x2A和0x38
实际测试这块屏幕的地址是0x38
屏幕宽的一边在下，从正方向看，坐标原点在屏幕右下角，坐标范围是0-128
*/
// #define FT6202_I2C_ADDR 0x2A
#define FT6202_I2C_ADDR 0x38

#define FT6206_MAX_TOUCH_POINT_NUM 2

#define FT6206_GESTURE_MOVE_UP    0x10
#define FT6206_GESTURE_MOVE_RIGHT 0x14
#define FT6206_GESTURE_MOVE_DOWN  0x18
#define FT6206_GESTURE_MOVE_LEFT  0x1C
#define FT6206_GESTURE_ZOOM_IN    0x48
#define FT6206_GESTURE_ZOOM_OUT   0x49
#define FT6206_GESTURE_NULL       0x00

// 原
#define FT6206_EVENT_NULL       0x03
#define FT6206_EVENT_PRESS_DOWN 0x00
#define FT6206_EVENT_LIFT_UP    0x01
#define FT6206_EVENT_CONTACT    0x02

typedef struct {
	uint16_t x;
	uint16_t y;
	uint8_t weight;
	uint8_t area;
	uint8_t event; // 参见 FT6206_EVENT_XXX
	uint8_t touch_id;
} ft6206_touch_point;

typedef struct {
	ft6206_touch_point points[FT6206_MAX_TOUCH_POINT_NUM];
	uint8_t num;
	uint8_t gesture; // 参见 FT6206_GESTURE_XXX
} ft6206_touch_info;

int8_t ft6206_init(void *args);
void ft6206_exit(void *args);
int8_t ft6206_read(void *args, ft6206_touch_info *info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // _FT6206_H
