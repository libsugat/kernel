#include <lib/kprint.h>
#include <kernel/tty.h>
#include <kernel/types.h>
#include <karg.h>

void kputc(const char c) {
    tty_putchar(c);
}

void kputs(const char *s) {
    if (!s) {
        s = "(null)";
    }
    tty_write_str(s);
}

void kputsn(const char *s, size_t len) {
    if (!s) {
        s = "(null)";
        len = 6;
    }
    tty_write(s, len);
}

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
        kprint_uint(-(uint32_t)n);
    } else {
        kprint_uint((uintmax_t)n);
    }
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
        
        switch (*(++p)) {
            // Printing character
            case 'c':
                kputc(k_va_arg(args, int));
                break;

            // Printing String
            case 's':
                kputs(k_va_arg(args, char*));
                break;

            // Printing hex data, with the width of cpu register
            case 'x':
                kprint_hex(k_va_arg(args, reg_t));
                break;

            case 'd':
                kprint_int(k_va_arg(args, int32_t));
                break;

            case 'u':
                kprint_uint(k_va_arg(args, uint32_t));
                break;

            // in case of string ending with %
            case '\0':
                kputc('%');
                goto printing_done;

            // If it was not a format specifier
            default:
                kputc('%');
                kputc(*p);
        };
    }
printing_done:
    k_va_end(args);
}
// void ksnprintf(char *buf, size_t size, const char *format, ...);

// Just a macro
#define kprint_hex_fn_def(suffix, type)                         \
void kprint_hex##suffix(const type val) {                       \
    const int START_SHIFT = (sizeof(type) * 2 - 1) * 4;         \
    for (int i = START_SHIFT; i >= 0; i -= 4) {                 \
        uint8_t num = (val >> i) & 0xF;                         \
        tty_putchar('0' + num + ((9 - num) >> 8 & 7));          \
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

