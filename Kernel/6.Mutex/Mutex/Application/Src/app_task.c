#include <rtthread.h>

#define THREAD_STACK_SIZE 1024
#define THREAD_TIMESLICE  5

/* Dynamic mutex */
static rt_mutex_t dynamic_mutex = RT_NULL;

/* Thread handles */
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;

static void print_line(void)
{
  rt_kprintf("------------------------------------------------------------\n");
}

static void print_section(const char *title)
{
  rt_kprintf("\n");
  print_line();
  rt_kprintf("  %s\n", title);
  print_line();
}

/* Demo 1: 递归锁（同一线程多次获取）*/
static void demo_recursive_lock(void)
{
  print_section("Demo 1: Recursive Locking");
  rt_kprintf("\n  Mutex supports recursive locking by the SAME thread.\n");
  rt_kprintf("  Each take() increments hold count, each release() decrements.\n\n");

  dynamic_mutex = rt_mutex_create("rec_mtx", RT_IPC_FLAG_PRIO);
  RT_ASSERT(dynamic_mutex != RT_NULL);

  rt_kprintf("  Initial state - Hold: %d, Owner: none\n\n", dynamic_mutex->hold);

  /* Take 3 times */
  for (int i = 1; i <= 3; i++)
  {
    rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
    rt_kprintf("  > Take #%d - Hold count: %d\n", i, dynamic_mutex->hold);
  }

  rt_kprintf("\n");

  /* Release 3 times */
  for (int i = 1; i <= 3; i++)
  {
    rt_mutex_release(dynamic_mutex);
    rt_kprintf("  < Release #%d - Hold count: %d, Value: %d \n", i, dynamic_mutex->hold,
               dynamic_mutex->value);
  }

  rt_kprintf("\n  Note: Mutex is only truly released when hold count = 0\n");

  rt_mutex_delete(dynamic_mutex);
  dynamic_mutex = RT_NULL;
}

/* Demo 2: 超时处理 */
static void thread_holder_entry(void *param)
{
  rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
  rt_kprintf("  [Holder] Got mutex, holding for 300ms...\n");
  rt_thread_mdelay(300);
  rt_kprintf("  [Holder] Releasing mutex\n");
  rt_mutex_release(dynamic_mutex);
}

static void thread_waiter_entry(void *param)
{
  rt_err_t ret;

  rt_thread_mdelay(50); /* Let holder get mutex first */

  /* Try with 100ms timeout - will fail */
  rt_kprintf("  [Waiter] Try with 100ms timeout...\n");
  ret = rt_mutex_take(dynamic_mutex, rt_tick_from_millisecond(100));
  if (ret == -RT_ETIMEOUT)
  {
    rt_kprintf("  [Waiter] Timeout! Mutex still busy.\n");
  }

  /* Try with 500ms timeout - will succeed */
  rt_kprintf("  [Waiter] Try with 500ms timeout...\n");
  ret = rt_mutex_take(dynamic_mutex, rt_tick_from_millisecond(500));
  if (ret == RT_EOK)
  {
    rt_kprintf("  [Waiter] Got mutex!\n");
    rt_mutex_release(dynamic_mutex);
  }
}

static void demo_timeout_handling(void)
{
  print_section("Demo 2: Timeout Handling");

  rt_kprintf("\n  Demonstrating different timeout behaviors.\n\n");

  dynamic_mutex = rt_mutex_create("to_mtx", RT_IPC_FLAG_PRIO);
  RT_ASSERT(dynamic_mutex != RT_NULL);

  tid1 = rt_thread_create("holder", thread_holder_entry, RT_NULL, THREAD_STACK_SIZE, 20,
                          THREAD_TIMESLICE);
  tid2 = rt_thread_create("waiter", thread_waiter_entry, RT_NULL, THREAD_STACK_SIZE, 20,
                          THREAD_TIMESLICE);

  rt_thread_startup(tid1);
  rt_thread_startup(tid2);

  rt_thread_mdelay(800);

  rt_mutex_delete(dynamic_mutex);
  dynamic_mutex = RT_NULL;
}

static void mutex_demo_entry(void *param)
{
  demo_recursive_lock();
  demo_timeout_handling();
}

int mutex_demo(void)
{
  rt_thread_t tid;

  tid = rt_thread_create("mtx_demo", mutex_demo_entry, RT_NULL, 2048, 15, 10);

  if (tid != RT_NULL)
  {
    rt_thread_startup(tid);
    return RT_EOK;
  }

  return -RT_ERROR;
}
INIT_APP_EXPORT(mutex_demo);
