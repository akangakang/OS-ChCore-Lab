#pragma once

#include <lib/type.h>

int create_thread(void *(*func) (void *), u64 arg, u32 prio, s32 cpuid);
