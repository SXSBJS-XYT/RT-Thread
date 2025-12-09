/**
 * @file    app_task.c
 * @brief   优先级翻转演示 - 对比信号量与互斥锁
 *
 * 实验说明：
 *   - 三个任务：Task_H(高)、Task_M(中)、Task_L(低)
 *   - 修改 USE_MUTEX 宏来切换使用互斥锁或信号量
 *   - 观察串口输出，对比两种情况下的任务执行顺序
 *
 * 预期结果：
 *   USE_MUTEX = 0 (信号量)：发生优先级翻转，Task_M 抢占 Task_L
 *   USE_MUTEX = 1 (互斥锁)：优先级继承，Task_L 不被 Task_M 抢占
 */
#include "app_task.h"
#include "main.h"
#include <rtthread.h>


#define USE_MUTEX 0 /* 0=信号量(会翻转), 1=互斥锁(有继承) */

// 任务优先级定义 (数值越小优先级越高)
#define PRIO_HIGH   8
#define PRIO_MEDIUM 15
#define PRIO_LOW    22

#define THREAD_STACK_SIZE 1024
#define THREAD_TIMESLICE  5

// 全局变量
#if USE_MUTEX
static rt_mutex_t g_lock;
#else
static rt_sem_t g_lock;
#endif

static rt_thread_t tid_high, tid_medium, tid_low;
static rt_tick_t start_tick;


// 辅助函数：打印带时间戳的消息
static void log_msg(const char *task, const char *msg)
{
  rt_tick_t elapsed = rt_tick_get() - start_tick;
  rt_kprintf("[%4d ms] %-8s: %s\n", elapsed * (1000 / RT_TICK_PER_SECOND), task, msg);
}

// 高优先级任务
static void thread_high_entry(void *param)
{
  rt_tick_t wait_start, wait_time;

  /* Delay to ensure Task_L gets lock first */
  rt_thread_mdelay(50);

  log_msg("Task_H", "Try to acquire lock...");
  wait_start = rt_tick_get();

#if USE_MUTEX
  rt_mutex_take(g_lock, RT_WAITING_FOREVER);
#else
  rt_sem_take(g_lock, RT_WAITING_FOREVER);
#endif

  wait_time = rt_tick_get() - wait_start;

  log_msg("Task_H", ">>> GOT LOCK! <<<");

  /* Simulate short work */
  rt_thread_mdelay(20);

#if USE_MUTEX
  rt_mutex_release(g_lock);
#else
  rt_sem_release(g_lock);
#endif

  log_msg("Task_H", "Done");

  /* Print summary */
  rt_kprintf("\n============ RESULT ============\n");
  rt_kprintf("Task_H waited: %d ms\n", wait_time * (1000 / RT_TICK_PER_SECOND));
  rt_kprintf("Expected (no inversion): ~240 ms\n");
#if USE_MUTEX
  rt_kprintf("Mode: MUTEX (priority inheritance)\n");
#else
  rt_kprintf("Mode: SEMAPHORE (no inheritance)\n");
  if (wait_time * (1000 / RT_TICK_PER_SECOND) > 300)
  {
    rt_kprintf(">>> PRIORITY INVERSION DETECTED! <<<\n");
  }
#endif
  rt_kprintf("================================\n");
}

// 中优先级任务
static void thread_medium_entry(void *param)
{
  volatile uint32_t cnt;

  /* Delay to start after Task_H is blocked */
  rt_thread_mdelay(80);

  log_msg("Task_M", "=== START! Busy loop, blocking Task_L ===");

  /* BUSY WAIT - does NOT yield CPU! */
  /* This will preempt Task_L and cause priority inversion */
  for (int i = 0; i < 5; i++)
  {
    cnt = 0;
    while (cnt < 2000000)
      cnt++; /* ~200ms total busy wait */
  }

  log_msg("Task_M", "=== DONE ===");
}

// 低优先级任务
static void thread_low_entry(void *param)
{
  volatile uint32_t cnt;

  log_msg("Task_L", "Acquire lock");

#if USE_MUTEX
  rt_mutex_take(g_lock, RT_WAITING_FOREVER);
#else
  rt_sem_take(g_lock, RT_WAITING_FOREVER);
#endif

  log_msg("Task_L", "GOT lock, start critical section (busy)...");

  /* Simulate long operation using BUSY WAIT */
  /* This keeps CPU busy, can be preempted by higher priority tasks */
  /* Total time ~240ms if not preempted */
  for (int i = 0; i < 8; i++)
  {
    cnt = 0;
    while (cnt < 800000)
      cnt++; /* ~30ms each iteration */
  }

  log_msg("Task_L", "Release lock");

#if USE_MUTEX
  rt_mutex_release(g_lock);
#else
  rt_sem_release(g_lock);
#endif
}

/* 任务初始化函数 */
int priority_inversion_demo(void)
{
#if USE_MUTEX
  rt_kprintf("\n  Priority Inversion Demo - Using [MUTEX] (has inheritance)\n");
#else
  rt_kprintf("\n  Priority Inversion Demo - Using [SEMAPHORE] (no inheritance)\n");
#endif
  rt_kprintf("============================================================\n");
  rt_kprintf("  Task_H priority: %d (highest)\n", PRIO_HIGH);
  rt_kprintf("  Task_M priority: %d (medium)\n", PRIO_MEDIUM);
  rt_kprintf("  Task_L priority: %d (lowest)\n", PRIO_LOW);
  rt_kprintf("============================================================\n\n");

  /* 记录开始时间 */
  start_tick = rt_tick_get();

  /* 创建锁 */
#if USE_MUTEX
  g_lock = rt_mutex_create("demo_mtx", RT_IPC_FLAG_PRIO);
#else
  g_lock = rt_sem_create("demo_sem", 1, RT_IPC_FLAG_PRIO);
#endif

  if (g_lock == RT_NULL)
  {
    rt_kprintf("Failed to create lock!\n");
    return -1;
  }

  /* 创建低优先级任务 - 最先启动 */
  tid_low = rt_thread_create("task_L", thread_low_entry, RT_NULL, THREAD_STACK_SIZE, PRIO_LOW,
                             THREAD_TIMESLICE);

  /* 创建高优先级任务 */
  tid_high = rt_thread_create("task_H", thread_high_entry, RT_NULL, THREAD_STACK_SIZE, PRIO_HIGH,
                              THREAD_TIMESLICE);

  /* 创建中优先级任务 */
  tid_medium = rt_thread_create("task_M", thread_medium_entry, RT_NULL, THREAD_STACK_SIZE,
                                PRIO_MEDIUM, THREAD_TIMESLICE);

  /* 启动所有任务 */
  if (tid_low)
    rt_thread_startup(tid_low);
  if (tid_high)
    rt_thread_startup(tid_high);
  if (tid_medium)
    rt_thread_startup(tid_medium);

  return 0;
}
INIT_APP_EXPORT(priority_inversion_demo);
