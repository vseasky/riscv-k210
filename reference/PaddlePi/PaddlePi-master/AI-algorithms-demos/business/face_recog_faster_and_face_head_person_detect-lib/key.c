#include "board_config.h"
#include "key.h"
#include "gpiohs.h"
#include "stdint.h"

//#define		DBG_KEY
#ifdef		DBG_KEY
#define		KEY_DBG			printk
#else
#define		KEY_DBG(...)		
#endif


#define 	KEY_LONGPRESS_TIME	200		// 200 * 10ms
#define		KEY_DELAY_GET		35		// 30 * 10

typedef struct {
	uint32_t	interval;
    uint32_t    read;
    uint32_t    write;
    uint8_t     buff[KEY_BUFF_LEN];
	uint32_t	status;
	uint32_t 	delay_get;
}keyp_t;

keyp_t key = {
	.read = 0,
	.write = 0,
	.interval = 0,
	.status = 0,
	.delay_get = 0
};
	
int key_triger(void* ctx)
{
	key.interval = 8;
	return 0;
}

void	key_scan(void)
{
	if(key.delay_get != 0) key.delay_get--;
	
	if(key.interval == 0) return;
	key.interval--;
	if(key.interval != 0) return;

	uint8_t keyv = KEY_PRESS;
	switch(key.status)
	{
	case	0:
		key.status = 1;
		key.interval = KEY_LONGPRESS_TIME;
		break;
	case	1:
		if(gpiohs_get_pin(KEY_IO) == 0) {
			keyv = KEY_LONGPRESS;
			KEY_DBG("key LONG hit\n");		
			key.status = 2;
		}
		else {
			KEY_DBG("key hit\n");
			key.status = 0;			
		}
		key.buff[key.write++] = keyv;
		if(key.write >= KEY_BUFF_LEN) key.write = 0;
		key.delay_get = KEY_DELAY_GET;
		break;
	case	2:
		key.status = 0;
		break;
	default:
		break;
	}
}

uint8_t	key_get(void)
{
	uint8_t val, val1;
	
	if(key.write == key.read) return 0;
	if(key.delay_get != 0) return 0;
	
	val = key.buff[key.read++];
	if(key.read >= KEY_BUFF_LEN) key.read = 0;

	if(key.write != key.read) {
		val1 = key.buff[key.read];

		if((val == KEY_PRESS) && (val1 == KEY_PRESS)) {
			key.read++;
			if(key.read >= KEY_BUFF_LEN) key.read = 0;
			val = KEY_DOUBLE;
		}
	}
	return val;
}

