// buggy program - faults with a read from location zero

#include <lib/print.h>

int main(int argc, char *argv[], char *envp[])
{
	printf("I read %08x from location 1!\n", *(unsigned *)1);
	return 0;
}
