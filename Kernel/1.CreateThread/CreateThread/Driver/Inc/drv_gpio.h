#ifndef __DRV_GPIO_H
#define __DRV_GPIO_H

#include <stdint.h>

/* GPIO对象结构体 */
typedef struct drv_gpio drv_gpio_t;

struct drv_gpio
{
  /* 私有数据 */
  void *port;   // GPIO端口
  uint16_t pin; // GPIO引脚

  /* 方法 */
  void (*init)(drv_gpio_t *self);
  void (*write)(drv_gpio_t *self, uint8_t state);
  uint8_t (*read)(drv_gpio_t *self);
  void (*toggle)(drv_gpio_t *self);
};

/* 构造函数 */
void drv_gpio_create(drv_gpio_t *self, void *port, uint16_t pin);

#endif
