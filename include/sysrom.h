#ifndef __SYSROM_H__
#define __SYSROM_H__


#include <sys/types.h>
#include <stdint.h>
#include <sys.h>

#define SYSROM_SIZE 0x2000
#define SYSROM_OFFSET(x) ((x) - SYS_ROM_START)


typedef struct sysrom {
	struct system * sys;

} sysrom_t;

sysrom_t * sysrom_init(struct system * sys);

int sysrom_load (const char * path);
uint8_t sysrom_read(uint16_t addr);

#endif /* !__SYSROM_H__! */
