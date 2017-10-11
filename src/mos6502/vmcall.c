#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <base.h>
#include <sys.h>
#include <memctrl.h>
#include <mos6502/cpu.h>
#include <mos6502/vmcall.h>
#include <gui/gui.h>

static uint16_t paravirt_argc;
static char **  paravirt_argv;

static int paravirt_gui_enabled;

static uint16_t 
get_ax (mos6502_t * cpu)
{
	return (uint16_t)cpu->a | ((uint16_t)cpu->x) << 8;
}

static void
set_ax (mos6502_t * cpu, uint16_t val)
{
	cpu->a = (uint8_t)(val & 0xff);
	cpu->x = (uint8_t)(val >> 8);
}

static uint16_t 
read_zp16 (mos6502_t * cpu, uint8_t addr)
{
	return read16(cpu, (uint16_t)addr);
}

static void 
write16 (mos6502_t * cpu, uint16_t addr, uint16_t val)
{
	mem_write(cpu->sys, addr, (uint8_t)(val & 0xff));
	mem_write(cpu->sys, addr + 1, (uint8_t)(val >> 8));
}

static uint16_t 
pop_parm (mos6502_t * cpu, uint16_t incr)
{
	uint16_t sp = read_zp16(cpu, 0x00);
	uint16_t val = read16(cpu, sp);
	write16(cpu, 0x0000, sp + incr);
	return val;
}


static int
handle_args (mos6502_t * cpu)
{
	unsigned argv = get_ax(cpu);
	unsigned sp   = read_zp16(cpu, 0x00);
	unsigned args = sp - (paravirt_argc + 1) * 2;
	int i = 0;

	INFO_PRINT("ARGS (0x%04X) (storing args at %04X)\n", argv, args);

	write16(cpu, argv, args);
	
	sp = args;
	
	while (i < paravirt_argc) {
		unsigned spi = 0;
		const char * thearg = paravirt_argv[i++];
		INFO_PRINT("working arg %s\n", thearg);
		sp -= strlen(thearg) + 1;
		do {
			mem_write(cpu->sys, sp + spi, thearg[spi]);
		} while (thearg[spi++]);
	
		write16(cpu, args, sp);
		args += 2;
	}

	write16(cpu, 0x0000, sp);
	set_ax(cpu, paravirt_argc);

	return 0;
}

static int
handle_exit (mos6502_t * cpu)
{
	INFO_PRINT("Received Paravirt. Exit request. Goodbye.\n");	
	if (paravirt_gui_enabled) {
		gui_deinit(cpu->sys->gui);
	}
	exit(EXIT_SUCCESS);
	return 0;
}

static int
handle_open (mos6502_t * cpu)
{
	char path[1024];
	int oflag = 0;
	int ret, i = 0;	
	
	unsigned mode  = pop_parm(cpu, cpu->y - 4);
	unsigned flags = pop_parm(cpu, 2);
	unsigned name  = pop_parm(cpu, 2);

	do {
		path[i] = mem_read(cpu->sys, name++);
	} while (path[i++]);

	DEBUG_PRINT("OPEN (\"%s\", 0x%04X)\n", path, flags);

	switch (flags & 0x03) {
		case 0x01:
			oflag |= O_RDONLY;
			break;
		case 0x02:
			oflag |= O_WRONLY;
			break;
		case 0x03:
			oflag |= O_RDWR;
			break;
	}
	
	if (flags & 0x10) {
		oflag |= O_CREAT;
	}
	
	if (flags & 0x20) {
		oflag |= O_TRUNC;
	}

	if (flags & 0x40) {
		oflag |= O_APPEND;
	}

	if (flags & 0x80) {
		oflag |= O_EXCL;
	}

	// keep gcc from complaining about unused mode
	(void)mode;

	ret = open(path, oflag);
	if (ret < 0) {
		ERROR_PRINT("Tried to open file %s with flags %x: %s\n", path, oflag,strerror(errno));
	} 

	set_ax(cpu, (uint16_t)ret);
	
	return 0;
}

static int
handle_close (mos6502_t * cpu)
{
	unsigned ret;
	unsigned fd = get_ax(cpu);

	DEBUG_PRINT("CLOSE (0x%04X)\n", fd);

	ret = close(fd);
	
	set_ax(cpu, ret);

	return 0;
}

static int
handle_read (mos6502_t * cpu)
{
	unsigned char * data;
	unsigned ret, i = 0;
	
	unsigned count = get_ax(cpu);
	unsigned buf   = pop_parm(cpu, 2);
	unsigned fd    = pop_parm(cpu, 2);

	DEBUG_PRINT("READ (0x%04x, 0x%04X, 0x%04X\n", fd, buf, count);

	data = malloc(count);

	ret = read(fd, data, count);

	if (ret != (unsigned)-1) {
		while (i < ret) {
			mem_write(cpu->sys, buf++, data[i++]);
		}
	}
	
	free(data);

	set_ax(cpu, ret);

	return 0;
}

static int
handle_write (mos6502_t * cpu)
{
	unsigned char * data;
	unsigned ret, i = 0;

	uint16_t count = get_ax(cpu);
	uint16_t buf   = pop_parm(cpu, 2);
	uint16_t fd    = pop_parm(cpu, 2);

	DEBUG_PRINT("Write (0x%04X, 0x%04X, 0x%04X)\n", fd, buf, count);
	
	data = malloc(count);

	while (i < count) {
		data[i++] = mem_read(cpu->sys, buf++);
	}

	ret = write(fd, data, count);

	free(data);

	set_ax(cpu, ret);

	return 0;
}


int 
handle_vmcall (decode_info_t * info)
{
	uint8_t call_num = mem_read(info->cpu->sys, info->addr);

	switch (call_num) {
		case VMCALL_ARGS:
			return handle_args(info->cpu);
		case VMCALL_EXIT:
			return handle_exit(info->cpu);
		case VMCALL_OPEN:
			return handle_open(info->cpu);
		case VMCALL_CLOSE:
			return handle_close(info->cpu);
		case VMCALL_READ:
			return handle_read(info->cpu);
		case VMCALL_WRITE:
			return handle_write(info->cpu);
		default:
			ERROR_PRINT("Unhandled VM CALL (instr len=%d) 0x%x (PC=0x%02x)\n", info->instr_len, call_num, info->cpu->pc);
			return -1;
	}

	return 0;
}

int
vmcall_init (int argc, char ** argv, int gui)
{
	INFO_PRINT("Initializing paravirtualization...\n");
	paravirt_argv = argv;
	paravirt_argc = (uint16_t)argc;
	paravirt_gui_enabled = gui;
	return 0;
}

