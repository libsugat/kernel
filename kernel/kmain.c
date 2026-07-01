#include <kernel/boot_params.h>
#include <kernel/tty.h>
#define PAGE_SIZE 4096

void start(void) {
    tty_init();

    char * msg = "Hello here I am kernel, Thanks Bootloader....\n";
    tty_write_str(msg);

    for (;;) {
        __asm__ __volatile__("hlt");
    }
}
