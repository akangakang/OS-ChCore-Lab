#include <lib/print.h>

#define BUG_ON(expr) \
	do { \
		if ((expr)) { \
			printf("BUG: %s:%d %s\n", __func__, __LINE__, #expr); \
			for ( ; ; ) { \
			} \
		} \
	} while (0)

#define BUG(str) \
	do { \
		printf("BUG: %s:%d %s\n", __func__, __LINE__, str); \
		for ( ; ; ) { \
		} \
	} while (0)

#define WARN(msg) \
	printf("WARN: %s:%d %s\n", __func__, __LINE__, msg)

#define WARN_ON(cond, msg) \
	do { \
		if ((cond)) { \
			printf("WARN: %s:%d %s on " #cond "\n", \
			       __func__, __LINE__, msg); \
		} \
	} while (0)

#define fail_cond(cond, fmt, ...) do {   \
	if (!(cond)) break;              \
	printf(fmt, ##__VA_ARGS__);      \
	usys_exit(-1);                   \
} while (0)
