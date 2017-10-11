#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <base.h>
#include <cartrom.h>

static uint8_t cartrom[CARTROM_SIZE];

#define CPU_6502 0
#define CPU_65C02 1

int
cartrom_load (const char * path)
{
	FILE * f = NULL;
	int val;
	struct stat s;
	int i = 0;

	// return the size
	if (stat(path, &s) == -1) {
		ERROR_PRINT("Could not stat file %s\n", path);
		return -1;
	}

	f = fopen(path, "rb");
	if (!f) {
		ERROR_PRINT("Could not open ROM file: %s\n", path);
		return -1;
	} 

	if ((val = fgetc(f)) != EOF) {
		if (val != CPU_6502 && val != CPU_65C02) {
			ERROR_PRINT("Invalid CPU type in ROM header\n");
			return -1;
		}
	}

	while ((val = fgetc(f)) != EOF && i < CARTROM_SIZE) {
		cartrom[i++] = val;
	}

	if (ferror(f)) {
		ERROR_PRINT("Error reading from rom_file: %s\n", strerror(errno));
		return -1;
	}

	INFO_PRINT("Loaded %d bytes of program ROM\n", i);

	fclose(f);

	return 0;
}


uint8_t
cartrom_read (uint16_t addr)
{
	uint8_t val = cartrom[CARTROM_OFFSET(addr)];
	DEBUG_PRINT("Read from cartridge ROM at address 0x%04x (returning 0x%02x)\n", addr, val);
	return val;
}

