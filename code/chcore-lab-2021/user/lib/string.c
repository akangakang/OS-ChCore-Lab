#include <lib/type.h>

/*
 * A very shabby implementation, optimize it if you like.
 */

void memset(void *dst, int c, u64 len)
{
	u64 i = 0;
	for (; i < len; i += 1) {
		((u8 *) dst)[i] = (u8) c;
	}
}

void memcpy(void *dst, const void *src, u64 len)
{
	u64 i = 0;
	for (; i < len; i += 1) {
		((u8 *) dst)[i] = ((u8 *) src)[i];
	}
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	int i = 0;
	const char *p = (const char *)s1;
	const char *q = (const char *)s2;

	while (i < n && *p == *q) {
		p += 1;
		q += 1;
		i += 1;
	}

	return (i == n) ? 0 : (*p - *q);
}

void strcpy(char *dst, const char *src)
{
	u64 i = 0;
	while (src[i] != 0) {
		dst[i] = src[i];
		i += 1;
	}
	dst[i] = 0;
}

u32 strcmp(const char *p, const char *q)
{
	while (*p && *p == *q) {
		p += 1;
		q += 1;
	}
	return (u32) ((u8) * p - (u8) * q);
}

u32 strncmp(const char *p, const char *q, size_t n)
{
	while (n > 0 && *p && *p == *q) {
		n -= 1;
		p += 1;
		q += 1;
	}
	if (n == 0)
		return 0;
	else
		return (u32) ((u8) * p - (u8) * q);
}

size_t strlen(const char *s)
{
	size_t i = 0;

	while (*s++)
		i++;

	return i;
}

// returns true if X and Y are same
static int compare(const char *X, const char *Y)
{
	while (*X && *Y) {
		if (*X != *Y)
			return 0;

		X++;
		Y++;
	}

	return (*Y == '\0');
}

// Function to implement strstr() function
const char *strstr(const char *X, const char *Y)
{
	while (*X != '\0') {
		if ((*X == *Y) && compare(X, Y))
			return X;
		X++;
	}

	return NULL;
}

char *strcat(char *dest, const char *src)
{
	size_t i, j;
	for (i = 0; dest[i] != '\0'; i++) ;
	for (j = 0; src[j] != '\0'; j++)
		dest[i + j] = src[j];
	dest[i + j] = '\0';

	return dest;
}
