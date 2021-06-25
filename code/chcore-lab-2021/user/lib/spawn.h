#pragma once

#include <lib/launcher.h>
#include <lib/type.h>

int spawn(char *path, int *new_process_cap, int *new_thread_cap,
	  struct pmo_map_request *pmo_map_reqs, int nr_pmo_map_reqs, int caps[],
	  int nr_caps, int aff);
