#include <arch/init.h>

#include <kernel/boot_info.h>
#include <kernel/interrupts.h>
#include <kernel/tty.h>
#include <kernel/mm.h>
#include <kernel/sched.h>

#include <lib/kprint.h>

void task_A(void);
void task_B(void);
extern void *page_meta_to_start_addr(struct page_meta *);
void initialize_interrupts() {}

__attribute__((__noreturn__)) 
void kernel_start(
    struct boot_info_struct *boot_info_ptr,
    reg_t UNUSED bootloader_signature
) 
{
    cpu_init_early();
    tty_init();

    kputs("Hello here I am kernel, Thanks Bootloader... \n\n");
    kputs("Initializing OS...\n");

    arch_hardware_init(boot_info_ptr);
    initialize_interrupts();
    init_mm(boot_info_ptr);
    init_sched();
    kputs("OS Initialization Successful...\n\n");

    // remove this asap
    struct page_meta * task_a_stack = alloc_page_frame(0);
    struct page_meta * task_b_stack = alloc_page_frame(0);
    create_new_task(&task_A, page_meta_to_start_addr(task_a_stack));
    create_new_task(&task_B, page_meta_to_start_addr(task_b_stack));
    kputs("Good Night...\n");
    arch_enable_interrupts();
    sched();
    for (;;) {
        kprintf("IDLE TASK\n");
        arch_wait_for_interrupt();
    }
}

void task_A(void) {
    for (;;) {
        kprintf("SUGAT\n");
    }
}

void task_B(void) {
    for (;;) {
        kprintf("BAGDE\n");
    }
}
