#include "app_task.h"
#include "main.h"
#include "srv_key.h"
#include "srv_led.h"
#include <rtthread.h>

/* LED对象 */
static srv_led_t led0;
static srv_led_t led1;

/* 按键对象 */
static srv_key_t key0;

/* 线程相关 */
static struct rt_thread led_thread;
static rt_uint8_t led_stack[256];

static struct rt_thread key_thread;
static rt_uint8_t key_stack[256];

static struct rt_thread management_thread;
static rt_uint8_t management_stack[256];

static rt_uint8_t suspend_flag = 0;
static rt_uint8_t resume_flag  = 0;

/* 按键回调函数 */
static void key0_callback(key_event_t event)
{
  switch (event)
  {
    case KEY_EVENT_SHORT_PRESS:
      rt_kprintf("KEY0: Short Press\n");
      suspend_flag = 1;
      break;

    case KEY_EVENT_LONG_PRESS:
      rt_kprintf("KEY0: Long Press\n");
      resume_flag = 1;
      break;

    case KEY_EVENT_RELEASED:
      rt_kprintf("KEY0: Released\n");
      break;

    default:
      break;
  }
}

static void led_thread_entry(void *param)
{
  while (1)
  {
    led0.toggle(&led0);
    led1.off(&led1);

    if (suspend_flag == 1)
    {
      suspend_flag = 0;
      led1.on(&led1);
      rt_thread_suspend(&led_thread);
      rt_kprintf("led_thread suspend!\n");
      rt_schedule();
    }

    rt_thread_mdelay(500);
  }
}

static void key_thread_entry(void *param)
{
  while (1)
  {
    key0.scan(&key0);
    rt_thread_mdelay(10);
  }
}

static void management_thread_entry(void *param)
{
  while (1)
  {
    if (resume_flag == 1)
    {
      resume_flag = 0;
      rt_thread_resume(&led_thread);
      rt_kprintf("led_thread resume!\n");
    }

    rt_thread_mdelay(10);
  }
}

/* 任务初始化函数 */
int app_task_init(void)
{
  /* 创建LED对象 */
  srv_led_create(&led0, LED0_GPIO_Port, LED0_Pin, LED_ACTIVE_LOW);
  srv_led_create(&led1, LED1_GPIO_Port, LED1_Pin, LED_ACTIVE_LOW);
  led0.init(&led0);
  led1.init(&led1);

  /* 创建按键对象 */
  srv_key_create(&key0, KEY0_GPIO_Port, KEY0_Pin, KEY_ACTIVE_LOW);
  key0.init(&key0);
  /* 设置按键回调 */
  key0.set_callback(&key0, key0_callback);

  /* 创建LED线程 */
  rt_thread_init(&led_thread, "led", led_thread_entry, RT_NULL, led_stack, sizeof(led_stack),
                 PRIORITY_LED, 10);
  rt_thread_startup(&led_thread);

  /* 创建按键线程 */
  rt_thread_init(&key_thread, "key", key_thread_entry, RT_NULL, key_stack, sizeof(key_stack),
                 PRIORITY_KEY, // 优先级比LED高
                 10);
  rt_thread_startup(&key_thread);

  /* 创建按键线程 */
  rt_thread_init(&management_thread, "management", management_thread_entry, RT_NULL,
                 management_stack, sizeof(management_stack), PRIORITY_MANAGEM, 10);
  rt_thread_startup(&management_thread);

  return 0;
}
INIT_APP_EXPORT(app_task_init);
