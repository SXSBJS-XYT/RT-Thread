#include "app_task.h"
#include "srv_led.h"
#include "main.h"
#include <rtthread.h>

/* LED对象实例 */
static srv_led_t led1;
static srv_led_t led2;

/* 线程相关 */
static struct rt_thread led_thread;
static rt_uint8_t led_stack[512];

/* LED线程入口 */
static void led_thread_entry(void *param)
{
    /* 初始化LED对象 */
    led2.init(&led2);
    led1.init(&led1);
    
    while (1)
    {
        led2.toggle(&led2);     // LED2翻转
        rt_thread_mdelay(500);
        
        led1.toggle(&led1);     // LED1翻转
        rt_thread_mdelay(500);
    }
}

/* 任务初始化函数 */
void app_task_init(void)
{
    /* 创建LED对象（构造） */
    srv_led_create(&led1, LED1_GPIO_Port, LED1_Pin, LED_ACTIVE_LOW);
	  srv_led_create(&led2, LED2_GPIO_Port, LED2_Pin, LED_ACTIVE_LOW);
    
    /* 创建线程 */
    rt_thread_init(&led_thread,
                   "led",
                   led_thread_entry,
                   RT_NULL,
                   led_stack,
                   sizeof(led_stack),
                   20,
                   10);
    rt_thread_startup(&led_thread);
}
