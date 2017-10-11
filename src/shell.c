#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <base.h>
#include <sys.h>
#include <memctrl.h>
#include <shell.h>

#include <mos6502/cpu.h>


// break point level 2
static uint8_t* bptl2[256];

#define BP_L2_IDX(x) (((x) >> 8) & 0xFF)
#define BP_L1_IDX(x) (((x) & 0xFF))
#define BP_PRESENT(x) ((x) & 0x1)
#define BP_SET_PRESENT(x) ((x) | 0x1)
#define BP_SET_NOT_PRESENT(x) ((x) & 0xFE)

static int 
insert_bp (uint16_t bp_addr)
{
	uint8_t bpt_idx = BP_L2_IDX(bp_addr);
	uint8_t * bpt = NULL;
	uint8_t bpte;

	// no bpt yet
	if (!bptl2[bpt_idx]) {
		// allocate new bpt
		bptl2[bpt_idx] = malloc(256);
		memset(bptl2[bpt_idx], 0, 256);
	}

	bpt = bptl2[bpt_idx];
	bpte = bpt[BP_L1_IDX(bp_addr)];

	if (BP_PRESENT(bpte)) {
		printf("Breakpoint [0x%04X] exists.\n", bp_addr);
		return -1;
	} else {
		bpt[BP_L1_IDX(bp_addr)] = BP_SET_PRESENT(bpte);
	}

	return 0;
}

static void
bp_list ()
{
	int i, j;
	int c = 0;
	printf("Breakpoint List:\n");
	for (i = 0; i < 256; i++) {
		if (bptl2[i]) {
			uint8_t * bpt = bptl2[i];
			for (j = 0; j < 256; j++) {
				uint8_t bpte = bpt[j];
				if (BP_PRESENT(bpte)) {
					printf("  %d: [0x%04X]\n", c++, (uint16_t)((i << 8) | j));
			
				}
			}
		}
	}
					
				
}

static int
is_valid_bp (uint16_t bp_addr)
{
	uint8_t bpt_idx = BP_L2_IDX(bp_addr);
	uint8_t * bpt = NULL;
	uint8_t bpte;
	
	if (!bptl2[bpt_idx]) {
		return 0;
	}

	bpt = bptl2[bpt_idx];
	bpte = bpt[BP_L1_IDX(bp_addr)];
	
	return BP_PRESENT(bpte);
}

static int
remove_bp (uint16_t bp_addr)
{
	uint8_t bpt_idx = BP_L2_IDX(bp_addr);
	uint8_t * bpt = NULL;
	uint8_t bpte;
	
	if (!bptl2[bpt_idx]) {
		return -1;
	}

	bpt = bptl2[bpt_idx];
	bpte = bpt[BP_L1_IDX(bp_addr)];

	if (BP_PRESENT(bpte)) {
		bpt[BP_L1_IDX(bp_addr)] = BP_SET_NOT_PRESENT(bpte);
		return 0;
	}

	return -1;
}

static int
cmd_help (mos6502_t * cpu, char * cmd)
{
	printf("List of commands:\n\n");
	printf("s or stepi -- Step one instruction\n");
	printf("sn or stepn -- Step N instructions\n");
	printf("b or break <addr> -- Set a breakpoint at <addr>\n");
	printf("b or break rm <addr> -- Remove the breakpoint at <addr>\n");
	printf("b or break list -- List all current breakpoints\n");
	printf("c or continue -- Continue until exit or breakpoint is hit\n");
	printf("j or jump <addr> -- Set PC to specified adddress (in hex) and continue\n");
	printf("r or regs -- Dump the machine registers\n");
	printf("pk or peek <addr> -- Print the contents of memory at this address\n");
	printf("po or poke <addr> <val> -- Set the contents of memory to this value at this address\n");
	printf("dm or dumpmem <addr> <n> -- Dump N bytes of memory at specified address\n");
	printf("irq -- Manually raise an IRQ\n");
	printf("nmi -- Manually raise an NMI\n");
	printf("pr -- Print the current instruction\n");
	printf("q, quit, or exit -- Quit the program\n");
	return 0;
	
}

