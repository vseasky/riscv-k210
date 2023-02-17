#include "board_config.h"

#if (BOARD_VERSION == BOARD_V1_3)
#include "key.h"
#include "gpiohs.h"

typedef struct 
{
	uint32_t	interval;
    uint32_t    read;
    uint32_t    write;
    uint8_t     buff[KEY_BUFF_LEN];
} keyp_t;

keyp_t key = 
{
	.read = 0,
	.write = 0,
	.interval = 0
};

int	key_trigger(void* ctx)
{
	if (gpiohs_get_pin(KEY_IO) != 0) 
	{
		return -1;
	}
	key.interval = 5;

	return 0;
}

void key_scan(void)
{
	if (key.interval == 0) 
	{
		return;
	}
	key.interval--;
	if (key.interval != 0) 
	{
		return;
	}

	if (gpiohs_get_pin(KEY_IO) != 0) 
	{
		return;
	}
	
	key.buff[key.write++] = KEY_PRESS;
	if (key.write >= KEY_BUFF_LEN) 
	{
		key.write = 0;
	}
}

uint8_t	key_get(void)
{
	uint8_t val;
	
	if (key.write == key.read) 
	{
		return 0;
	}
	
	val = key.buff[key.read++];
	if (key.read >= KEY_BUFF_LEN) 
	{
		key.read = 0;
	}

	return val;
}

#endif // (BOARD_VERSION == BOARD_V1_3)

