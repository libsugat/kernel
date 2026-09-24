#include <kernel/types.h>
#include <kernel/tty.h>
#include <kdef.h>

#define VGA_WHITE_ON_BLACK 0x0F
#define VGA_GREEN_ON_BLACK 0x02
#define VGA_RED_ON_BLACK   0x04

#define VGA_BUFFER_ADDRESS 0xB8000

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

struct vga_entry {
    char c;
    u8 setting;
} PACKED;

#define DEBUG

static size_t tty_row;
static size_t tty_col;
static u8     tty_color;
static struct vga_entry volatile * const tty_buff = (struct vga_entry volatile *) VGA_BUFFER_ADDRESS;

#if defined(DEBUG)
#define COM1 0x3F8
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void init_serial(void) {
    outb(COM1 + 1, 0x00);    // Disable all interrupts
    outb(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1 + 0, 0x03);    // Set divisor to 3 (38400 baud)
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

void serial_putc(char c) {
    // Wait until transmit buffer is empty (Line Status Register bit 5)
    while ((inb(COM1 + 5) & 0x20) == 0);
    outb(COM1, c);
}
#endif

void tty_init(void) {
    tty_row = 0;
    tty_col = 0;
    tty_color = VGA_WHITE_ON_BLACK;
    for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; ++i) {
        tty_buff[i] = (struct vga_entry){' ', VGA_WHITE_ON_BLACK};
    }
    
#if defined(DEBUG)
    init_serial();
#endif
}

void tty_clear(void) {
    tty_init();
}


static void vga_scroll(void) {
    size_t n = (VGA_HEIGHT - 1) * VGA_WIDTH;
    for (size_t i = 0; i < n; i++) {
        tty_buff[i] = tty_buff[VGA_WIDTH + i];
    }

    struct vga_entry blank_entry = {
        .c = ' ', 
        .setting = tty_color
    };
    
    for (size_t i = 0; i < VGA_WIDTH; i++) {
        tty_buff[n + i] = blank_entry;
    }
}

void tty_putchar(char c) {
    switch (c) {
        case '\n':
            tty_col = 0;
            ++tty_row;
            break;    
        case '\t':
            tty_col += 4;
            break;
        default:;
            const size_t index = tty_row * VGA_WIDTH + tty_col;
            tty_buff[index] = (struct vga_entry){c, tty_color};

            ++tty_col;
            if (tty_col >= VGA_WIDTH) {
                tty_col = 0;
                tty_row++;
            }
    }

    if (tty_row == VGA_HEIGHT) {
        tty_row = VGA_HEIGHT - 1;
        vga_scroll();
    }

#if defined(DEBUG)
    serial_putc(c);
#endif
}

void tty_write(const char* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        tty_putchar(data[i]);
    }
}

void tty_write_str(const char * str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        tty_putchar(str[i]);
    }
}
