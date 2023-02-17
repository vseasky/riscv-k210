#ifndef		__KEY_H
#define		__KEY_H

#define		KEY_PRESS			(1)
#define		KEY_LONGPRESS		(2)
#define     KEY_BUFF_LEN		(4)

int	key_trigger(void* ctx);
void key_scan(void);
uint8_t key_get(void);

#endif