static int
cmd_step(mos6502_t * cpu, char * cmd)
{
	return mos6502_step(cpu);
}

static int
cmd_stepn (mos6502_t * cpu, char * cmd)
{
	int n = 0;
	int i = 0;

	if (sscanf(cmd, "stepn %d", &n) == 1 ||
	    sscanf(cmd, "sn %d", &n) == 1) {
		
		for (i = 0; i < n && !is_valid_bp(cpu->pc); i++) {
			mos6502_step(cpu);
		}
		if (is_valid_bp(cpu->pc)) {
			printf("Breakpoint [0x%04X] reached.\n", cpu->pc);
			remove_bp(cpu->pc);
		}

	} else {
		printf("Invalid command format.\n");
		cmd_help(cpu, cmd);
	}
		
	return 0;
}

static int
cmd_jump (mos6502_t * cpu, char * cmd)
{
	uint16_t addr = 0;

	if (sscanf(cmd, "j %hx\n", &addr) == 1 ||
	    sscanf(cmd, "jump %hx\n", &addr) == 1) {
		cpu->pc = addr;
	} else {
		printf("Invalid command format.\n");
		cmd_help(cpu, cmd);
	}
	return 0;
}

static int
cmd_regs (mos6502_t * cpu, char * cmd)
{
	mos6502_dumpregs(cpu);
	return 0;
}

static int
cmd_peek (mos6502_t * cpu, char * cmd)
{
	uint16_t addr = 0;

	if (sscanf(cmd, "pk %hx", &addr) == 1 ||
	    sscanf(cmd, "peek %hx", &addr) == 1) {
		printf("%04X: %02X\n", addr, mem_read(cpu->sys, addr % MEM_END));
	} else {
		printf("Invalid command format.\n");
		cmd_help(cpu, cmd);
	}

	return 0;
}

static int
cmd_poke (mos6502_t * cpu, char * cmd)
{
	uint16_t addr = 0;
	uint8_t val = 0;

	if (sscanf(cmd, "po %hx %hhx", &addr, &val) == 2 ||
	    sscanf(cmd, "poke %hx %hhx", &addr, &val) == 2) {
		mem_write(cpu->sys, addr, val);
	} else {
		printf("Invalid command format.\n");
		cmd_help(cpu, cmd);
	}
	
	return 0;
}

static int
cmd_dumpmem (mos6502_t * cpu, char * cmd)
{
	uint16_t addr;
	uint16_t n;
	int i = 0;

	if (sscanf(cmd, "dm %hx %hd", &addr, &n) == 2 ||
	    sscanf(cmd, "dumpmem %hx %hd", &addr, &n) == 2) {

		if (n < 4) { 
			n = 4;
		}
	
		for (i = 0; i < n; i+=4, addr+=4) {
			printf("%04X: %02X %02X %02X %02X\n", 
				addr, 
				mem_read(cpu->sys, addr % MEM_END),
				mem_read(cpu->sys, (addr+1)% MEM_END),
				mem_read(cpu->sys, (addr+2)% MEM_END),
				mem_read(cpu->sys, (addr+3)% MEM_END));
		}
			
	} else {
		printf("Invalid command format.\n");
		cmd_help(cpu, cmd);
	}
	
	return 0;
}

static int
cmd_irq (mos6502_t * cpu, char * cmd)
{
	mos6502_raise_irq(cpu);
	return 0;
}

static int
cmd_nmi (mos6502_t * cpu, char * cmd)
{
	mos6502_raise_nmi(cpu);
	return 0;
}

static int
cmd_print_instr (mos6502_t * cpu, char * cmd)
{
	mos6502_print_instr(cpu);
	return 0;
}

static int
cmd_cont (mos6502_t * cpu, char * cmd)
{
	while (!is_valid_bp(cpu->pc)) {
		if (mos6502_step(cpu) < 0) {
			return -1;
		}
	}

	if (is_valid_bp(cpu->pc)) {
		printf("Breakpoint [0x%04X] reached.\n", cpu->pc);
		remove_bp(cpu->pc);
	}
	
	return 0;
}

