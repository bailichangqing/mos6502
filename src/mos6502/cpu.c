#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <base.h>
#include <sys.h>
#include <memctrl.h>
#include <mos6502/cpu.h>
#include <mos6502/vmcall.h>

#define NMI_HANDLER_ADDR 0xFFFA

#define RESET_HANDLER_ADDR 0xFFFC

#define IRQ_HANDLER_ADDR 0xFFFE

#define STATUS_C 0x80
#define STATUS_Z 0x40
#define STATUS_I 0x20
#define STATUS_D 0x10
#define STATUS_B 0x08
#define STATUS_U 0x04
#define STATUS_V 0x02
#define STATUS_N 0x01


void (*operandtable[0x100])(decode_info_t* decodeinfo);
addr_mode_t addrmodetable[0x100];

/* 
 * KCH: HINTS
 * 	Some routines/structs/data you'll probably want to write:
 * 		- routines to set the CPU flags based on a value (esp n and z)
 * 		- maps (arrays) to map opcode to instruction length, instruction name,
 * 		  addressing mode, instruction latency (in cycles), and page crossing
 * 		  latency, and a map from opcode to handler function (for your execute stage)
 * 		- separate functions for your decode() and execute() stages in step().
 *
 * 	When you need to read/write a byte from memory, use mem_read and
 * 	mem_write, defined in include/memctrl.h. Notice that you'll need to
 * 	pass a system pointer to this routine.
 *
 *      Notice that I've given you a few convenience routines to start with.
 *
 * 	Make sure to use the buggy read16 when you're calculating the address
 * 	for an indirect jump. 
 *
 * 	Make sure to set the flags properly when executing instructions that
 * 	affect flags.
 *
 * 	Be careful when adjusting cycle counter when executing branches. They 
 * 	can have variable latency when there is a page crossing.
 *
 *	Use the macro-based print routines I've provided for you instead of, e.g. printf:
 *		INFO_PRINT  - print information to the console
 *		DEBUG_PRINT - print information only when compiled for debug mode (see .config file)
 *		ERROR_PRINT - print an error
 *
 *
 * 	Don't forget that we've added an instruction to the 6502 ISA for paravirtual calls.
 * 	This is why I've called it the 6502v. Details are:
 * 		mnemonic:  VMC (VM Call)
 * 		opcode:    0x80
 * 		length:    2 bytes (2nd byte is an immediate call #)
 * 		addr mode: MODE_IMM (immediate)
 * 		latency:   6 cycles
 * 		pagecross: doesn't matter
 *
 *
 *
 */


uint16_t 
read16 (mos6502_t * cpu, uint16_t addr)
{
	DEBUG_PRINT("Handling memory read at address 0x%04x\n", addr);
	uint16_t lo = (uint16_t)mem_read(cpu->sys, addr);
	uint16_t hi = (uint16_t)mem_read(cpu->sys, addr+1);
	return lo | (hi << 8);
}

// emulates a wraparound bug: low byte wraps, high byte 
// not incremented (occurs with indirect addressing, only on JMP)
uint16_t 
buggy_read16 (mos6502_t * cpu, uint16_t addr)
{
	uint16_t first = addr;
	uint16_t secnd = (addr & 0xFF00) | (uint16_t)((uint8_t)addr + 1);
	uint16_t lo = (uint16_t)mem_read(cpu->sys, first);
	uint16_t hi = (uint16_t)mem_read(cpu->sys, secnd);
	return (hi << 8) | lo;
}

void
push (mos6502_t * cpu, uint8_t val)
{
	mem_write(cpu->sys, 
		  (0x100 | cpu->sp),
		  val);
	cpu->sp--;
}

// KCH: really, this is pop
uint8_t
pull (mos6502_t * cpu)
{
	cpu->sp++;
	return mem_read(cpu->sys, (0x100 | cpu->sp));
}

