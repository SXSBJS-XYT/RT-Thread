#include <rtthread.h>
#include "app_task.h"
#include <string.h>

// #define DEMO1
// #define DEMO2
// #define DEMO3
// #define DEMO4
// #define DEMO5
// #define DEMO6
#define DEMO7

/* Dynamic message queue */
static rt_mq_t dynamic_mq = RT_NULL;

/* Static message queue */
static struct rt_messagequeue static_mq;
/* Pool size = msg_size * max_msgs (simplified, actual needs alignment) */
static rt_uint8_t static_mq_pool[MQ_MSG_SIZE * MQ_MAX_MSGS];

/* Thread handles */
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;

/* Simple command message */
struct cmd_msg
{
  rt_uint8_t cmd_id;
  rt_uint8_t param;
};

/* Sensor data message */
struct sensor_msg
{
  rt_uint32_t sensor_id;
  rt_int32_t temperature; /* x100, e.g., 2550 = 25.50C */
  rt_int32_t humidity;    /* x100 */
  rt_tick_t timestamp;
};

/* Log message (variable length) */
struct log_msg
{
  rt_uint8_t level; /* 0=DEBUG, 1=INFO, 2=WARN, 3=ERROR */
  char text[24];    /* Log text */
};

/* Generic message with type tag */
#define MSG_TYPE_CMD    1
#define MSG_TYPE_SENSOR 2
#define MSG_TYPE_LOG    3

struct generic_msg
{
  rt_uint8_t type;
  union
  {
    struct cmd_msg cmd;
    struct sensor_msg sensor;
    struct log_msg log;
  } data;
};

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
  char send_buf[32];
  char recv_buf[32];
  rt_size_t recv_len;

  print_section("Demo 1: Basic Send and Receive");

  rt_kprintf("\n  Message Queue COPIES data (unlike Mailbox which passes pointers)\n\n");

  dynamic_mq = rt_mq_create("basic_mq", MQ_MSG_SIZE, MQ_MAX_MSGS, RT_IPC_FLAG_PRIO);
  RT_ASSERT(dynamic_mq != RT_NULL);

  /* ----- Send strings ----- */
  rt_kprintf("[1.1] Sending string messages\n\n");

  /* Send message 1 */
  rt_strcpy(send_buf, "Hello RT-Thread!");
  rt_kprintf("  > Send: \"%s\" (%d bytes)\n", send_buf, rt_strlen(send_buf) + 1);
  rt_mq_send(dynamic_mq, send_buf, rt_strlen(send_buf) + 1);

  /* Send message 2 */
  rt_strcpy(send_buf, "Message Queue Demo");
  rt_kprintf("  > Send: \"%s\" (%d bytes)\n", send_buf, rt_strlen(send_buf) + 1);
  rt_mq_send(dynamic_mq, send_buf, rt_strlen(send_buf) + 1);

  rt_kprintf("  + Queue entries: %d\n\n", dynamic_mq->entry);

  /* ----- Receive ----- */
  rt_kprintf("[1.2] Receiving messages (FIFO order)\n\n");

  while (dynamic_mq->entry > 0)
  {
    rt_memset(recv_buf, 0, sizeof(recv_buf));
    ret = rt_mq_recv(dynamic_mq, recv_buf, sizeof(recv_buf), RT_WAITING_NO);
    if (ret == RT_EOK)
    {
      rt_kprintf("  < Recv: \"%s\"\n", recv_buf);
    }
  }

  rt_kprintf("  + Queue entries: %d\n", dynamic_mq->entry);

  rt_mq_delete(dynamic_mq);
  dynamic_mq = RT_NULL;
}
#endif

