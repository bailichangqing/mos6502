#ifndef __SYS_H__
#define __SYS_H__

#define RAM_SIZE 0x8000 // 32KB
#define PAGE_SIZE 0x100 // 256B
#define PAGE_SHIFT 8
#define NPAGES_RAM (RAM_SIZE>>PAGE_SHIFT)

#define IO_START       0x8000
#define CART_ROM_START 0xa000
#define SYS_ROM_START  0xe000
#define MEM_END        0x10000

#define NPAGES_IO      ((CART_ROM_START-IO_START)>>PAGE_SHIFT)

#include <stdint.h>


typedef struct system {
	struct memctrl * memctrl;
	struct mos6502 * cpu;
	uint8_t ram[RAM_SIZE];
	struct io_subsys * io;
	struct gui_state * gui;
} system_t;


system_t * sys_init(const char * rom_path,
		    const char * sysrom_path,
		    int is_interactive,
		    int argc_ext,
		    char ** argv_ext);

void sys_reset(system_t * sys);

#endif /* !__SYS_H__! */
