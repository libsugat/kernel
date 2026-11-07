#include <kdef.h>

#include <kernel/sched.h>
#include <kernel/mm.h>
#include <kernel/mem_symbols.h>
#include <kernel/interrupts.h>

void arch_init_sched(struct task_struct * idle) {
    uintptr_t stack = (uintptr_t)kstack_bottom;
    idle->kstack = (uint8_t *)stack;
}

void task_trampoline(void (*entry)(void)) {
    arch_enable_interrupts();

    if (entry) entry();

    for (;;) {
        arch_wait_for_interrupt();
    }
}

struct task_struct _create_new_task(void (*entry)(void), uint8_t *stack) {
    struct task_struct new_task;
    new_task.kstack = stack;
    uint32_t * sp = (uint32_t *)((uintptr_t)stack + PAGE_SIZE);
    *(--sp) = (uintptr_t)entry;
    *(--sp) = 0;
    *(--sp) = (uintptr_t)task_trampoline;
    *(--sp) = 0x00000202;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    new_task.esp = (reg_t)sp;

    return new_task;
}
