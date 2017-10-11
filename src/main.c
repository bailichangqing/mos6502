#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#include <base.h>
#include <sys.h>


static void
version (void)
{
	printf("Hawknest 6502v Emulator Version %s\n", HAWKNEST_VERSION_STRING);
	printf("Kyle C. Hale (c) 2017, Illinois Institute of Technology\n");
	exit(EXIT_SUCCESS);
}


static void
usage (char ** argv)
{
	fprintf(stderr, "This is the Hawknest 6502v system emulator. Usage:\n\n");
	fprintf(stderr, "  %s [options] -c <cartrom_file> -s <sysrom_file>\n\n", argv[0]);
	fprintf(stderr, "Arguments:\n\n");
	fprintf(stderr, " %20.20s Cartridge ROM file to use (required)\n", "--cartridge, -c");
	fprintf(stderr, " %20.20s System ROM file to use (required)\n", "--sysrom, -s");
	fprintf(stderr, "\n");
	fprintf(stderr, "Operating modes:\n\n");
	fprintf(stderr, " %20.20s Run an interactive Hawknest session (useful for debugging)\n", "--interactive, -i");
	fprintf(stderr, "\n");
	fprintf(stderr, "Miscellaneous:\n\n");
	fprintf(stderr, " %20.20s Print this message\n", "--help, -h");
	fprintf(stderr, " %20.20s Print the version number and exit\n", "--version, -V");

	fprintf(stderr, "\n\n");

	fprintf(stderr, "To get a list of options used in the interactive shell, type \"help\" or \"?\" at the shell.\n\n");
	
	exit(EXIT_SUCCESS);
}

static struct option long_options[] = {
	{"cartridge", required_argument, 0, 'c'},
	{"sysrom", required_argument, 0, 's'},
	{"interactive", no_argument, 0, 'i'},
	{"help", no_argument, 0, 'h'},
	{"version", no_argument, 0, 'V'},
	{0, 0, 0, 0}
};

int 
main (int argc, char ** argv) 
{
	int c;
	char * cartrom_path = NULL;
	char * sysrom_path = NULL;
	int is_interactive = 0;

	while (1) {
		int opt_idx = 0;
		c = getopt_long(argc, argv, "s:c:ihV", long_options, &opt_idx);
		
		if (c == -1) {
			break;
		}

		switch (c) {
			case 'c':
				cartrom_path = optarg;
				break;
			case 's':
				sysrom_path = optarg;
				break;
			case 'i':
				is_interactive = 1;
				break;
			case 'V':
				version();
				break;
			case 'h':
				usage(argv);
				break;
			case '?':
				break;
			default:
				printf("Unknown option: %o.\n", c);
				break;
		}
	}
	
	if (cartrom_path == NULL) {
		fprintf(stderr, "Must provide a cartridge ROM file!\n"); 
		usage(argv);
	}

	if (sysrom_path == NULL) {
		fprintf(stderr, "Must provide a system ROM binary!\n");
		usage(argv);
	}

	system_t * sys = sys_init(cartrom_path, 
				  sysrom_path, 
				  is_interactive,
				  argc - optind,
				  &argv[optind]);

	if (!sys) {
		ERROR_PRINT("Could not create Hawknest system\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}


