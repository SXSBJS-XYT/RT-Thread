#include <rtthread.h>
#include "app_task.h"
#include <string.h>

#define DEMO1
// #define DEMO2
// #define DEMO3
// #define DEMO4

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

/* Print memory block info */
static void print_mem_info(const char *name, void *ptr, rt_size_t size)
{
    if (ptr != RT_NULL) {
        rt_kprintf("  %-12s: addr=0x%08x, size=%d bytes\n", 
                   name, (rt_uint32_t)ptr, size);
    } else {
        rt_kprintf("  %-12s: ALLOCATION FAILED!\n", name);
    }
}

/* Print current heap status */
static void print_heap_status(const char *label)
{
#ifdef RT_USING_MEMHEAP_AS_HEAP
    rt_kprintf("  [%s] (memheap mode - stats not available)\n", label);
#else
    rt_uint32_t total, used, max_used;
    rt_memory_info(&total, &used, &max_used);
    rt_kprintf("  [%s] Total: %d, Used: %d, MaxUsed: %d, Free: %d\n",
               label, total, used, max_used, total - used);
#endif
}

#ifdef DEMO1
/* Demo 1: Basic malloc/free */
static void demo1_basic_malloc(void)
{
    print_section("Demo 1: Basic malloc/free");
    
    rt_kprintf("\n  rt_malloc(size) - Allocate memory from heap\n");
    rt_kprintf("  rt_free(ptr)    - Return memory to heap\n\n");
    
    print_heap_status("Before");
    
    /* Allocate different sizes */
    rt_kprintf("\n  Allocating memory blocks...\n");
    
    void *ptr1 = rt_malloc(32);
    print_mem_info("32 bytes", ptr1, 32);
    
    void *ptr2 = rt_malloc(64);
    print_mem_info("64 bytes", ptr2, 64);
    
    void *ptr3 = rt_malloc(128);
    print_mem_info("128 bytes", ptr3, 128);
    
    print_heap_status("After alloc");
    
    /* Use the memory */
    if (ptr1) {
        rt_memset(ptr1, 0xAA, 32);
        rt_kprintf("\n  Filled ptr1 with 0xAA\n");
    }
    
    /* Free memory */
    rt_kprintf("\n  Freeing all memory blocks...\n");
    rt_free(ptr1);
    rt_free(ptr2);
    rt_free(ptr3);
    
    print_heap_status("After free");
    
    rt_kprintf("\n  NOTE: rt_free(RT_NULL) is safe - does nothing.\n");
    rt_free(RT_NULL);  /* Safe! */
}
#endif

#ifdef DEMO2
/* Demo 2: calloc - Allocate and Zero */
static void demo2_calloc(void)
{
    print_section("Demo 2: calloc - Allocate and Zero-Initialize");
    rt_kprintf("\n  Allocate structure array\n");
    
    struct sensor_data {
        int id;
        int value;
        int count;
    };
    
    struct sensor_data *sensors = rt_calloc(5, sizeof(struct sensor_data));
    if (sensors) {
        rt_kprintf("  Allocated 5 sensor_data structs (all fields = 0)\n");
        rt_kprintf("  sensors[0].id=%d, value=%d, count=%d\n",
                   sensors[0].id, sensors[0].value, sensors[0].count);
        rt_free(sensors);
    }
}
#endif