#ifdef DEMO2
/* Demo 2: Variable-Size Messages */
static void demo_variable_size(void)
{
  char recv_buf[32];

  print_section("Demo 2: Variable-Size Messages");

  rt_kprintf("\n  Each message can have DIFFERENT size (up to max_msg_size)\n\n");

  dynamic_mq = rt_mq_create("var_mq", MQ_MSG_SIZE, MQ_MAX_MSGS, RT_IPC_FLAG_PRIO);
  RT_ASSERT(dynamic_mq != RT_NULL);

  /* Send different size messages */
  struct
  {
    const char *data;
    rt_size_t len;
  } msgs[] = {{"Hi", 3}, {"Hello", 6}, {"Hello World", 12}, {"This is a longer message!", 26}};

  rt_kprintf("  Sending messages of different sizes:\n\n");
  for (int i = 0; i < 4; i++)
  {
    rt_mq_send(dynamic_mq, (void *)msgs[i].data, msgs[i].len);
    rt_kprintf("  > Send %2d bytes: \"%s\"\n", msgs[i].len, msgs[i].data);
  }

  rt_kprintf("\n  Receiving:\n\n");
  while (dynamic_mq->entry > 0)
  {
    rt_memset(recv_buf, 0, sizeof(recv_buf));
    rt_mq_recv(dynamic_mq, recv_buf, sizeof(recv_buf), RT_WAITING_NO);
    rt_kprintf("  < Recv: \"%s\" (strlen=%d)\n", recv_buf, rt_strlen(recv_buf));
  }

  rt_mq_delete(dynamic_mq);
  dynamic_mq = RT_NULL;
}
#endif

#ifdef DEMO3
/* Demo 3: Struct Messages */
static void demo_struct_messages(void)
{
  struct sensor_msg send_msg, recv_msg;

  print_section("Demo 3: Struct Messages");

  rt_kprintf("\n  Unlike Mailbox, Message Queue COPIES the entire struct.\n");
  rt_kprintf("  Sender and receiver have independent copies.\n\n");

  dynamic_mq = rt_mq_create("struct_mq", sizeof(struct sensor_msg), 4, RT_IPC_FLAG_PRIO);
  RT_ASSERT(dynamic_mq != RT_NULL);

  rt_kprintf("  struct sensor_msg {\n");
  rt_kprintf("      uint32_t sensor_id;\n");
  rt_kprintf("      int32_t  temperature;\n");
  rt_kprintf("      int32_t  humidity;\n");
  rt_kprintf("      rt_tick_t timestamp;\n");
  rt_kprintf("  };  // %d bytes\n\n", sizeof(struct sensor_msg));

  /* Send struct messages */
  for (int i = 0; i < 3; i++)
  {
    send_msg.sensor_id   = i + 1;
    send_msg.temperature = 2500 + i * 100; /* 25.00, 26.00, 27.00 */
    send_msg.humidity    = 5000 + i * 500; /* 50.00, 55.00, 60.00 */
    send_msg.timestamp   = rt_tick_get();

    rt_mq_send(dynamic_mq, &send_msg, sizeof(send_msg));
    rt_kprintf("  > Send sensor[%d]: T=%d.%02dC, H=%d.%02d%%\n", send_msg.sensor_id,
               send_msg.temperature / 100, send_msg.temperature % 100, send_msg.humidity / 100,
               send_msg.humidity % 100);
  }

  rt_kprintf("\n");

  /* Receive struct messages */
  while (dynamic_mq->entry > 0)
  {
    rt_mq_recv(dynamic_mq, &recv_msg, sizeof(recv_msg), RT_WAITING_NO);
    rt_kprintf("  < Recv sensor[%d]: T=%d.%02dC, H=%d.%02d%%, tick=%d\n", recv_msg.sensor_id,
               recv_msg.temperature / 100, recv_msg.temperature % 100, recv_msg.humidity / 100,
               recv_msg.humidity % 100, recv_msg.timestamp);
  }

  rt_kprintf("\n  Advantage: No lifetime issues (data is copied)\n");
  rt_kprintf("  Tradeoff: Memory copy overhead for large messages\n");

  rt_mq_delete(dynamic_mq);
  dynamic_mq = RT_NULL;
}
#endif

