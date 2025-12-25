# RT-Thread PIN 设备驱动学习笔记

## 1. 概述

PIN 设备是 RT-Thread 中最简单的设备驱动，用于 GPIO 控制。它不使用标准的 `rt_device_read/write` 接口，而是提供专用 API，追求简洁高效。

## 2. 架构层次

```
┌─────────────────────────────────────────────────────────────┐
│                      应用层                                  │
│         rt_pin_mode() / rt_pin_write() / rt_pin_read()      │
├─────────────────────────────────────────────────────────────┤
│                    PIN 框架层                                │
│     components/drivers/pin/pin.c                            │
│     定义 struct rt_pin_ops，提供统一接口                     │
├─────────────────────────────────────────────────────────────┤
│                   BSP 驱动层                                 │
│     bsp/stm32/libraries/HAL_Drivers/drv_gpio.c             │
│     实现 ops，调用 HAL_GPIO_xxx()                           │
└─────────────────────────────────────────────────────────────┘
```

## 3. 核心 API

| API | 功能 | 说明 |
|-----|------|------|
| `rt_pin_mode(pin, mode)` | 配置引脚模式 | 输入/输出/上拉/下拉/开漏 |
| `rt_pin_write(pin, value)` | 写电平 | PIN_HIGH / PIN_LOW |
| `rt_pin_read(pin)` | 读电平 | 返回 PIN_HIGH 或 PIN_LOW |
| `rt_pin_attach_irq(pin, mode, hdr, args)` | 绑定中断回调 | 设置触发方式和回调函数 |
| `rt_pin_detach_irq(pin)` | 解绑中断 | - |
| `rt_pin_irq_enable(pin, enabled)` | 使能/禁用中断 | PIN_IRQ_ENABLE / PIN_IRQ_DISABLE |

### 3.1 引脚模式

```c
#define PIN_MODE_OUTPUT         0x00    /* 推挽输出 */
#define PIN_MODE_INPUT          0x01    /* 浮空输入 */
#define PIN_MODE_INPUT_PULLUP   0x02    /* 上拉输入 */
#define PIN_MODE_INPUT_PULLDOWN 0x03    /* 下拉输入 */
#define PIN_MODE_OUTPUT_OD      0x04    /* 开漏输出 */
```

### 3.2 中断触发模式

```c
#define PIN_IRQ_MODE_RISING             0x00    /* 上升沿 */
#define PIN_IRQ_MODE_FALLING            0x01    /* 下降沿 */
#define PIN_IRQ_MODE_RISING_FALLING     0x02    /* 双边沿 */
#define PIN_IRQ_MODE_HIGH_LEVEL         0x03    /* 高电平 */
#define PIN_IRQ_MODE_LOW_LEVEL          0x04    /* 低电平 */
```

### 3.3 引脚编号

使用 `GET_PIN()` 宏将端口+引脚转换为统一编号：

```c
#define LED_PIN    GET_PIN(B, 5)   /* PB5 */
#define KEY_PIN    GET_PIN(C, 13)  /* PC13 */
```

## 4. 框架实现原理

### 4.1 操作函数集定义

```c
struct rt_pin_ops
{
    void (*pin_mode)(struct rt_device *device, rt_base_t pin, rt_uint8_t mode);
    void (*pin_write)(struct rt_device *device, rt_base_t pin, rt_uint8_t value);
    rt_int8_t (*pin_read)(struct rt_device *device, rt_base_t pin);
    rt_err_t (*pin_attach_irq)(...);
    rt_err_t (*pin_detach_irq)(...);
    rt_err_t (*pin_irq_enable)(...);
    rt_base_t (*pin_get)(const char *name);
};
```

### 4.2 API 到 ops 的调用链

```c
void rt_pin_write(rt_base_t pin, rt_uint8_t value)
{
    _hw_pin.ops->pin_write(&_hw_pin.parent, pin, value);
}
```

### 4.3 BSP 注册

```c
/* drv_gpio.c */
const static struct rt_pin_ops _stm32_pin_ops = {
    stm32_pin_mode,
    stm32_pin_write,
    stm32_pin_read,
    stm32_pin_attach_irq,
    stm32_pin_detach_irq,
    stm32_pin_irq_enable,
    stm32_pin_get,
};

int rt_hw_pin_init(void)
{
    return rt_device_pin_register("pin", &_stm32_pin_ops, RT_NULL);
}
INIT_BOARD_EXPORT(rt_hw_pin_init);
```

## 5. Demo 代码

### 5.1 LED 闪烁（基础）

```c
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define LED_PIN    GET_PIN(B, 5)

int main(void)
{
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    
    while (1)
    {
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}
```

### 5.2 按键中断控制 LED

```c
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define LED_PIN    GET_PIN(B, 5)
#define KEY_PIN    GET_PIN(C, 13)

static rt_uint8_t led_state = PIN_LOW;

static void key_irq_callback(void *args)
{
    led_state = !led_state;
    rt_pin_write(LED_PIN, led_state);
}

int main(void)
{
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(LED_PIN, led_state);
    
    rt_pin_mode(KEY_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(KEY_PIN, PIN_IRQ_MODE_FALLING, key_irq_callback, RT_NULL);
    rt_pin_irq_enable(KEY_PIN, PIN_IRQ_ENABLE);
    
    while (1)
    {
        rt_thread_mdelay(1000);
    }
}
```

### 5.3 中断 + 信号量（推荐模式）

```c
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define LED_PIN    GET_PIN(B, 5)
#define KEY_PIN    GET_PIN(C, 13)

static struct rt_semaphore key_sem;

static void key_irq_callback(void *args)
{
    rt_sem_release(&key_sem);  /* 中断中只释放信号量 */
}

static void key_process_thread(void *param)
{
    rt_uint8_t led_state = PIN_LOW;
    
    while (1)
    {
        rt_sem_take(&key_sem, RT_WAITING_FOREVER);
        
        /* 消抖 */
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
    
    rt_thread_t tid = rt_thread_create("key_proc", key_process_thread, RT_NULL,
                                       1024, 10, 20);
    if (tid) rt_thread_startup(tid);
    
    return 0;
}
```

## 6. 最佳实践

1. **中断回调要快进快出**：不要在回调中做耗时操作，用信号量/邮箱通知线程处理
2. **按键消抖**：软件消抖 10~20ms，或硬件 RC 滤波
3. **引脚编号统一用宏**：`GET_PIN(X, n)` 提高可移植性
4. **中断优先级**：RT-Thread 的 PIN 中断默认优先级，如需调整要修改 BSP

## 7. 相关源码路径

```
components/drivers/include/drivers/pin.h   -- 接口定义
components/drivers/pin/pin.c               -- 框架实现
bsp/stm32/libraries/HAL_Drivers/drv_gpio.c -- STM32 驱动实现
```

## 8. 常见问题

**Q: 中断不触发？**
- 检查是否调用了 `rt_pin_irq_enable()`
- 检查引脚模式是否配置正确（需要输入模式）
- 检查 menuconfig 中是否使能了对应的 EXTI

**Q: GET_PIN 宏算出来的值不对？**
- 不同 BSP 的实现可能不同，查看具体 BSP 的 `drv_gpio.c`

---

