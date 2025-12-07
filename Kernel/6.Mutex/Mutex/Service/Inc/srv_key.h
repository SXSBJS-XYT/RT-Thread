#ifndef __SRV_KEY_H
#define __SRV_KEY_H

#include <stdint.h>
#include "drv_gpio.h"

/* 按键活跃电平 */
typedef enum
{
  KEY_ACTIVE_LOW = 0, // 按下时低电平
  KEY_ACTIVE_HIGH     // 按下时高电平
} key_active_t;

/* 按键状态 */
typedef enum
{
  KEY_STATE_IDLE = 0,  // 空闲
  KEY_STATE_DEBOUNCE,  // 消抖中
  KEY_STATE_PRESSED,   // 已按下
  KEY_STATE_LONG_PRESS // 长按中
} key_state_t;

/* 按键事件 */
typedef enum
{
  KEY_EVENT_NONE = 0,    // 无事件
  KEY_EVENT_PRESSED,     // 按下事件
  KEY_EVENT_RELEASED,    // 释放事件
  KEY_EVENT_SHORT_PRESS, // 短按事件（释放时触发）
  KEY_EVENT_LONG_PRESS   // 长按事件
} key_event_t;

/* 按键回调函数类型 */
typedef void (*key_callback_t)(key_event_t event);

/* 按键对象结构体 */
typedef struct srv_key srv_key_t;

struct srv_key
{
  /* 私有数据 */
  drv_gpio_t gpio;           // 组合GPIO对象
  key_active_t active_level; // 活跃电平
  key_state_t state;         // 当前状态
  uint32_t press_tick;       // 按下时刻
  uint32_t debounce_ms;      // 消抖时间
  uint32_t long_press_ms;    // 长按判定时间
  key_callback_t callback;   // 事件回调函数

  /* 方法 */
  void (*init)(srv_key_t *self);
  uint8_t (*is_pressed)(srv_key_t *self);
  key_event_t (*scan)(srv_key_t *self);
  void (*set_callback)(srv_key_t *self, key_callback_t cb);
};

/* 构造函数 */
void srv_key_create(srv_key_t *self, void *port, uint16_t pin, key_active_t active);

#endif
