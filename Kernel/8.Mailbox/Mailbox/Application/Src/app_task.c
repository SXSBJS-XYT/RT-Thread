#include "app_task.h"
#include <string.h>

// #define DEMO1
// #define DEMO2
// #define DEMO3
// #define DEMO4
#define DEMO5

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

/* Helper Functions */
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

#ifdef DEMO1
/* Demo 1: Basic Send and Receive */
static void demo_basic_send_recv(void)
{
  rt_err_t ret;
  rt_ubase_t recv_value;

  print_section("Demo 1: Basic Send and Receive");

  /* Create mailbox */
  dynamic_mb = rt_mb_create("demo_mb", MAILBOX_SIZE, RT_IPC_FLAG_PRIO);
  RT_ASSERT(dynamic_mb != RT_NULL);

  rt_kprintf("\n  Mailbox can transfer 4-byte values (integers or pointers)\n\n");

  /* ----- Send integers ----- */
  rt_kprintf("[1.1] Sending integers\n\n");

  rt_kprintf("  > Sending: 100, 200, 300\n");
  rt_mb_send(dynamic_mb, 100);
  rt_mb_send(dynamic_mb, 200);
  rt_mb_send(dynamic_mb, 300);
  rt_kprintf("  + Mailbox entries: %d\n\n", dynamic_mb->entry);

  /* Receive */
  rt_kprintf("  > Receiving (FIFO order):\n");
  while (rt_mb_recv(dynamic_mb, &recv_value, RT_WAITING_NO) == RT_EOK)
  {
    rt_kprintf("    Received: %d\n", recv_value);
  }
  rt_kprintf("  + Mailbox entries: %d\n", dynamic_mb->entry);

  /* ----- Send with timeout ----- */
  rt_kprintf("\n[1.2] rt_mb_send_wait() with timeout\n\n");

  /* Fill the mailbox */
  rt_kprintf("  > Filling mailbox to capacity (%d)...\n", MAILBOX_SIZE);
  for (int i = 0; i < MAILBOX_SIZE; i++)
  {
    rt_mb_send(dynamic_mb, i + 1);
  }
  rt_kprintf("  + Mailbox is FULL (entries: %d)\n\n", dynamic_mb->entry);

  /* Try to send with timeout - will fail */
  rt_kprintf("  > Try send with 100ms timeout (mailbox full)...\n");
  ret = rt_mb_send_wait(dynamic_mb, 999, rt_tick_from_millisecond(100));
  if (ret == -RT_ETIMEOUT)
  {
    rt_kprintf("  - Timeout! Mailbox is full.\n");
  }
  else if (ret == -RT_EFULL)
  {
    rt_kprintf("  - Mailbox full (non-blocking).\n");
  }

  /* Cleanup */
  rt_mb_delete(dynamic_mb);
  dynamic_mb = RT_NULL;
}
#endif

#ifdef DEMO2
/* Demo 2: Urgent Mail */
static void demo_urgent_mail(void)
{
  rt_ubase_t recv_value;

  print_section("Demo 2: Urgent Mail (rt_mb_urgent)");

  rt_kprintf("\n  rt_mb_urgent() inserts mail at FRONT of queue,\n");
  rt_kprintf("  bypassing normal FIFO order.\n\n");

  dynamic_mb = rt_mb_create("urg_mb", MAILBOX_SIZE, RT_IPC_FLAG_PRIO);
  RT_ASSERT(dynamic_mb != RT_NULL);

  /* Send normal mails */
  rt_kprintf("  > Send normal: 1, 2, 3\n");
  rt_mb_send(dynamic_mb, 1);
  rt_mb_send(dynamic_mb, 2);
  rt_mb_send(dynamic_mb, 3);

  /* Send urgent mail - goes to front! */
  rt_kprintf("  > Send URGENT: 999\n");
  rt_mb_urgent(dynamic_mb, 999);

  /* Receive - urgent should come first */
  rt_kprintf("\n  > Receiving order:\n");
  int order = 1;
  while (rt_mb_recv(dynamic_mb, &recv_value, RT_WAITING_NO) == RT_EOK)
  {
    if (recv_value == 999)
    {
      rt_kprintf("    #%d: %d  <-- URGENT (was sent last, received first!)\n", order++, recv_value);
    }
    else
    {
      rt_kprintf("    #%d: %d\n", order++, recv_value);
    }
  }

  rt_kprintf("\n  Use case: High-priority commands, error notifications\n");

  rt_mb_delete(dynamic_mb);
  dynamic_mb = RT_NULL;
}
#endif

