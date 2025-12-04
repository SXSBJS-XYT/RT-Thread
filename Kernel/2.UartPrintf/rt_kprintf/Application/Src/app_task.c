#include "app_task.h"
#include "main.h"
#include <rtthread.h>


/* 线程相关 */
static struct rt_thread printf_thread;
static rt_uint8_t printf_stack[512];

extern rt_uint32_t cnt;

/* printf线程入口 */
static void printf_thread_entry(void *param)
{

    while (1)
    {
        rt_kprintf("cnt = %d", cnt);
        rt_thread_mdelay(500);
    }
}

/* 任务初始化函数 */
void app_task_init(void)
{
    /* 创建线程 */
    rt_thread_init(&printf_thread, "printf", printf_thread_entry, RT_NULL, printf_stack,
                   sizeof(printf_stack), 20, 10);
    rt_thread_startup(&printf_thread);
}
