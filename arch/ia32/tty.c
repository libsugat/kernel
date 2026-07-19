#include <kernel/types.h>
#include <kernel/tty.h>

#define VGA_WHITE_ON_BLACK 0x0F
#define VGA_GREEN_ON_BLACK 0x02
#define VGA_RED_ON_BLACK   0x04

#define VGA_BUFFER_ADDRESS 0xB8000

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

struct vga_entry {
    char c;
    u8 setting;
} __attribute__((__packed__));

static size_t tty_row;
static size_t tty_col;
static u8     tty_color;
static struct vga_entry __volatile__ * const tty_buff = (struct vga_entry __volatile__ *) VGA_BUFFER_ADDRESS;

void tty_init(void) {
    tty_row = 0;
    tty_col = 0;
    tty_color = VGA_WHITE_ON_BLACK;
    for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; ++i) {
        tty_buff[i] = (struct vga_entry){' ', VGA_WHITE_ON_BLACK};
    }
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
    if (c == '\n') {
        tty_col = 0;
        ++tty_row;
    }
    else {
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
