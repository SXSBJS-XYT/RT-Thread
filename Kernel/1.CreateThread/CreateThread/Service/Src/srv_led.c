#include "srv_led.h"

/* 私有方法的实现 */
static void _led_init(srv_led_t *self)
{
    self->gpio.init(&self->gpio);
    self->state = LED_OFF;
    self->off(self);  // 默认关闭
}

static void _led_on(srv_led_t *self)
{
    if (self->active_level == LED_ACTIVE_LOW) {
        self->gpio.write(&self->gpio, 0);   // 低电平点亮
    } else {
        self->gpio.write(&self->gpio, 1);   // 高电平点亮
    }
    self->state = LED_ON;
}

static void _led_off(srv_led_t *self)
{
    if (self->active_level == LED_ACTIVE_LOW) {
        self->gpio.write(&self->gpio, 1);   // 高电平熄灭
    } else {
        self->gpio.write(&self->gpio, 0);   // 低电平熄灭
    }
    self->state = LED_OFF;
}

static void _led_toggle(srv_led_t *self)
{
    if (self->state == LED_ON) {
        self->off(self);
    } else {
        self->on(self);
    }
}

static void _led_set(srv_led_t *self, led_state_t state)
{
    if (state == LED_ON) {
        self->on(self);
    } else {
        self->off(self);
    }
}

static led_state_t _led_get_state(srv_led_t *self)
{
    return self->state;
}

/* 构造函数 */
void srv_led_create(srv_led_t *self, void *port, uint16_t pin, led_active_t active)
{
    /* 初始化GPIO对象 */
    drv_gpio_create(&self->gpio, port, pin);
    
    /* 初始化数据 */
    self->active_level = active;
    self->state = LED_OFF;
    
    /* 绑定方法 */
    self->init      = _led_init;
    self->on        = _led_on;
    self->off       = _led_off;
    self->toggle    = _led_toggle;
    self->set       = _led_set;
    self->get_state = _led_get_state;
}