#ifdef DEMO3
/* Demo 3: Memory Alignment */
static void demo3_alignment(void)
{
    print_section("Demo 3: Memory Alignment");
    
    rt_kprintf("\n  RT-Thread aligns allocations to RT_ALIGN_SIZE (usually 4 or 8)\n");
    rt_kprintf("  This ensures efficient CPU access.\n\n");
    
    /* Show alignment */
    rt_kprintf("  Allocating various sizes:\n");
    
    void *p1 = rt_malloc(1);
    void *p2 = rt_malloc(3);
    void *p3 = rt_malloc(5);
    void *p4 = rt_malloc(7);
    void *p5 = rt_malloc(16);
    
    rt_kprintf("    1 byte  -> addr=0x%08x (%%4=%d)\n", (rt_uint32_t)p1, (rt_uint32_t)p1 % 4);
    rt_kprintf("    3 bytes -> addr=0x%08x (%%4=%d)\n", (rt_uint32_t)p2, (rt_uint32_t)p2 % 4);
    rt_kprintf("    5 bytes -> addr=0x%08x (%%4=%d)\n", (rt_uint32_t)p3, (rt_uint32_t)p3 % 4);
    rt_kprintf("    7 bytes -> addr=0x%08x (%%4=%d)\n", (rt_uint32_t)p4, (rt_uint32_t)p4 % 4);
    rt_kprintf("   16 bytes -> addr=0x%08x (%%4=%d)\n", (rt_uint32_t)p5, (rt_uint32_t)p5 % 4);
    
    rt_kprintf("\n  All addresses are aligned (%%4 == 0)\n");
    
    rt_free(p1);
    rt_free(p2);
    rt_free(p3);
    rt_free(p4);
    rt_free(p5);
}
#endif

#ifdef DEMO4
/* Demo 4: Memory Statistics */
static void demo4_statistics(void)
{
    print_section("Demo 4: Memory Statistics");  
    rt_uint32_t total, used, max_used;
    
    rt_kprintf("\n  rt_memory_info(&total, &used, &max_used)\n\n");
    
    rt_memory_info(&total, &used, &max_used);
    
    rt_kprintf("  Current heap status:\n");
    rt_kprintf("    Total heap size : %d bytes\n", total);
    rt_kprintf("    Currently used  : %d bytes (%.1f%%)\n", 
               used, (float)used * 100 / total);
    rt_kprintf("    Peak usage      : %d bytes (%.1f%%)\n",
               max_used, (float)max_used * 100 / total);
    rt_kprintf("    Currently free  : %d bytes\n", total - used);
    
    /* Allocate and show change */
    rt_kprintf("\n  Allocating 1024 bytes...\n");
    void *test = rt_malloc(1024);
    
    rt_memory_info(&total, &used, &max_used);
    rt_kprintf("    Used after alloc: %d bytes\n", used);
    
    rt_free(test);
    rt_memory_info(&total, &used, &max_used);
    rt_kprintf("    Used after free : %d bytes\n", used);
    rt_kprintf("    Peak still shows: %d bytes\n", max_used);
}
#endif

/*===========================================================================*/
/* Main Demo Entry                                                           */
/*===========================================================================*/
static void mem_demo_entry(void *param)
{
    rt_kprintf("\n");
    rt_kprintf("============================================================\n");
    rt_kprintf("       RT-Thread Small Memory Management Tutorial\n");
    rt_kprintf("============================================================\n");
    rt_kprintf("\n");
    rt_kprintf("  Dynamic memory allocation from heap.\n");
    rt_kprintf("  Enable RT_USING_HEAP in rtconfig.h.\n");
#ifdef DEMO1    
    demo1_basic_malloc();
    rt_thread_mdelay(300);
#endif
#ifdef DEMO2 	
    demo2_calloc();
    rt_thread_mdelay(300);
#endif 
#ifdef DEMO3	
    demo3_alignment();
    rt_thread_mdelay(300);
#endif
#ifdef DEMO4    
    demo4_statistics();
    rt_thread_mdelay(300);
#endif    
}

int mem_demo(void)
{
    rt_thread_t tid;
    
    tid = rt_thread_create("mem_demo", mem_demo_entry, RT_NULL,
                           THREAD_STACK_SIZE, 15, THREAD_TIMESLICE);
    
    if (tid != RT_NULL) {
        rt_thread_startup(tid);
        return RT_EOK;
    }
    
    return -RT_ERROR;
}
INIT_APP_EXPORT(mem_demo);
