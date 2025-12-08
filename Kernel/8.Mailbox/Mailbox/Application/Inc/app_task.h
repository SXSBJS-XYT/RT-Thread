#ifndef __APP_TASK_H
#define __APP_TASK_H
#include <rtthread.h>

#define THREAD_STACK_SIZE 1024
#define THREAD_TIMESLICE  5
#define MAILBOX_SIZE      4 /* Can hold 4 mails */

/* Dynamic mailbox */
static rt_mailbox_t dynamic_mb = RT_NULL;

/* Static mailbox */
static struct rt_mailbox static_mb;
static rt_uint32_t static_mb_pool[MAILBOX_SIZE]; /* Mail storage pool */

/* Thread handles */
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;

/* Message structure for Demo 5 */
struct sensor_data
{
  rt_uint32_t id;
  rt_uint32_t temperature;
  rt_uint32_t humidity;
  rt_tick_t timestamp;
};

/* Pre-allocated message buffers */
static struct sensor_data sensor_msg[4];

#endif
