#include <lib/kprint.h>
#include <kernel/tty.h>
#include <kernel/types.h>
#include <karg.h>

static void kprint_uint(uint32_t n) {
    char buffer[10]; 
    size_t i = 0;
    if (n == 0) {
        kputc('0');
        return;
    }
    while (n > 0) {
        buffer[i++] = (char)('0' + (n % 10));
        n /= 10;
    }
    while (i > 0) {
        kputc(buffer[--i]);
    }
}

static void kprint_int(int32_t n) {
    if (n < 0) {
        kputc('-');
        uint32_t u = -(uint32_t)n;
        kprint_uint(u);
    } else {
        kprint_uint((uintmax_t)n);
    }
}

void kputc(const char c) {
    tty_putchar(c);
}

void kputs(const char *s) {
    tty_write_str(s);
}

void kputsn(const char *s, size_t len) {
    tty_write(s, len);
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
            uint32_t num;
            int32_t unum;
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

            case 'd':
                data.x = k_va_arg(args, int32_t);
                kprint_int(data.num);
                break;

            case 'u':
                data.x = k_va_arg(args, uint32_t);
                kprint_uint(data.unum);
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

// Just a macro
#define kprint_hex_fn_def(suffix, type)                         \
void kprint_hex##suffix(const type val) {                       \
    const int START_SHIFT = (sizeof(type) * 2 - 1) * 4;         \
    for (int i = START_SHIFT; i >= 0; i -= 4) {                 \
        uint8_t num = (val >> i) & 0xF;                         \
        tty_putchar(num < 10 ? ('0' + num): (num + ('A' - 10)));\
    }                                                           \
}

kprint_hex_fn_def(, reg_t); // For default type of register
kprint_hex_fn_def(8, uint8_t);
kprint_hex_fn_def(16, uint16_t);
kprint_hex_fn_def(32, uint32_t);
kprint_hex_fn_def(64, uint64_t);


void kmemdump_n(const void *addr, size_t size, const uint8_t access_width) {
    uintptr_t addr_val = (uintptr_t) addr;

    for (size_t i = 0; i < size; i+= 16) {
        kprint_hex(addr_val + i);
        kputs(": ");

        size_t rem = size - i;
        size_t row_bytes = (rem < 16) ? rem : 16;
        for (size_t j = 0; j < row_bytes; j += access_width) {
            uintptr_t current_addr = addr_val + i + j;
            switch (access_width) {
                case 1: kprint_hex8(*(volatile uint8_t *)current_addr); break;
                case 2: kprint_hex16(*(volatile uint16_t *)current_addr); break;
                case 4: kprint_hex32(*(volatile uint32_t *)current_addr); break;
                case 8: kprint_hex64(*(volatile uint64_t *)current_addr); break;
                default: kprint_hex(*(volatile uint8_t *)current_addr);
            }
            kputc(' ');
        }
        kputc('\n');
    }
    kputc('\n');
}

void kmemdump(const void *addr, size_t size) {
    kmemdump_n(addr, size,  sizeof(uint8_t));
}

// void klog(int level, const char *format, ...);

