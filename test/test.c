#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <conio.h>

extern void testwrite();

static void
test_fwrite () 
{
	char * x = "hello world\n";
	fwrite(x, strlen(x)+1, 1, stdout);
}

static void 
test_asmwrite ()
{
	testwrite();
}

static void
test_legfilewrite()
{
	char * x = "hello world\n";
	int fd = open("test.txt", O_RDWR|O_CREAT);
	if (fd < 0) {
		printf("Could not open file: %s\n", strerror(errno));
		return;
	}
	write(fd, x, strlen(x)+1);
	close(fd);
}
static void
test_filewrite ()
{
	char * x = "hello world\n";
	FILE * fd = fopen("test.txt", "w+");
	if (!fd) {
		printf("Could not open file: %s\n", strerror(errno));
		return;
	}
	if (fwrite(x, strlen(x)+1, 1, fd) != strlen(x)+1) {
		printf("Could not write file\n");
	}
	fclose(fd);
}

static void
test_printf () {
	printf("Hello printf\n");
}

int main (int argc, char ** argv) {
	
	test_fwrite();
	test_asmwrite();
	test_printf();
	test_legfilewrite();
	
	return 0;
}