static int
cmd_brk (mos6502_t * cpu, char * cmd)
{
	uint16_t addr = 0;

	if (sscanf(cmd, "break %hx", &addr) == 1 ||
	    sscanf(cmd, "b %hx", &addr) == 1) {
		if (insert_bp(addr) == 0) {
			printf("Breakpoint set at [0x%04X]\n", addr);
		}
	} else if (sscanf(cmd, "break rm %hx", &addr) == 1 ||
		   sscanf(cmd, "b rm %hx", &addr) == 1) {
		if (remove_bp(addr) == 0) {
			printf("Breakpoint [0x%04X] removed\n", addr);
		}
	} else if (strncmp(cmd, "break list", 10) == 0 ||
		   strncmp(cmd, "b list", 6) == 0) {
		bp_list();
	} else {	
		printf("Invalid command format.\n");
		cmd_help(cpu, cmd);
	}

	return 0;
}

static int
cmd_quit (mos6502_t * cpu, char * cmd)
{
	printf("Quitting. Goodbye.\n");
	exit(0);
	return 0;
}



#define CMP(x, n) (strncmp(cmd, x, n) == 0)

#define ONECHAR(x) (cmd[0] == x && cmd[1] == '\n')

static int
handle_cmd (mos6502_t * cpu, char * cmd)
{
	if (CMP("help", 4) || CMP("?", 1) || ONECHAR('h')) {
		return cmd_help(cpu, cmd);
	} else if (ONECHAR('s') || CMP("stepi", 5)) {
		return cmd_step(cpu, cmd);
	} else if (CMP("sn", 2) || CMP("stepn", 5)) {
		return cmd_stepn(cpu, cmd);
	} else if (CMP("j ", 2) || CMP("jump", 4)) {
		return cmd_jump(cpu, cmd);
	} else if (ONECHAR('r') || CMP("regs", 4)) {
		return cmd_regs(cpu, cmd);
	} else if (CMP("pk", 2) || CMP("peek", 4)) {
		return cmd_peek(cpu, cmd);
	} else if (CMP("po", 2) || CMP("poke", 4)) {
		return cmd_poke(cpu, cmd);
	} else if (CMP("dm", 2) || CMP("dumpmem", 7)) {
		return cmd_dumpmem(cpu, cmd);
	} else if (ONECHAR('q') || CMP("quit", 4) || CMP("exit", 4)) {
		return cmd_quit(cpu, cmd);
	} else if (CMP("irq", 3)) {
		return cmd_irq(cpu, cmd);
	} else if (CMP("nmi", 3)) {
		return cmd_nmi(cpu, cmd);
	} else if (CMP("pr", 2)) {
		return cmd_print_instr(cpu, cmd);
	} else if (CMP("b ", 2) || CMP("break", 5)) {
		return cmd_brk(cpu, cmd);
	} else if (ONECHAR('c') || CMP("continue", 8)) {
		return cmd_cont(cpu, cmd);
	} else {
		printf("Undefined command: \"%s\". Try \"help\".\n", strtok(cmd, "\n"));
		
	}

	return 0;
}


void
start_shell (mos6502_t * cpu) 
{
	char buf[MAX_CMD_LEN];
	char prev[MAX_CMD_LEN];
	
	memset(buf, 0, MAX_CMD_LEN);
	memset(prev, 0, MAX_CMD_LEN);

	while (1) {
		printf(PROMPT_STR);

		while (fgets(buf, MAX_CMD_LEN, stdin) != NULL) {
			if (buf[0] != '\n') {
				handle_cmd(cpu, buf);
				memcpy(prev, buf, MAX_CMD_LEN);
				memset(buf, 0, MAX_CMD_LEN);
			} else {
				if (prev[0] != 0) {
					handle_cmd(cpu, prev);
				}
			}
			printf(PROMPT_STR);
		}

	}

}


