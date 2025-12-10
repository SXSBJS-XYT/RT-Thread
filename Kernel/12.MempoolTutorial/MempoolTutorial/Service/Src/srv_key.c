// Service/Src/srv_key.c
#include "srv_key.h"
#include <rtthread.h>

/* 默认参数 */
#define DEFAULT_DEBOUNCE_MS   20   // 消抖时间20ms
#define DEFAULT_LONG_PRESS_MS 1000 // 长按判定1秒

/* 获取当前时间（毫秒） */
static uint32_t _get_tick_ms(void)
{
  return rt_tick_get_millisecond();
}

/* 初始化 */
static void _key_init(srv_key_t *self)
{
  self->gpio.init(&self->gpio);
  self->state         = KEY_STATE_IDLE;
  self->press_tick    = 0;
  self->debounce_ms   = DEFAULT_DEBOUNCE_MS;
  self->long_press_ms = DEFAULT_LONG_PRESS_MS;
  self->callback      = RT_NULL;
}

/* 读取按键原始状态（是否按下） */
static uint8_t _key_is_pressed(srv_key_t *self)
{
  uint8_t level = self->gpio.read(&self->gpio);

  if (self->active_level == KEY_ACTIVE_LOW)
  {
    return (level == 0) ? 1 : 0; // 低电平表示按下
  }
  else
  {
    return (level == 1) ? 1 : 0; // 高电平表示按下
  }
}

/* 按键扫描（状态机） */
static key_event_t _key_scan(srv_key_t *self)
{
  key_event_t event     = KEY_EVENT_NONE;
  uint8_t is_pressed    = self->is_pressed(self);
  uint32_t current_tick = _get_tick_ms();

  switch (self->state)
  {
    case KEY_STATE_IDLE:
      if (is_pressed)
      {
        self->state      = KEY_STATE_DEBOUNCE;
        self->press_tick = current_tick;
      }
      break;

    case KEY_STATE_DEBOUNCE:
      if (is_pressed)
      {
        if ((current_tick - self->press_tick) >= self->debounce_ms)
        {
          self->state      = KEY_STATE_PRESSED;
          self->press_tick = current_tick;
          event            = KEY_EVENT_PRESSED;
        }
      }
      else
      {
        self->state = KEY_STATE_IDLE; // 抖动，回到空闲
      }
      break;

    case KEY_STATE_PRESSED:
      if (!is_pressed)
      {
        self->state = KEY_STATE_IDLE;
        event       = KEY_EVENT_SHORT_PRESS; // 短按释放
      }
      else if ((current_tick - self->press_tick) >= self->long_press_ms)
      {
        self->state = KEY_STATE_LONG_PRESS;
        event       = KEY_EVENT_LONG_PRESS; // 进入长按
      }
      break;

    case KEY_STATE_LONG_PRESS:
      if (!is_pressed)
      {
        self->state = KEY_STATE_IDLE;
        event       = KEY_EVENT_RELEASED; // 长按后释放
      }
      break;

    default:
      self->state = KEY_STATE_IDLE;
      break;
  }

  /* 触发回调 */
  if (event != KEY_EVENT_NONE && self->callback != RT_NULL)
  {
    self->callback(event);
  }

  return event;
}

/* 设置回调函数 */
static void _key_set_callback(srv_key_t *self, key_callback_t cb)
{
  self->callback = cb;
}

/* 构造函数 */
void srv_key_create(srv_key_t *self, void *port, uint16_t pin, key_active_t active)
{
  /* 初始化GPIO对象 */
  drv_gpio_create(&self->gpio, port, pin);

  /* 初始化数据 */
  self->active_level  = active;
  self->state         = KEY_STATE_IDLE;
  self->press_tick    = 0;
  self->debounce_ms   = DEFAULT_DEBOUNCE_MS;
  self->long_press_ms = DEFAULT_LONG_PRESS_MS;
  self->callback      = RT_NULL;

  /* 绑定方法 */
  self->init         = _key_init;
  self->is_pressed   = _key_is_pressed;
  self->scan         = _key_scan;
  self->set_callback = _key_set_callback;
}
