#ifndef __MEMCTRL_H__
#define __MEMCTRL_H__

#include <stdint.h>

typedef struct memctrl {

	struct system * sys; 
	uint8_t (*read)(system_t * sys, uint16_t addr);
	void (*write)(system_t * sys, uint16_t addr, uint8_t val);

} memctrl_t;

uint8_t mem_read(system_t *sys, uint16_t addr);
void mem_write(system_t *sys, uint16_t addr, uint8_t val);

memctrl_t * memctrl_init(struct system * sys);

#endif /* !__MEMCTRL_H__! */
