#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <sys/mman.h>

/* unit test */
#include "minunit.h"
/* kernel/mm/xxx */
#include "buddy.h"
#include "slab.h"

#define ROUND 1000
#define NPAGES (128 * 1000)

#define START_VADDR phys_to_virt(24*1024*1024)

void printk(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vprintf(fmt, va);
	va_end(va);
}

struct phys_mem_pool global_mem;

/* test buddy allocator */

static unsigned long buddy_num_free_page(struct phys_mem_pool *zone)
{
	unsigned long i, ret;

	ret = 0;
	for (i = 0; i < BUDDY_MAX_ORDER; ++i) {
		ret += zone->free_lists[i].nr_free;
	}
	return ret;
}

static void valid_page_idx(struct phys_mem_pool *zone, long idx)
{
	mu_assert((idx < zone->pool_phys_page_num)
		  && (idx >= 0), "invalid page idx");
}

static unsigned long get_page_idx(struct phys_mem_pool *zone, struct page *page)
{
	long idx;

	idx = page - zone->page_metadata;
	valid_page_idx(zone, idx);

	return idx;
}

static void test_alloc(struct phys_mem_pool *zone, long n, long order)
{
	long i;
	struct page *page;

	for (i = 0; i < n; ++i) {
		page = buddy_get_pages(zone, order);
		mu_check(page != NULL);
		get_page_idx(zone, page);
	}
	return;
}

void test_buddy(void)
{
	void *start;
	unsigned long npages;
	unsigned long size;
	unsigned long start_addr;

	long nget;
	long ncheck;

	struct page *page;
	long i;

	/* free_mem_size: npages * 0x1000 */
	npages = 128 * 0x1000;
	/* PAGE_SIZE + page metadata size */
	size = npages * (sizeof(struct page));
	start = mmap((void *)0x50000000000, size, PROT_READ | PROT_WRITE,
		     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	size = npages * (0x1000);
	start_addr =
	    (unsigned long)mmap((void *)0x60000000000, size,
				PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	/* skip the metadata area */
	init_buddy(&global_mem, start, start_addr, npages);

	/* check the init state */
	nget = buddy_num_free_page(&global_mem);
	ncheck = npages / powl(2, BUDDY_MAX_ORDER - 1);
	//printf("ncheck is %ld\n", ncheck);
	mu_check(nget == ncheck);

	/* alloc single page for $npages times */
	test_alloc(&global_mem, npages, 0);

	/* should have 0 free pages */
	nget = buddy_num_free_page(&global_mem);
	ncheck = 0;
	mu_check(nget == ncheck);

	/* free all pages */
	for (i = 0; i < npages; ++i) {
		page = global_mem.page_metadata + i;
		buddy_free_pages(&global_mem, page);
	}
	nget = buddy_num_free_page(&global_mem);
	ncheck = npages / powl(2, BUDDY_MAX_ORDER - 1);
	mu_check(nget == ncheck);

	/* alloc 2-pages for $npages/2 times */
	test_alloc(&global_mem, npages / 2, 1);

	/* should have 0 free pages */
	nget = buddy_num_free_page(&global_mem);
	ncheck = 0;
	mu_check(nget == ncheck);

	/* free all pages */
	for (i = 0; i < npages; i += 2) {
		page = global_mem.page_metadata + i;
		buddy_free_pages(&global_mem, page);
	}
	nget = buddy_num_free_page(&global_mem);
	ncheck = npages / powl(2, BUDDY_MAX_ORDER - 1);
	mu_check(nget == ncheck);

	/* alloc MAX_ORDER-pages for  */
	test_alloc(&global_mem, npages / powl(2, BUDDY_MAX_ORDER - 1),
		   BUDDY_MAX_ORDER - 1);

	/* should have 0 free pages */
	nget = buddy_num_free_page(&global_mem);
	ncheck = 0;
	mu_check(nget == ncheck);

	/* free all pages */
	for (i = 0; i < npages; i += powl(2, BUDDY_MAX_ORDER - 1)) {
		page = global_mem.page_metadata + i;
		buddy_free_pages(&global_mem, page);
	}
	nget = buddy_num_free_page(&global_mem);
	ncheck = npages / powl(2, BUDDY_MAX_ORDER - 1);
	mu_check(nget == ncheck);

	/* alloc single page for $npages times */
	test_alloc(&global_mem, npages, 0);

	/* should have 0 free pages */
	nget = buddy_num_free_page(&global_mem);
	ncheck = 0;
	mu_check(nget == ncheck);

	/* free a half pages */
	for (i = 0; i < npages; i += 2) {
		page = global_mem.page_metadata + i;
		buddy_free_pages(&global_mem, page);
	}
	nget = buddy_num_free_page(&global_mem);
	ncheck = npages / 2;
	mu_check(nget == ncheck);

	/* free another half pages */
	for (i = 1; i < npages; i += 2) {
		page = global_mem.page_metadata + i;
		buddy_free_pages(&global_mem, page);
	}
	nget = buddy_num_free_page(&global_mem);
	ncheck = npages / powl(2, BUDDY_MAX_ORDER - 1);
	mu_check(nget == ncheck);
}

MU_TEST_SUITE(test_suite)
{
	MU_RUN_TEST(test_buddy);
}

int main(int argc, char *argv[])
{
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return minunit_status;
}
