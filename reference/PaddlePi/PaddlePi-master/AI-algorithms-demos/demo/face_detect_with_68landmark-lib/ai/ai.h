#ifndef		__AI_H
#define		__AI_H

typedef	struct {
	uint8_t  *detect_kmodel_addr;
	uint8_t  *landmark68_kmodel_addr;
} ai_addr_t;

void	ai_init(ai_addr_t *ai_addr);
void	ai_run(uint8_t *display_address);

#endif

