#ifndef _ARCH_X86_64_STRING_H
#define _ARCH_X86_64_STRING_H

#include <kdef.h>
#include <kernel/types.h>

#define __HAVE_ARCH_MEMSET
static inline void *arch_memset(void *dest, int val, size_t len) {
    void *orig_dest = dest;
    __asm__ __volatile__ (
        "rep stosb"
        : "+D"(dest), "+c"(len)
        : "a"((uint8_t)val)
        : "memory"
    );
    return orig_dest;
}

#define __HAVE_ARCH_MEMCPY
static inline void *arch_memcpy(void *restrict dest, const void *restrict src, size_t len) {
    void *orig_dest = dest;
    __asm__ __volatile__ (
        "rep movsb"
        : "+D"(dest), "+S"(src), "+c"(len)
        :
        : "memory"
    );
    return orig_dest;
}

/* Bonus: Fast 4KB page zeroing using 64-bit rep stosq (512 quadwords = 4096 bytes) */
#define __HAVE_ARCH_ZERO_PAGE
static inline void arch_zero_page(void *page_vaddr) {
    size_t qwords = 4096 / 8;
    __asm__ __volatile__ (
        "rep stosq"
        : "+D"(page_vaddr), "+c"(qwords)
        : "a"(0ULL)
        : "memory"
    );
}

#endif /* _ARCH_X86_64_STRING_H */