static void
push16 (mos6502_t * cpu, uint16_t val)
{
	uint8_t hi = (uint8_t)(val >> 8);
	uint8_t lo = (uint8_t)(val & 0xff);
	push(cpu, hi);
	push(cpu, lo);
}

static uint16_t
pull16 (mos6502_t * cpu)
{
	uint16_t lo = (uint16_t)pull(cpu);
	uint16_t hi = (uint16_t)pull(cpu);
	return (hi << 8) | lo;
}

/* 
 * KCH: Here the CPU is created and it is set to its reset state (by calling your 
 * mos6502_reset() routine). I've only allocated the structure for you to 
 * avoid segmentation faults in other parts of the code.
 *
 */
mos6502_t *
mos6502_init (system_t * sys)
{
	mos6502_t * cpu = (mos6502_t*)malloc(sizeof(mos6502_t));
	if (!cpu) {
		ERROR_PRINT("Could not allocate CPU\n");
		return NULL;
	}
	memset(cpu, 0, sizeof(mos6502_t));
	
	cpu->sys = sys;
	
	// FILL ME IN
	// fill in operandtable
	// fill in addrmodetable
	mos6502_reset(cpu);
	
	return cpu;
}


/* 
 * KCH: put the CPU in its reset state. You can find
 * an example of what this should mean at the following URL:
 *     http://nesdev.com/NESDoc.pdf ; pp. 13
 *
 */
void
mos6502_reset (mos6502_t * cpu)
{
	// FILL ME IN
	cpu->a = 0x00;
	cpu->x = 0x00;
	cpu->y = 0x00;
    
	cpu->pc = read16(cpu,RESET_HANDLER_ADDR);
	cpu->sp = 0xFF;
	cpu->p.val = STATUS_I;
	cpu->cycle_counter = 7;	//according to NESDOC PP13

}


/*
 * KCH: fetch, decode, and execute one instruction. This routine
 * will be called by the system code to move the CPU forward. The
 * rough order of things should be:
 * 	- check for interrupts, if there are any, handle them appropriately
 * 	- reset the interrupt state
 * 	- fetch an instruction opcode
 * 	- decode the instruction (filling in a decode_info_t struct appropriately)
 * 	- increment PC based on the above
 * 	- execute the instruction, passing the decoder info to the handler
 * 	- increase the cycle counter by the instruction latency
 * 	- make sure to increase cycle counter further if there was a page crossing and
 * 	  page crossings matter for this instruction
 * 	- RETURN the total number of cycles it took to execute the instruction
 */
int
mos6502_step (mos6502_t * cpu)
{
	// FILL ME IN
	return 0;
}



/*
 *
 * KCH: Raise an IRQ. Set the interrupt status correctly to reflect this. Your
 * step routine will check the status and act on it.  This routine will be
 * called by external devices (viz. the controller when a button is presserd)
 *
 */
void 
mos6502_raise_irq (mos6502_t * cpu)
{
	// FILL ME IN
}

/*
 * KCH: Raise an NMI. Set the interrupt status correctly to reflect this.
 *
 */
void
mos6502_raise_nmi (mos6502_t * cpu)
{
	// FILL ME IN
}


/*
 * KCH: this routine should print out the registers in the machine,
 * including:
 * 	- A, X, and Y
 * 	- PS (processor status reg)
 * 	- SP (stack pointer)
 * 	- PC
 *
 * Feel free to either print out the raw PS value or to actually parse
 * each flag. It is up to you.
 *
 */
void
mos6502_dumpregs (mos6502_t * cpu)
{
	// FILL ME IN
}

/* 
 * KCH: this function should print out details about the current instruction,
 * 	including:
 * 	- the current PC
 * 	- each byte of the instruction (including the opcode)
 * 	- the instruction mnemonic (e.g. STX, TSX, JMP, RTI, etc.)
 * 	- the addressing mode
 * 	- the latency of the instruction
 */
void
mos6502_print_instr (mos6502_t * cpu)
{
	// FILL ME IN
}
