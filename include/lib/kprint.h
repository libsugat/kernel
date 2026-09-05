#ifndef LIB_KPRINT_H
#define LIB_KPRINT_H

#include <kernel/types.h>

void kputc(const char c);
void kputs(const char *s);
void kputsn(const char *s, size_t len);
void kprintf(const char *format, ...);
// void ksnprintf(char *buf, size_t size, const char *format, ...);

void kprint_hex(const reg_t val); // using your reg_t width

void kprint_hex8(const uint8_t val);
void kprint_hex16(const uint16_t val);
void kprint_hex32(const uint32_t val);
void kprint_hex64(const uint64_t val);

void kmemdump(const void *addr, size_t size);
void kmemdump_n(const void *addr, size_t size, const uint8_t access_width);

// void klog(int level, const char *format, ...);

#endif // KPRINT_H
