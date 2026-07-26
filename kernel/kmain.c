#include <kernel/boot_params.h>
#include <kernel/interrupts.h>
#include <kernel/tty.h>

#include <lib/kprint.h>

__attribute__((__noreturn__)) void kernel_start(void) {
    tty_init();

    kputs("Hello here I am kernel, Thanks Bootloader... \n\n");
    kputs("Initializing OS...\n");

    initialize_interrupts();
    kputs("OS Initialization Successful...\n\n");

    // remove this as soon as possible
    kputs("Good Night...\n");
    for (;;) {
        kputs("zzz....\n");
        arch_wait_for_interrupt();
        kputs("Who what that, shut up..\n");
    }
}
