#include <lib/kprint.h>
#include <kernel/tty.h>
#include <kernel/types.h>
#include <karg.h>

void kputc(const char c) {
    tty_putchar(c);
}

void kputs(const char *s) {
    tty_write_str(s);
}

void kprintf(const char *format, ...)
{
    k_va_list args;
    k_va_start(args, format);

    for (const char * p = format; *p != '\0';p++) {
        if (*p != '%') {
            kputc(*p);
            continue;
        }

        union {
            char c, *s;
            reg_t x;
        } data;
        
        switch (*(++p)) {
            // Printing character
            case 'c':
                data.c = (int)k_va_arg(args, int);
                kputc(data.c);
                break;

            // Printing String
            case 's':
                data.s = k_va_arg(args, char*);
                kputs(data.s);
                break;

            // Printing hex data, with the width of cpu register
            case 'x':
                data.x = k_va_arg(args, reg_t);
                kprint_hex(data.x);
                break;

            // in case of string ending with %
            case '\0':
                kputc('%');
                return;

            // If it was not a format specifier
            default:
                kputc('%');
                kputc(*p);
        };
    }
    k_va_end(args);
}
// void ksnprintf(char *buf, size_t size, const char *format, ...);

void kprint_hex(const reg_t val) {
    const int START_SHIFT = (sizeof(reg_t) * 2 - 1) * 4;
    for (int i = START_SHIFT; i >= 0; i -= 4) {
        uint8_t num = (val >> i) & 0xF;
        tty_putchar(num < 10 ? ('0' + num): (num + ('A' - 10)));
    }
}

void kprint_hex8(const uint8_t val) {
    const int START_SHIFT = (sizeof(uint8_t) * 2 - 1) * 4;
    for (int i = START_SHIFT; i >= 0; i -= 4) {
        uint8_t num = (val >> i) & 0xF;
        tty_putchar(num < 10 ? ('0' + num): (num + ('A' - 10)));
    }
}

void kprint_hexdump(const void *addr, size_t size) {
    uint8_t *ptr = (uint8_t *) addr;
    uintptr_t addr_val = (uintptr_t) addr;

    for (size_t i = 0; i < size; i+= 16) {
        kprint_hex(addr_val + i);
        kputs(": ");

        size_t rem = size - i;
        size_t row_bytes = (rem < 16) ? rem : 16;
        for (size_t j = 0; j < row_bytes; j++) {
            kprint_hex8(ptr[i + j]);
            kputc(' ');
        }
        kputc('\n');
    }
    kputc('\n');
}

// void klog(int level, const char *format, ...);

