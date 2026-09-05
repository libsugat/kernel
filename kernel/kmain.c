#include <arch/interrupt.h>
#include <arch/init.h>

#include <kernel/boot_info.h>
#include <kernel/interrupts.h>
#include <kernel/tty.h>
#include <kernel/mm.h>

#include <lib/kprint.h>

#include <kdef.h>

__attribute__((__noreturn__)) 
void kernel_start(
    struct boot_info_struct *boot_info_ptr,
    reg_t UNUSED bootloader_signature
) 
{
    tty_init();

    kputs("Hello here I am kernel, Thanks Bootloader... \n\n");
    kputs("Initializing OS...\n");


    arch_hardware_init(boot_info_ptr);
    init_mm(boot_info_ptr);
    initialize_interrupts();
    kputs("OS Initialization Successful...\n\n");

    // remove this as soon as possible
    kputs("Good Night...\n");
    arch_enable_interrupts();
    for (;;) {
        // kputs("zzz....\n");
        arch_wait_for_interrupt();
        // kputs("Who what that, shut up..\n");
    }
}
