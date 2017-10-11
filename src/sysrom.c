#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <base.h>
#include <sys.h>
#include <sysrom.h>


static uint8_t sysrom[SYSROM_SIZE];


sysrom_t * 
sysrom_init(system_t * sys)
{
	sysrom_t * sr = NULL;

	INFO_PRINT("Initializing system ROM...\n");

	sr = malloc(sizeof(sysrom_t));
	if (!sr) {
		ERROR_PRINT("Could not allocate system ROM\n");
		return NULL;
	}
	memset(sr, 0, sizeof(sysrom_t));

	sr->sys = sys;

	// Fill system ROM with illegal opcodes
	memset(sysrom, 0xFF, SYSROM_SIZE);

	// set the inital reset vector to beginning of cartridge ROM
	sysrom[SYSROM_SIZE-4] = 0x00; // 0xfffc
	sysrom[SYSROM_SIZE-3] = 0xa0; // 0xfffd
	

	return sr;
}

int
sysrom_load (const char * path)
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
		ERROR_PRINT("Could not open System ROM file: %s\n", path);
		return -1;
	} 


	while ((val = fgetc(f)) != EOF && i < SYSROM_SIZE) {
		sysrom[i++] = val;
	}

	if (ferror(f)) {
		ERROR_PRINT("Error reading from rom_file: %s\n", strerror(errno));
		return -1;
	}

	INFO_PRINT("Loaded %d bytes of System ROM\n", i);

	fclose(f);

	return 0;
}


uint8_t 
sysrom_read (uint16_t addr)
{
	uint8_t val = sysrom[SYSROM_OFFSET(addr)];
	DEBUG_PRINT("Read from system ROM at address 0x%04x (returning 0x%02x)\n", addr, val);
	return val;
}




