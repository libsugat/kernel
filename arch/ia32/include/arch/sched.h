#ifndef ARCH_SCHED_H
#define ARCH_SCHED_H

#include <kdef.h>
#ifndef __ASSEMBLER__

#include <kernel/types.h>

struct task_struct {
    reg_t esp;
    uint8_t * kstack;
};

struct task_struct _create_new_task(void (*entry)(void), uint8_t * stack);

// From entry.S
extern void switch_to(struct task_struct* curr, struct task_struct *next);

ALWAYS_INLINE uint32_t get_stack_pointer() {
    uint32_t esp;
    __asm__ __volatile__ (
            "movl %%esp, %0"
            : "=r"(esp)
            :
            :
            );
    return esp;
}

void arch_init_sched(struct task_struct* idle_task);

#endif // __ASSEMBLER__

#endif // ARCH_SCHED_H
