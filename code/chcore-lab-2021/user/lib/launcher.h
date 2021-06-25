#pragma once

#include <lib/cpio.h>

#include <lib/elf.h>
#include <lib/launcher.h>
#include <lib/proc.h>

struct info_page {
	volatile u64 ready_flag;
	volatile u64 exit_flag;
	u64 nr_args;
	u64 args[];
};

#define CPIO_BIN 0x5000000

int parse_elf_from_binary(const char *binary, struct user_elf *user_elf);
void *single_file_handler(const void *start, size_t size, void *data);
int readelf_from_kernel_cpio(const char *filename, struct user_elf *user_elf);
int readelf_from_fs(const char *pathbuf, struct user_elf *user_elf);
