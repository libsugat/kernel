#include<kernel/types.h>

#include<lib/string.h>

void *memset(void *dest, int val, size_t len) {
#ifdef __HAVE_ARCH_MEMSET
    return arch_memset(dest, val, len);
#else
    uint8_t *ptr = (uint8_t *)dest;
    uint8_t byte = (uint8_t)val;
    while (len--) {
        *ptr++ = byte;
    }
    return dest;
#endif
}

void *memcpy(void *restrict dest, const void *restrict src, size_t len) {
#ifdef __HAVE_ARCH_MEMCPY
    return arch_memcpy(dest, src, len);
#else
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (len--) {
        *d++ = *s++;
    }
    return dest;
#endif
}

void zero_page(void *page_vaddr) {
#ifdef __HAVE_ARCH_ZERO_PAGE
    arch_zero_page(page_vaddr);
#else
    uint64_t *p = (uint64_t *)page_vaddr;
    for (size_t i = 0; i < (4096 / sizeof(uint64_t)); i++) {
        p[i] = 0;
    }
#endif
}

void *memmove(void *dest, const void *src, size_t len) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    if (d == s || len == 0) {
        return dest;
    }

    if (d < s) {
        while (len--) {
            *d++ = *s++;
        }
    } else {
        d += len;
        s += len;
        while (len--) {
            *--d = *--s;
        }
    }
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t len) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    while (len--) {
        if (*p1 != *p2) {
            return (int)*p1 - (int)*p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}
