#ifndef __CARTROM_H__
#define __CARTROM_H__

#include <sys/types.h>
#include <stdint.h>

#define CARTROM_SIZE 0x4000

#define CARTROM_START 0xa000
#define CARTROM_OFFSET(x) ((x) - CARTROM_START)

typedef struct cartrom {
	struct system * sys;
} cartrom_t;

int cartrom_load(const char * path);

uint8_t cartrom_read(uint16_t addr);


#endif /* !__CARTROM_H__! */
