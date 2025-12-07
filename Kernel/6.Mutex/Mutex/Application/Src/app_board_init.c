#include <rtthread.h>
#include "main.h"
#include "gpio.h"
#include "usart.h"

static int board_gpio_init(void)
{
  MX_GPIO_Init();

  return 0;
}
INIT_BOARD_EXPORT(board_gpio_init);

static int board_peripheral_init(void)
{
  MX_USART1_UART_Init();

  return 0;
}
INIT_DEVICE_EXPORT(board_peripheral_init);
