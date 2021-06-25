// buggy program - causes a bad instruction exception

#include <lib/print.h>
int main(int argc, char *argv[], char *envp[])
{
	asm volatile (".byte 0x40, 0x00, 0x00, 0x00");
	printf("Survived bad instruction\n");
	return 0;
}
