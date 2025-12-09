#include <rtthread.h>
#include "app_task.h"

static rt_thread_t tid1 = RT_NULL;

/* 线程1的信号处理函数 */
void thread1_signal_handler(int sig)
{
  rt_kprintf("thread1 received signal %d\n", sig);
}

/* 线程1的入口函数 */
static void thread1_entry(void *parameter)
{
  int cnt = 0;
  /* 安装信号 */
  rt_signal_install(SIGUSR1, thread1_signal_handler);
  rt_signal_unmask(SIGUSR1);
  // rt_signal_mask(SIGUSR1);
  /* 运行10次 */
  while (cnt < 10)
  {
    /* 线程1采用低优先级运行，一直打印计数值 */
    rt_kprintf("thread1 count : %d\n", cnt);
    cnt++;
    rt_thread_mdelay(100);
  }
}

/* 信号示例的初始化 */
int signal_sample(void)
{
  /* 创建线程1 */
  tid1 = rt_thread_create("thread1", thread1_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY,
                          THREAD_TIMESLICE);
  if (tid1 != RT_NULL)
    rt_thread_startup(tid1);
  rt_thread_mdelay(300);
  /* 发送信号 SIGUSR1 给线程1 */
  rt_thread_kill(tid1, SIGUSR1);
  return 0;
}
INIT_APP_EXPORT(signal_sample);
