#ifndef		__KEY_H
#define		__KEY_H

#include 	"stdint.h"

#define		KEY_PRESS			1
#define		KEY_LONGPRESS		2
#define		KEY_DOUBLE			3
#define     KEY_BUFF_LEN		4

int	key_triger(void* ctx);
void key_scan(void);
uint8_t key_get(void);

#endif
