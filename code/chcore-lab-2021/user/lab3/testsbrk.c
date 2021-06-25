#include <lib/syscall.h>
#include <lib/print.h>
#include <lib/type.h>

#define ALLOCATE_SIZE 4096
#define STRING_SIZE	  64

int main(int argc, char *argv[], char *envp[])
{
	int i;
	u64 start, end;
	char *s;

	start = usys_handle_brk(0);
	end = usys_handle_brk(start + ALLOCATE_SIZE);

	if (end - start < ALLOCATE_SIZE) {
		printf("sbrk not correctly implemented\n");
	}

	s = (char *)start;
	for (i = 0; i < STRING_SIZE; i++) {
		printf("%d,", i);
		s[i] = 'A' + (i % 26);
	}
	s[STRING_SIZE] = '\0';
	printf("\n");

	printf("SBRK_TEST: %s\n", s);
	return 0;
}
