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

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#include <serial_v2.h>
#include <errno.h>

rt_device_t dev_uart1;
struct serial_configure uart1_configs = RT_SERIAL_CONFIG_DEFAULT;

int main(void)
{
    rt_err_t ret = 0;
    dev_uart1 = rt_device_find("uart1");
    if(dev_uart1 == NULL){
        LOG_E("rt_device_find[uart1] failed\n");
        return ENXIO;
    }

    ret = rt_device_open(dev_uart1, RT_DEVICE_FLAG_INT_RX | RT_DEVICE_OFLAG_RDWR);
    if(ret != 0){
        LOG_E("rt_device_open[uart1] failed\n");
        return ret;
    }

    rt_device_control(dev_uart1, RT_DEVICE_CTRL_CONFIG, (void *)&uart1_configs);

    rt_device_write(dev_uart1, 0, "Uart1 Init\n", rt_strlen("Uart1 Init\n"));

    return RT_EOK;
}
