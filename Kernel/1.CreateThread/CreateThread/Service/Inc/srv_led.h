#ifndef __SRV_LED_H
#define __SRV_LED_H

#include <stdint.h>
#include "drv_gpio.h"

/* LED活跃电平定义 */
typedef enum {
    LED_ACTIVE_LOW = 0,     // 低电平点亮
    LED_ACTIVE_HIGH         // 高电平点亮
} led_active_t;

/* LED状态 */
typedef enum {
    LED_OFF = 0,
    LED_ON
} led_state_t;

/* LED对象结构体 */
typedef struct srv_led srv_led_t;

struct srv_led {
    /* 私有数据 */
    drv_gpio_t   gpio;          // 组合GPIO对象
    led_active_t active_level;  // 活跃电平
    led_state_t  state;         // 当前状态
    
    /* 方法 */
    void (*init)(srv_led_t *self);
    void (*on)(srv_led_t *self);
    void (*off)(srv_led_t *self);
    void (*toggle)(srv_led_t *self);
    void (*set)(srv_led_t *self, led_state_t state);
    led_state_t (*get_state)(srv_led_t *self);
};

/* 构造函数 */
void srv_led_create(srv_led_t *self, void *port, uint16_t pin, led_active_t active);

#endif
