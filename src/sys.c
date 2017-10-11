
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <base.h>
#include <sys.h>
#include <memctrl.h>
#include <sysrom.h>
#include <cartrom.h>
#include <io.h>
#include <shell.h>

#include <mos6502/cpu.h>
#include <mos6502/vmcall.h>
#include <dev/controller.h>
#include <dev/gfx.h>
#include <gui/gui.h>


static void
ram_init (system_t * sys)
{
	INFO_PRINT("Initializing RAM...\n");

	// set it to all FFs (illegal opcodes)
	int i;
	for (i = 0; i < RAM_SIZE; i++) {
		sys->ram[i] = 0xff;
	}

}


system_t * 
sys_init (const char * rom_path,
	  const char * sysrom_path,
	  int is_interactive,
 	  int argc_ext,
	  char ** argv_ext)
{
	INFO_PRINT("Initializing Hawknest system%s...\n",
		(is_interactive ? " (interactive mode)" : ""));

	system_t * s = malloc(sizeof(system_t));
	

	if (!s) {
		ERROR_PRINT("Could not allocate system struct\n");
		return NULL;
	}
	memset(s, 0, sizeof(system_t));

	/* initialize the memory controller */
	s->memctrl = memctrl_init(s);
	if (!s->memctrl) {
		ERROR_PRINT("Could not initialize memory controller\n");
		return NULL;
	}

	/* initialize RAM */
	ram_init(s);

	if (cartrom_load(rom_path) != 0) {
		ERROR_PRINT("Error loading cartridge ROM (%s)\n", rom_path);
		return NULL;
	}

	sysrom_init(s);

	if (sysrom_load(sysrom_path) != 0) {
		ERROR_PRINT("Error loading System ROM (%s)\n", sysrom_path); return NULL;
	}

	/* initialize the I/O subsystem */
	s->io = io_init(s);

	if (!is_interactive) {
		/* init the GUI */
		gui_init(s);

		/* initialize devices */
		controller_init(s);
		gfx_init(s);
	}

	/* initialize the paravirtual hooks */
	vmcall_init(argc_ext, argv_ext, !is_interactive);

	/* initialize the CPU */
	s->cpu = mos6502_init(s);

	if (is_interactive) {
		start_shell(s->cpu);
	} else {
		gui_loop(s);
	}

	return s;
}


void 
sys_reset (system_t * sys)
{
	// reset RAM
	ram_init(sys);

	mos6502_reset(sys->cpu);
}

