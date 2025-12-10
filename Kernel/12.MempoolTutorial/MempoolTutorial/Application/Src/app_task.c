#include <rtthread.h>
#include "app_task.h"
#include <string.h>

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

/* Basic Memory Pool */
static void basic_mempool(void)
{
  print_section("Demo 1: Basic Memory Pool");

  /* Create a memory pool: 8 blocks, 32 bytes each */
  rt_mp_t mp = rt_mp_create("demo_mp", 8, 32);

  if (mp == RT_NULL)
  {
    rt_kprintf("  ERROR: Failed to create memory pool!\n");
    return;
  }

  rt_kprintf("  Created pool: 8 blocks x 32 bytes = 256 bytes total\n\n");

  /* Allocate some blocks */
  void *blk1 = rt_mp_alloc(mp, RT_WAITING_FOREVER);
  void *blk2 = rt_mp_alloc(mp, RT_WAITING_FOREVER);
  void *blk3 = rt_mp_alloc(mp, RT_WAITING_FOREVER);

  rt_kprintf("  Allocated 3 blocks:\n");
  rt_kprintf("    blk1 = 0x%08X\n", (rt_uint32_t)blk1);
  rt_kprintf("    blk2 = 0x%08X\n", (rt_uint32_t)blk2);
  rt_kprintf("    blk3 = 0x%08X\n", (rt_uint32_t)blk3);

  /* Use the blocks */
  rt_memset(blk1, 0xAA, 32);
  rt_strcpy((char *)blk2, "Hello Memory Pool!");

  rt_kprintf("\n  blk2 content: \"%s\"\n", (char *)blk2);

  /* Free blocks */
  rt_kprintf("\n  Freeing all blocks...\n");
  rt_mp_free(blk1);
  rt_mp_free(blk2);
  rt_mp_free(blk3);

  rt_kprintf("  All blocks returned to pool.\n");

  /* Cleanup */
  rt_mp_delete(mp);
  rt_kprintf("  Pool deleted.\n");
}

/* Main Demo Entry*/
static void mp_demo_entry(void *param)
{
  rt_kprintf("\n");
  rt_kprintf("============================================================\n");
  rt_kprintf("            RT-Thread Memory Pool Tutorial\n");
  rt_kprintf("============================================================\n");

  basic_mempool();
  rt_thread_mdelay(300);
}

int main()
{
  rt_thread_t tid;

  tid = rt_thread_create("mp_demo", mp_demo_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY,
                         THREAD_TIMESLICE);

  if (tid != RT_NULL)
  {
    rt_thread_startup(tid);
    return RT_EOK;
  }

  return -RT_ERROR;
}
