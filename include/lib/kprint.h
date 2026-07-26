#ifndef LIB_KPRINT_H
#define LIB_KPRINT_H

#include <kernel/types.h>

void kputc(const char c);
void kputs(const char *s);
void kprintf(const char *format, ...);
// void ksnprintf(char *buf, size_t size, const char *format, ...);

void kprint_hex(const reg_t val); // using your reg_t width
void kprint_hexdump(const void *addr, size_t size);

// void klog(int level, const char *format, ...);

#endif // KPRINT_H