#ifdef DEMO4
/* Demo 4: Urgent Messages */
static void demo_urgent_messages(void)
{
  struct cmd_msg cmd, recv_cmd;

  print_section("Demo 4: Urgent Messages (rt_mq_urgent)");

  rt_kprintf("\n  rt_mq_urgent() inserts message at FRONT of queue.\n\n");

  dynamic_mq = rt_mq_create("urg_mq", sizeof(struct cmd_msg), 8, RT_IPC_FLAG_PRIO);
  RT_ASSERT(dynamic_mq != RT_NULL);

  /* Send normal messages */
  rt_kprintf("  > Send normal: CMD_1, CMD_2, CMD_3\n");
  cmd.cmd_id = 1;
  cmd.param  = 10;
  rt_mq_send(dynamic_mq, &cmd, sizeof(cmd));
  cmd.cmd_id = 2;
  cmd.param  = 20;
  rt_mq_send(dynamic_mq, &cmd, sizeof(cmd));
  cmd.cmd_id = 3;
  cmd.param  = 30;
  rt_mq_send(dynamic_mq, &cmd, sizeof(cmd));

  /* Send urgent message */
  rt_kprintf("  > Send URGENT: CMD_99 (emergency!)\n\n");
  cmd.cmd_id = 99;
  cmd.param  = 0xFF;
  rt_mq_urgent(dynamic_mq, &cmd, sizeof(cmd));

  /* Receive - urgent should come first */
  rt_kprintf("  Receiving order:\n");
  int order = 1;
  while (dynamic_mq->entry > 0)
  {
    rt_mq_recv(dynamic_mq, &recv_cmd, sizeof(recv_cmd), RT_WAITING_NO);
    if (recv_cmd.cmd_id == 99)
    {
      rt_kprintf("    #%d: CMD_%d (param=0x%02X) <-- URGENT first!\n", order++, recv_cmd.cmd_id,
                 recv_cmd.param);
    }
    else
    {
      rt_kprintf("    #%d: CMD_%d (param=%d)\n", order++, recv_cmd.cmd_id, recv_cmd.param);
    }
  }

  rt_kprintf("\n  Use case: Emergency stop, high-priority commands\n");

  rt_mq_delete(dynamic_mq);
  dynamic_mq = RT_NULL;
}
#endif

#ifdef DEMO5
/* Demo 5: Timeout Handling */
static void thread_delayed_sender_mq(void *param)
{
  struct cmd_msg cmd = {.cmd_id = 42, .param = 100};

  rt_thread_mdelay(200);
  rt_kprintf("  [Sender] Sending message after 200ms\n");
  rt_mq_send(dynamic_mq, &cmd, sizeof(cmd));
}

static void demo_timeout_handling(void)
{
  rt_err_t ret;
  struct cmd_msg recv_cmd;

  print_section("Demo 5: Timeout Handling");

  dynamic_mq = rt_mq_create("to_mq", sizeof(struct cmd_msg), 4, RT_IPC_FLAG_PRIO);
  RT_ASSERT(dynamic_mq != RT_NULL);

  /* Start delayed sender */
  tid1 = rt_thread_create("mq_send", thread_delayed_sender_mq, RT_NULL, THREAD_STACK_SIZE, 20,
                          THREAD_TIMESLICE);
  rt_thread_startup(tid1);

  /* ----- Non-blocking ----- */
  rt_kprintf("\n[5.1] Non-blocking: RT_WAITING_NO\n\n");

  ret = rt_mq_recv(dynamic_mq, &recv_cmd, sizeof(recv_cmd), RT_WAITING_NO);
  if (ret == -RT_ETIMEOUT)
  {
    rt_kprintf("  > No message (returns immediately)\n");
  }

  /* ----- Short timeout ----- */
  rt_kprintf("\n[5.2] Short timeout: 100ms (message arrives at 200ms)\n\n");

  rt_kprintf("  > Waiting...\n");
  ret = rt_mq_recv(dynamic_mq, &recv_cmd, sizeof(recv_cmd), rt_tick_from_millisecond(100));
  if (ret == -RT_ETIMEOUT)
  {
    rt_kprintf("  - Timeout! Message not yet sent.\n");
  }

  /* ----- Long timeout ----- */
  rt_kprintf("\n[5.3] Long timeout: 500ms\n\n");

  rt_kprintf("  > Waiting...\n");
  ret = rt_mq_recv(dynamic_mq, &recv_cmd, sizeof(recv_cmd), rt_tick_from_millisecond(500));
  if (ret == RT_EOK)
  {
    rt_kprintf("  + Received: CMD_%d, param=%d\n", recv_cmd.cmd_id, recv_cmd.param);
  }

  rt_thread_mdelay(50);

  rt_mq_delete(dynamic_mq);
  dynamic_mq = RT_NULL;
}
#endif

