#include "malloc.h"
#include "bug.h"
#include "string.h"
#include "syscall.h"

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

#define MALLOC_SZ (50ull * 1024 * 1024)
static char *malloc_buf_;
static size_t malloc_header_ = 0;

void *malloc(size_t size)
{
	BUG_ON(malloc_header_ + size > MALLOC_SZ);

	if (malloc_header_ == 0) {
		int pmo_cap, r;
		pmo_cap = usys_create_pmo(MALLOC_SZ, PMO_ANONYM);
		if (pmo_cap < 0) {
			printf("usys_create_pmo ret:%d\n", pmo_cap);
			usys_exit(pmo_cap);
		}
		r = usys_map_pmo(SELF_CAP, pmo_cap, MAP_VA, VM_READ | VM_WRITE);
		if (r < 0) {
			printf("usys_map_pmo ret:%d\n", r);
			usys_exit(r);
		}

		malloc_buf_ = (char *)MAP_VA;
	}

	void *ptr = (void *)&malloc_buf_[malloc_header_];
	malloc_header_ += size;

	return ptr;
}

void *calloc(size_t nmemb, size_t size)
{
	(void)nmemb;
	(void)size;

	void *ptr = malloc(nmemb * size);
	memset(ptr, 0, nmemb * size);

	return ptr;
}

void free(void *ptr)
{
	(void)ptr;
}