#ifdef DEMO3
/* Demo 3: Receive Timeout Handling */
static void thread_delayed_sender(void *param)
{
  rt_thread_mdelay(200); /* Delay before sending */
  rt_kprintf("  [Sender] Sending mail after 200ms delay\n");
  rt_mb_send(dynamic_mb, 12345);
}

static void demo_recv_timeout(void)
{
  rt_err_t ret;
  rt_ubase_t recv_value;

  print_section("Demo 3: Receive Timeout Handling");

  dynamic_mb = rt_mb_create("to_mb", MAILBOX_SIZE, RT_IPC_FLAG_PRIO);
  RT_ASSERT(dynamic_mb != RT_NULL);

  /* Start sender thread that will send after 200ms */
  tid1 = rt_thread_create("sender", thread_delayed_sender, RT_NULL, THREAD_STACK_SIZE, 20,
                          THREAD_TIMESLICE);
  rt_thread_startup(tid1);

  /* ----- Non-blocking receive ----- */
  rt_kprintf("\n[3.1] Non-blocking: RT_WAITING_NO\n\n");

  ret = rt_mb_recv(dynamic_mb, &recv_value, RT_WAITING_NO);
  if (ret == -RT_ETIMEOUT)
  {
    rt_kprintf("  > No mail available (returns immediately)\n");
  }

  /* ----- Timeout receive (too short) ----- */
  rt_kprintf("\n[3.2] Short timeout: 100ms (mail arrives at 200ms)\n\n");

  rt_kprintf("  > Waiting for mail...\n");
  ret = rt_mb_recv(dynamic_mb, &recv_value, rt_tick_from_millisecond(100));
  if (ret == -RT_ETIMEOUT)
  {
    rt_kprintf("  - Timeout after 100ms, mail not yet arrived.\n");
  }

  /* ----- Timeout receive (long enough) ----- */
  rt_kprintf("\n[3.3] Long timeout: 500ms\n\n");

  rt_kprintf("  > Waiting for mail...\n");
  ret = rt_mb_recv(dynamic_mb, &recv_value, rt_tick_from_millisecond(500));
  if (ret == RT_EOK)
  {
    rt_kprintf("  + Received: %d\n", recv_value);
  }

  /* ----- Blocking forever ----- */
  rt_kprintf("\n[3.4] RT_WAITING_FOREVER\n");
  rt_kprintf("  > Blocks until mail arrives (use with caution!)\n");

  rt_thread_mdelay(100); /* Let sender thread complete */

  rt_mb_delete(dynamic_mb);
  dynamic_mb = RT_NULL;
}
#endif

#ifdef DEMO4
/* Demo 4: Producer-Consumer Pattern */
static void thread_producer(void *param)
{
  for (int i = 1; i <= 8; i++)
  {
    rt_kprintf("  [Producer] Sending: %d\n", i * 10);

    /* Wait if mailbox full */
    rt_mb_send_wait(dynamic_mb, i * 10, RT_WAITING_FOREVER);

    rt_thread_mdelay(50); /* Produce interval */
  }
  rt_kprintf("  [Producer] Done\n");
}

static void thread_consumer(void *param)
{
  rt_ubase_t value;
  int count = 0;

  while (count < 8)
  {
    /* Wait for mail */
    if (rt_mb_recv(dynamic_mb, &value, rt_tick_from_millisecond(500)) == RT_EOK)
    {
      rt_kprintf("  [Consumer] Received: %d\n", value);
      count++;
      rt_thread_mdelay(100); /* Consume slower than produce */
    }
    else
    {
      break; /* Timeout */
    }
  }
  rt_kprintf("  [Consumer] Done\n");
}

static void demo_producer_consumer(void)
{
  print_section("Demo 4: Producer-Consumer Pattern");

  rt_kprintf("\n  Producer sends 8 messages (fast)\n");
  rt_kprintf("  Consumer receives 8 messages (slow)\n");
  rt_kprintf("  Mailbox capacity: %d (acts as buffer)\n\n", MAILBOX_SIZE);

  dynamic_mb = rt_mb_create("pc_mb", MAILBOX_SIZE, RT_IPC_FLAG_PRIO);
  RT_ASSERT(dynamic_mb != RT_NULL);

  tid1 = rt_thread_create("producer", thread_producer, RT_NULL, THREAD_STACK_SIZE, 20,
                          THREAD_TIMESLICE);
  tid2 = rt_thread_create("consumer", thread_consumer, RT_NULL, THREAD_STACK_SIZE, 20,
                          THREAD_TIMESLICE);

  rt_thread_startup(tid1);
  rt_thread_startup(tid2);

  /* Wait for demo to complete */
  rt_thread_mdelay(1500);

  rt_mb_delete(dynamic_mb);
  dynamic_mb = RT_NULL;
}
#endif