#ifdef DEMO6
/* Demo 6: Queue Full Handling */
static void demo_queue_full(void)
{
  rt_err_t ret;
  struct cmd_msg cmd = {.cmd_id = 1, .param = 0};

  print_section("Demo 6: Queue Full Handling");

  rt_kprintf("\n  When queue is full:\n");
  rt_kprintf("  - rt_mq_send(): returns -RT_EFULL immediately\n");
  rt_kprintf("  - rt_mq_send_wait(): blocks until space or timeout\n\n");

  /* Small queue: only 3 messages */
  dynamic_mq = rt_mq_create("full_mq", sizeof(struct cmd_msg), 3, RT_IPC_FLAG_PRIO);
  RT_ASSERT(dynamic_mq != RT_NULL);

  rt_kprintf("  Queue capacity: 3 messages\n\n");

  /* Fill it */
  for (int i = 1; i <= 3; i++)
  {
    cmd.cmd_id = i;
    ret        = rt_mq_send(dynamic_mq, &cmd, sizeof(cmd));
    rt_kprintf("  > Send CMD_%d: %s (entries: %d)\n", i, ret == RT_EOK ? "OK" : "FAIL",
               dynamic_mq->entry);
  }

  /* Try to send when full */
  rt_kprintf("\n  > Send CMD_4 (queue full): ");
  cmd.cmd_id = 4;
  ret        = rt_mq_send(dynamic_mq, &cmd, sizeof(cmd));
  if (ret == -RT_EFULL)
  {
    rt_kprintf("-RT_EFULL\n");
  }

  /* Try with wait */
  rt_kprintf("  > Send CMD_4 with 100ms timeout: ");
  ret = rt_mq_send_wait(dynamic_mq, &cmd, sizeof(cmd), rt_tick_from_millisecond(100));
  if (ret == -RT_ETIMEOUT)
  {
    rt_kprintf("-RT_ETIMEOUT\n");
  }

  rt_kprintf("\n  Solutions:\n");
  rt_kprintf("  1. Increase queue size\n");
  rt_kprintf("  2. Handle -RT_EFULL (drop or retry)\n");
  rt_kprintf("  3. Use rt_mq_send_wait() with appropriate timeout\n");

  rt_mq_delete(dynamic_mq);
  dynamic_mq = RT_NULL;
}
#endif

#ifdef DEMO7
/* Demo 7: Multi-Type Messages */
static void thread_msg_producer(void *param)
{
  struct generic_msg msg;

  /* Send command */
  msg.type            = MSG_TYPE_CMD;
  msg.data.cmd.cmd_id = 1;
  msg.data.cmd.param  = 100;
  rt_mq_send(dynamic_mq, &msg, sizeof(msg));
  rt_kprintf("  [Producer] Sent CMD: id=%d\n", msg.data.cmd.cmd_id);

  rt_thread_mdelay(50);

  /* Send sensor data */
  msg.type                    = MSG_TYPE_SENSOR;
  msg.data.sensor.sensor_id   = 1;
  msg.data.sensor.temperature = 2550;
  msg.data.sensor.humidity    = 6500;
  msg.data.sensor.timestamp   = rt_tick_get();
  rt_mq_send(dynamic_mq, &msg, sizeof(msg));
  rt_kprintf("  [Producer] Sent SENSOR: T=%d.%02d\n", msg.data.sensor.temperature / 100,
             msg.data.sensor.temperature % 100);

  rt_thread_mdelay(50);

  /* Send log */
  msg.type           = MSG_TYPE_LOG;
  msg.data.log.level = 1; /* INFO */
  rt_strncpy(msg.data.log.text, "System running OK", sizeof(msg.data.log.text) - 1);
  rt_mq_send(dynamic_mq, &msg, sizeof(msg));
  rt_kprintf("  [Producer] Sent LOG: \"%s\"\n", msg.data.log.text);

  rt_thread_mdelay(50);

  /* Send another command */
  msg.type            = MSG_TYPE_CMD;
  msg.data.cmd.cmd_id = 2;
  msg.data.cmd.param  = 200;
  rt_mq_send(dynamic_mq, &msg, sizeof(msg));
  rt_kprintf("  [Producer] Sent CMD: id=%d\n", msg.data.cmd.cmd_id);
}

