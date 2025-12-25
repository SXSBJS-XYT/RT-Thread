/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-12-25     RT-Thread    first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define DEMO3

#define LED_PIN    GET_PIN(B, 1)
#define KEY_PIN    GET_PIN(C, 13)

//LED闪烁（最基础）
#ifdef DEMO1

int main(void)
{
    /* 配置为输出模式 */
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    while (1)
    {
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}
//按键中断控制 LED
#elif defined(DEMO2)

static rt_uint8_t led_state = PIN_LOW;

/* 中断回调 - 注意：在中断上下文执行，要快进快出 */
static void key_irq_callback(void *args)
{
    led_state = !led_state;
    rt_pin_write(LED_PIN, led_state);
}

int main(void)
{
    /* LED 输出 */
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(LED_PIN, led_state);

    /* 按键输入 + 下降沿中断 */
    rt_pin_mode(KEY_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(KEY_PIN, PIN_IRQ_MODE_FALLING, key_irq_callback, RT_NULL);
    rt_pin_irq_enable(KEY_PIN, PIN_IRQ_ENABLE);

    while (1)
    {
        rt_thread_mdelay(1000);  /* 主循环可以做其他事 */
    }
}
//中断 + 信号量（推荐模式）
#elif defined(DEMO3)

static struct rt_semaphore key_sem;

static void key_irq_callback(void *args)
{
    /* 中断中只释放信号量，不做耗时操作 */
    rt_sem_release(&key_sem);
}

static void key_process_thread(void *param)
{
    rt_uint8_t led_state = PIN_LOW;

    while (1)
    {
        /* 等待按键信号 */
        rt_sem_take(&key_sem, RT_WAITING_FOREVER);

        /* 简单消抖 */
        rt_thread_mdelay(20);
        if (rt_pin_read(KEY_PIN) == PIN_LOW)
        {
            led_state = !led_state;
            rt_pin_write(LED_PIN, led_state);
            rt_kprintf("LED: %s\n", led_state ? "ON" : "OFF");
        }
    }
}

int main(void)
{
    rt_sem_init(&key_sem, "key_sem", 0, RT_IPC_FLAG_FIFO);

    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(KEY_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(KEY_PIN, PIN_IRQ_MODE_FALLING, key_irq_callback, RT_NULL);
    rt_pin_irq_enable(KEY_PIN, PIN_IRQ_ENABLE);

    /* 创建处理线程 */
    rt_thread_t tid = rt_thread_create("key_proc", key_process_thread, RT_NULL,
                                       1024, 10, 20);
    if (tid) rt_thread_startup(tid);

    return 0;
}

#endif
