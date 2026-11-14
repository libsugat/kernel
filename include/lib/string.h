#ifndef _LIB_STRING_H
#define _LIB_STRING_H

#include <kernel/types.h>

void *memset(void *dest, int val, size_t len);
void *memcpy(void *restrict dest, const void *restrict src, size_t len);
void *memmove(void *dest, const void *src, size_t len);
int memcmp(const void *s1, const void *s2, size_t len);
size_t strlen(const char *str);
void zero_page(void *page_vaddr);

#endif /* _LIB_STRING_H */
