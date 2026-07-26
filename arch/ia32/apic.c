#include <asm/msr.h>
#include <kernel/interrupts.h>

#include <arch/idt.h>
#include <arch/apic.h>

static void init_lapic(void) {
    // Check init_lapic capabilities

    // Enable lapic
    // reg_t eax=0, edx=0;
    // __asm__ __volatile__("rdmsr":"=a"(eax), "=d"(edx) :"c"(0x1b));
    // eax |= 0x800;
    // __asm__ __volatile__("wrmsr"::"a"(eax), "d"(edx));
    u64 msr_data = rdmsr(0x1b) | 0x800;
    wrmsr(0x1b, msr_data);
    
    volatile struct lapic_mmio * lapic_mmio_ptr = (volatile struct lapic_mmio *)0xFEE00000;

    // Disable Lint0 i.e legacy iqr controllers
    lapic_mmio_ptr->lvt_lint0.val |= (1 << 16); // Mask off lint0
    // Setup Spurios Register
    lapic_mmio_ptr->spurious_ivr.val = 0x1FF;   // Set Spurious Interrupt Vector 
}

struct local_irqc_ops lapic_ops = {
    .init = init_lapic,
    .eoi = NULL,
    .mask = NULL,
    .unmask = NULL
};
