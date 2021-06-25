#include <lib/print.h>
#include <lib/type.h>
#include <lib/syscall.h>

#define PMO_SIZE        0x1000
#define MAP_VA          0x1000000

/* virtual memory rights */
#define VM_READ  (1 << 0)
#define VM_WRITE (1 << 1)
#define VM_EXEC  (1 << 2)

/* PMO types */
#define PMO_ANONYM 0
#define PMO_DATA   1

/* a thread's own cap_group */
#define SELF_CAP   0

int main(int argc, char *argv[], char *envp[])
{
	int pmo_cap, r;

	pmo_cap = usys_create_pmo(0x1000, PMO_ANONYM);
	if (pmo_cap < 0) {
		printf("usys_create_pmo ret:%d\n", pmo_cap);
		usys_exit(pmo_cap);
	}
	r = usys_map_pmo(SELF_CAP, pmo_cap, MAP_VA, VM_READ | VM_WRITE);
	if (r < 0) {
		printf("usys_map_pmo ret:%d\n", r);
		usys_exit(r);
	}
	return 0;

}
