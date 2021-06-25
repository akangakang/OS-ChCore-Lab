#pragma once

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef long long s64;
typedef int s32;
typedef short s16;
typedef signed char s8;

#define NULL ((void *)0)

typedef char bool;
#define true (1)
#define false (0)

typedef u64 size_t;
typedef long long int ssize_t;
typedef long long int off_t;
typedef unsigned long long int ino_t;

struct process_metadata {
	u64 phdr_addr;
	u64 phentsize;
	u64 phnum;
	u64 flags;
	u64 entry;
};

typedef u64 vmr_prop_t;

struct user_elf_seg {
	u64 elf_pmo;
	size_t seg_sz;
	u64 p_vaddr;
	vmr_prop_t flags;
};

struct user_elf {
	struct user_elf_seg user_elf_seg[2];
	char path[256];
	struct process_metadata elf_meta;
};

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
