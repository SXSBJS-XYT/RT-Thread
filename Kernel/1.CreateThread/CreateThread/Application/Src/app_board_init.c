#include <rtthread.h>
#include "main.h"
#include "gpio.h"

static int board_gpio_init(void)
{
  MX_GPIO_Init();

  return 0;
}
INIT_BOARD_EXPORT(board_gpio_init);
