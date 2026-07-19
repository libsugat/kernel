#ifndef ARCH_IA32_INTERRUPT_H
#define ARCH_IA32_INTERRUPT_H

#ifndef __ASSEMBLER__

#include <kernel/types.h>

struct interrupt_frame {
    // Segment register
    uint32_t ds, es, fs, gs;

    // General purpose registers
    reg_t eax, ebx, ecx, edx, esi, edi, ebp;

    // Interrupt Info
    uint32_t vector_no;
    uint32_t error_code;

    // Hardware State
    uintptr_t eip;
    uint32_t cs;
    uint32_t eflags;

    // Hardware state if user_space, (should be used carefully)
    reg_t esp_usr;
    uint32_t ss_usr;
} __attribute__((__packed__));

inline __attribute__((always_inline)) void arch_enable_interrupts(void) {
    __asm__ __volatile__ ("sti");
}

inline __attribute__((always_inline)) void arch_disable_interrupts(void) {
    __asm__ __volatile__ ("cli");
}

#endif // __ASSEMBLER__
#endif // ARCH_IA32_INTERRUPT_H
