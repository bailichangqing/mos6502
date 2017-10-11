#ifndef __MOS6502_H__
#define __MOS6502_H__

#include <stdint.h>

typedef struct {
	union {
		uint8_t val;
		struct {
			uint8_t c : 1; // carry flag
			uint8_t z : 1; // zero flag
			uint8_t i : 1; // interrupt disable
			uint8_t d : 1; // decimal mode
			uint8_t b : 1; // break command
			uint8_t u : 1; // unused
			uint8_t v : 1; // overflow flag
			uint8_t n : 1; // negative flag
		} __attribute__((packed));
	} __attribute__((packed));
} __attribute__((packed)) stat_reg_t;


typedef enum {
	INTR_NONE = 0,
	INTR_IRQ,
	INTR_NMI,
} intr_t;

typedef enum {
	MODE_NONE = 0,
	MODE_ABS,
	MODE_ABSX,
	MODE_ABSY,
	MODE_ACC,
	MODE_IMM,
	MODE_IMPL,
	MODE_IDXIND,
	MODE_IND,
	MODE_INDIDX,
	MODE_REL,
	MODE_ZEROP,
	MODE_ZEROPX,
	MODE_ZEROPY,
} addr_mode_t;

typedef struct mos6502 {

	struct system * sys;

	/* regs */
	uint16_t pc;  // program counter
	uint8_t sp;   // stack pointer
	uint8_t  a;   // accumulator 
	uint8_t  x;   // GPR 1
	uint8_t  y;   // GPR 2
	stat_reg_t p; // processor status word

	intr_t intr_status; 

	uint64_t cycle_counter;

} mos6502_t;

typedef struct decoder_info {
	uint8_t opcode;
	uint8_t page_crossed;
	uint8_t instr_len;
	uint16_t addr;
	addr_mode_t mode;
	mos6502_t * cpu;
} decode_info_t;


mos6502_t * mos6502_init(system_t * sys);
int mos6502_step(mos6502_t * cpu);
void mos6502_reset(mos6502_t * cpu);
void mos6502_dumpregs(mos6502_t * cpu);
void mos6502_print_instr(mos6502_t * cpu);
void mos6502_raise_irq(mos6502_t * cpu);
void mos6502_raise_nmi(mos6502_t * cpu);

uint16_t read16(mos6502_t * cpu, uint16_t addr);
uint8_t  pull(mos6502_t * cpu);
void     push(mos6502_t * cpu, uint8_t val);

#endif /* !__MOS6502_H__! */
