#pragma once
#include <lib/type.h>

void memset(void *dst, int c, u64 len);
void memcpy(void *dst, const void *src, u64 len);
int memcmp(const void *s1, const void *s2, size_t n);
void strcpy(char *dst, const char *src);
u32 strcmp(const char *s1, const char *s2);
u32 strncmp(const char *s1, const char *s2, size_t size);
size_t strlen(const char *s);
char *strstr(const char *haystack, const char *needle);
char *strcat(char *dest, const char *src);