#ifdef DEMO5
/* Demo 5: Passing Pointers (Struct Data) */
static void thread_sensor_reader(void *param)
{
  for (int i = 0; i < 4; i++)
  {
    /* Fill sensor data */
    sensor_msg[i].id          = i + 1;
    sensor_msg[i].temperature = 20 + i * 5;
    sensor_msg[i].humidity    = 50 + i * 10;
    sensor_msg[i].timestamp   = rt_tick_get();

    rt_kprintf("  [Sensor] Send ptr to sensor[%d]: T=%d, H=%d\n", i, sensor_msg[i].temperature,
               sensor_msg[i].humidity);

    /* Send POINTER to the struct (not the struct itself!) */
    rt_mb_send(dynamic_mb, (rt_ubase_t)&sensor_msg[i]);

    rt_thread_mdelay(100);
  }
}

static void thread_data_processor(void *param)
{
  rt_ubase_t recv_ptr;
  struct sensor_data *data;
  int count = 0;

  while (count < 4)
  {
    if (rt_mb_recv(dynamic_mb, &recv_ptr, rt_tick_from_millisecond(500)) == RT_EOK)
    {
      /* Cast received value back to pointer */
      data = (struct sensor_data *)recv_ptr;

      rt_kprintf("  [Process] Recv sensor[%d]: T=%d, H=%d, tick=%d\n", data->id, data->temperature,
                 data->humidity, data->timestamp);
      count++;
    }
  }
}

static void demo_passing_pointers(void)
{
  print_section("Demo 5: Passing Pointers (Struct Data)");

  rt_kprintf("\n  Mailbox only transfers 4 bytes. For larger data:\n");
  rt_kprintf("  1. Allocate struct in shared memory or static buffer\n");
  rt_kprintf("  2. Send the POINTER through mailbox\n");
  rt_kprintf("  3. Receiver casts pointer back to struct type\n\n");

  rt_kprintf("  struct sensor_data {\n");
  rt_kprintf("      uint32_t id;\n");
  rt_kprintf("      uint32_t temperature;\n");
  rt_kprintf("      uint32_t humidity;\n");
  rt_kprintf("      rt_tick_t timestamp;\n");
  rt_kprintf("  };  // %d bytes (but mailbox sends 4-byte pointer)\n\n",
             sizeof(struct sensor_data));

  dynamic_mb = rt_mb_create("ptr_mb", MAILBOX_SIZE, RT_IPC_FLAG_PRIO);
  RT_ASSERT(dynamic_mb != RT_NULL);

  tid1 = rt_thread_create("sensor", thread_sensor_reader, RT_NULL, THREAD_STACK_SIZE, 20,
                          THREAD_TIMESLICE);
  tid2 = rt_thread_create("process", thread_data_processor, RT_NULL, THREAD_STACK_SIZE, 20,
                          THREAD_TIMESLICE);

  rt_thread_startup(tid1);
  rt_thread_startup(tid2);

  rt_thread_mdelay(800);

  rt_kprintf("\n  IMPORTANT: Ensure data lifetime! Don't send pointers to\n");
  rt_kprintf("  local variables that may go out of scope.\n");

  rt_mb_delete(dynamic_mb);
  dynamic_mb = RT_NULL;
}
#endif

/*===========================================================================*/
/* Main Demo Entry                                                           */
/*===========================================================================*/
static void mailbox_demo_entry(void *param)
{
  rt_kprintf("\n");
  rt_kprintf("============================================================\n");
  rt_kprintf("         RT-Thread Mailbox Tutorial\n");
  rt_kprintf("============================================================\n");
#ifdef DEMO1
  demo_basic_send_recv();
  rt_thread_mdelay(200);
#endif
#ifdef DEMO2
  demo_urgent_mail();
  rt_thread_mdelay(200);
#endif
#ifdef DEMO3
  demo_recv_timeout();
  rt_thread_mdelay(200);
#endif
#ifdef DEMO4
  demo_producer_consumer();
  rt_thread_mdelay(200);
#endif
#ifdef DEMO5
  demo_passing_pointers();
#endif
}

int mailbox_demo(void)
{
  rt_thread_t tid;

  tid = rt_thread_create("mb_demo", mailbox_demo_entry, RT_NULL, 2048, 15, 10);

  if (tid != RT_NULL)
  {
    rt_thread_startup(tid);
    return RT_EOK;
  }

  return -RT_ERROR;
}
INIT_APP_EXPORT(mailbox_demo);
