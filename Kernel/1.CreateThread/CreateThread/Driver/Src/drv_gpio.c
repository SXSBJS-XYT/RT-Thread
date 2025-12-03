#include "drv_gpio.h"
#include "main.h"

/* 私有方法实现 */
static void _gpio_init(drv_gpio_t *self)
{
    // CubeMX已初始化，这里可以留空或做额外配置
}

static void _gpio_write(drv_gpio_t *self, uint8_t state)
{
    HAL_GPIO_WritePin((GPIO_TypeDef *)self->port, 
                      self->pin, 
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t _gpio_read(drv_gpio_t *self)
{
    return HAL_GPIO_ReadPin((GPIO_TypeDef *)self->port, self->pin);
}

static void _gpio_toggle(drv_gpio_t *self)
{
    HAL_GPIO_TogglePin((GPIO_TypeDef *)self->port, self->pin);
}

/* 构造函数 */
void drv_gpio_create(drv_gpio_t *self, void *port, uint16_t pin)
{
    /* 初始化数据 */
    self->port = port;
    self->pin  = pin;
    
    /* 绑定方法 */
    self->init   = _gpio_init;
    self->write  = _gpio_write;
    self->read   = _gpio_read;
    self->toggle = _gpio_toggle;
}
