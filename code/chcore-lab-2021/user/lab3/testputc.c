#include <lib/syscall.h>

int main(int argc, char *argv[], char *envp[])
{
	usys_putc('&');
	usys_putc('\n');
	return 0;
}