static void thread_msg_consumer(void *param)
{
  struct generic_msg msg;
  const char *level_str[] = {"DEBUG", "INFO", "WARN", "ERROR"};
  int count               = 0;

  while (count < 4)
  {
    if (rt_mq_recv(dynamic_mq, &msg, sizeof(msg), rt_tick_from_millisecond(500)) == RT_EOK)
    {

      switch (msg.type)
      {
        case MSG_TYPE_CMD:
          rt_kprintf("  [Consumer] CMD: id=%d, param=%d\n", msg.data.cmd.cmd_id,
                     msg.data.cmd.param);
          break;

        case MSG_TYPE_SENSOR:
          rt_kprintf("  [Consumer] SENSOR[%d]: T=%d.%02dC, H=%d.%02d%%\n",
                     msg.data.sensor.sensor_id, msg.data.sensor.temperature / 100,
                     msg.data.sensor.temperature % 100, msg.data.sensor.humidity / 100,
                     msg.data.sensor.humidity % 100);
          break;

        case MSG_TYPE_LOG:
          rt_kprintf("  [Consumer] LOG[%s]: %s\n", level_str[msg.data.log.level],
                     msg.data.log.text);
          break;

        default:
          rt_kprintf("  [Consumer] Unknown type: %d\n", msg.type);
          break;
      }
      count++;
    }
  }
  rt_kprintf("  [Consumer] Done\n");
}

static void demo_multi_type_messages(void)
{
  print_section("Demo 7: Multi-Type Messages (Tagged Union)");

  rt_kprintf("\n  Use type tag + union for different message types:\n\n");
  rt_kprintf("  struct generic_msg {\n");
  rt_kprintf("      uint8_t type;        // MSG_TYPE_CMD, SENSOR, LOG\n");
  rt_kprintf("      union {\n");
  rt_kprintf("          struct cmd_msg    cmd;\n");
  rt_kprintf("          struct sensor_msg sensor;\n");
  rt_kprintf("          struct log_msg    log;\n");
  rt_kprintf("      } data;\n");
  rt_kprintf("  };\n\n");

  dynamic_mq = rt_mq_create("multi_mq", sizeof(struct generic_msg), 8, RT_IPC_FLAG_PRIO);
  RT_ASSERT(dynamic_mq != RT_NULL);

  tid1 = rt_thread_create("producer", thread_msg_producer, RT_NULL, THREAD_STACK_SIZE, 20,
                          THREAD_TIMESLICE);
  tid2 = rt_thread_create("consumer", thread_msg_consumer, RT_NULL, THREAD_STACK_SIZE, 21,
                          THREAD_TIMESLICE);

  rt_thread_startup(tid1);
  rt_thread_startup(tid2);

  rt_thread_mdelay(800);

  rt_mq_delete(dynamic_mq);
  dynamic_mq = RT_NULL;
}
#endif

static void mq_demo_entry(void *param)
{
#ifdef DEMO1
  demo_basic_send_recv();
  rt_thread_mdelay(200);
#endif
#ifdef DEMO2
  demo_variable_size();
  rt_thread_mdelay(200);
#endif
#ifdef DEMO3
  demo_struct_messages();
  rt_thread_mdelay(200);
#endif
#ifdef DEMO4
  demo_urgent_messages();
  rt_thread_mdelay(200);
#endif
#ifdef DEMO5
  demo_timeout_handling();
  rt_thread_mdelay(200);
#endif
#ifdef DEMO6
  demo_queue_full();
  rt_thread_mdelay(200);
#endif
#ifdef DEMO7
  demo_multi_type_messages();
#endif
}

int mq_demo(void)
{
  rt_thread_t tid;

  tid = rt_thread_create("mq_demo", mq_demo_entry, RT_NULL, 2048, 15, 10);

  if (tid != RT_NULL)
  {
    rt_thread_startup(tid);
    return RT_EOK;
  }

  return -RT_ERROR;
}
INIT_APP_EXPORT(mq_demo);
