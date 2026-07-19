#include <lib/kprint.h>
#include <kernel/tty.h>
#include <kernel/types.h>
#include <stdarg.h>

void kputchar(const char c) {
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
            kputchar(*p);
            continue;
        }
        switch (*(++p)) {
            case 'c':
                char c = k_va_arg(args, char);
                kputchar(c);
                break;
            case 's':
                char *s = k_va_arg(args, char*);
                kputs(s);
                break;
            case 'x':
                reg_t x = k_va_arg(args, reg_t);
                kprint_hex(x);
                break;
            default:
                kputchar('%');
                kputchar(*p);
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
    volatile uint32_t *ptr = (volatile uint32_t *) addr;
    // uint8_t *ptr = (uint8_t *) addr;
    uintptr_t addr_val = (uintptr_t) addr;

    kputchar('\n');
    for (size_t i = 0; i < size; i+= 16) {
        kprint_hex(addr_val + i);
        kputs(": ");

        size_t rem = size - i;
        size_t row_bytes = (rem < 16) ? rem : 16;
        // for (size_t j = 0; j < row_bytes; j++) {
        //     kprint_hex8(ptr[i + j]);
        
        for (size_t j = 0; j < row_bytes; j+=4) {
            kprint_hex(ptr[(i + j) / 4]);
            // kprint_hex(ptr[(i + j) / 4]);
            kputchar(' ');
        }
        kputchar('\n');
    }
    kputchar('\n');
}

// void klog(int level, const char *format, ...);

