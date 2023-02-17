#include "board_config.h"

#if (BOARD_VERSION == BOARD_V1_3)

#include "key.h"
#include "timer.h"

static int isr_tick_task(void *ctx)
{
	key_scan();
    return 0;
}

void tick_init(size_t nanoseconds)
{
    timer_init(0);
    timer_irq_register(0, 0, 0, 1, isr_tick_task, NULL);
    timer_set_interval(0, 0, nanoseconds);
    timer_set_enable(0, 0, 1);	
}

#endif // (BOARD_VERSION == BOARD_V1_3)

