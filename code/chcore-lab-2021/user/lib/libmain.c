#include <lib/syscall.h>

extern int main(int argc, char *argv[], char *envp[]);

void _start_c(long *p)
{
	int argc = p[0];
	char **argv = (void *)(p + 1);
	char **envp = (void *)(p + 3);

	main(argc, argv, envp);
	/*
	 * Lab3: Your code here
	 * Complete the main function
	 */
	usys_exit(0);
	return;
}
