#include <string.h>
#include <stdint.h>
#include <base.h>
#include <sys.h>
#include <memctrl.h>
#include <io.h>
#include <sysrom.h>
#include <cartrom.h>

/* 
 * The below callback functions implement the memory map
 * for our system board. 
 * These are somewhat based on the Ricoh version of the
 * 6502 built for the NES system 
 * 
 */



/* 
 *         MEMORY MAP
 *  /----------------------/  0x10000
 *  |      System ROM      |
 *  |        (8K)          |
 *  /----------------------/  0xe000
 *  |      Cartridge ROM   | 
 *  |         (16K)        | 
 *  /----------------------/  0xa000
 *  |      I/O Space       | 
 *  |         (8K)         | 
 *  /----------------------/  0x8000
 *  |        RAM           |  
 *  |        (32K)         | 
 *  /----------------------/  0x0000
 *
 *
 * Note that the zero page occupies 0x0 through 0x0100
 * and the stack occupies 0x0100 through 0x0200
 *
 */
static void
custom_write (system_t * sys, uint16_t addr, uint8_t val)
{
	if (addr < IO_START) {
		sys->ram[addr] = val;
	} else if (addr < CART_ROM_START) {
		io_write(addr, val);
	} else if (addr < SYS_ROM_START) {
		DEBUG_PRINT("Cartridge ROM is not writable! (0x%04x)\n", addr);
	} else if (addr < MEM_END) {
		DEBUG_PRINT("System ROM is not writable! (0x%04x)\n", addr);
	} else {
		ERROR_PRINT("Unhandled memory write at addr: 0x%04x\n", addr);
	}
}

static uint8_t 
custom_read (system_t * sys, uint16_t addr)
{
	if (addr < IO_START) {
		return sys->ram[addr];
	} else if (addr < CART_ROM_START) {
		return io_read(addr);
	} else if (addr < SYS_ROM_START) {
		return cartrom_read(addr);
	} else if (addr < MEM_END) {
		return sysrom_read(addr);
	} else {
		ERROR_PRINT("Unhandled memory read at addr: 0x%04x\n", addr);
	}

	return 0;
}



memctrl_t * 
memctrl_init (system_t * sys)
{
	memctrl_t * m = NULL;

	INFO_PRINT("Initializing memory controller...\n");

	m = malloc(sizeof(memctrl_t));

	if (!m) {
		ERROR_PRINT("Could not allocate memory controller\n");
		return NULL;
	}

	memset(m, 0, sizeof(memctrl_t));

	m->sys   = sys;
	m->read  = custom_read;
	m->write = custom_write;

	return m;
}

uint8_t 
mem_read (system_t * sys, uint16_t addr)
{
	return sys->memctrl->read(sys, addr);
}

void
mem_write (system_t * sys, uint16_t addr, uint8_t val)
{
	sys->memctrl->write(sys, addr, val);
}





