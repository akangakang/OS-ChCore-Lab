// buggy program - causes a bad instruction exception

#include <lib/print.h>
int main(int argc, char *argv[], char *envp[])
{
	asm volatile ("mrs x0, elr_el1");
	printf("Survived protected instruction\n");
	return 0;
}
