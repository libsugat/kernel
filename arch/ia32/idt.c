#include "arch/interrupt.h"
#include <arch/idt.h>
#include <arch/apic.h>

#include <kernel/interrupts.h>
#include <kernel/types.h>
#include <lib/kprint.h>

#include <arch/apic.h>

// entry.S
extern idt_t idt_table;
extern struct idtr idt_desc;

extern struct local_irqc_ops lapic_ops;

// isr_stub.s
extern void (*isr_stub_list[256])(void);
void initialize_interrupts(void) {
    for (int i = 0; i < 256; i++) {
        set_int_gate(&idt_table, i, isr_stub_list[i]);
    }

    reg_t eax=0, ebx=0, ecx=0, edx = 0;
    __asm__ __volatile__("cpuid; shr $16, %%ebx"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(1), "c"(0)
            );
    __asm__ __volatile__(
        "cpuid\n\t"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1)
    );
    kprintf("CPUID leaf 1: %x %x %x %x\n", eax, ebx, ecx, edx);

    // Temperory Need clean up
    volatile struct lapic_mmio* lapic_mmio_ptr = (volatile struct lapic_mmio *)(0xfee00000);
    
    lapic_ops.init();

    // lapic_mmio_ptr->spurious_ivr.val = 0x1FF; // Set Spurious Interrupt Vector 
    // lapic_mmio_ptr->lvt_lint0.val |= (1 << 16); // Mask LINT0

    // This is for timer
    // 1. Set the Divide Configuration Register (e.g., divide by 16)
    lapic_mmio_ptr->timer_div_config.val = 0x3; 
    // 2. Set the LVT Timer Register: Periodic Mode (bit 17) + Vector 0x30
    lapic_mmio_ptr->lvt_timer.val = (1 << 17) | 0x20; 
    // 3. Set initial count to start the ticks
    lapic_mmio_ptr->timer_init_count.val = 10000000;


    // Select the IMCR register
    __asm__ __volatile__ ("outb %%al, %%dx" : : "a"((uint8_t)0x70), "d"((uint16_t)0x22));
    // Force all interrupts to go through the APIC bus directly
    __asm__ __volatile__ ("outb %%al, %%dx" : : "a"((uint8_t)0x01), "d"((uint16_t)0x23));

    // Hardcoded I/O APIC Registers for QEMU
    *(volatile uint32_t*)(0xFEC00000 + 0x00) = 0x13; // Select high 32-bits of IRQ 1 redirection entry
    *(volatile uint32_t*)(0xFEC00000 + 0x10) = 0x00; // Destination: CPU LAPIC ID 0

    *(volatile uint32_t*)(0xFEC00000 + 0x00) = 0x12; // Select low 32-bits of IRQ 1 redirection entry
    *(volatile uint32_t*)(0xFEC00000 + 0x10) = 0x21; // Vector 0x21, Unmasked, Physical, Fixed
    
    // uint32_t edx, eax;
    uint32_t msr_address = 0x1B; // IA32_APIC_BASE_MSR MSR
    // uint32_t msr_address = 0x100; // IA32_APIC_BASE_MSR_BSP MSR
    // uint32_t msr_address = 0x800; // IA32_APIC_BASE_MSR_ENABLE MSR
    __asm__ __volatile__(
            "rdmsr"
            : "=a"(eax),
              "=d"(edx)
            : "c"(msr_address)
            : "memory"
        );
    kprintf("MSR: %x :: %x:%x\n", msr_address, edx, eax);

    kprintf("Local APIC Version: %x\n", lapic_mmio_ptr->version.val);

    // Enable interrupts
    arch_enable_interrupts();
}


void higher_irq_handler(struct interrupt_frame * intr_frame) {
    // if (intr_frame->vector_no != 0x20) {
    kprintf("hardware irq: %x\n", intr_frame->vector_no);
    // }

    // send eoi to local apic
    // Currently just a glue for allowing
    // testing hardware interrupts
    volatile uint32_t* lapic_eoi = (uint32_t*)(0xfee00000 + 0xb0);
    *lapic_eoi = 0;
}

void master_fault_handler(struct interrupt_frame * intr_frame) {
    kprintf("Interrupt: %x; Error Code: %x; Flags: %x\n", 
            intr_frame->vector_no,
            intr_frame->error_code,
            intr_frame->eflags
           );

    switch (intr_frame->vector_no) {
        case 0x0D:
            kputs("========================\n");
            kputs("General Protection Fault\n");
            kputs("========================\n");
            __asm__ __volatile__("hlt");
            break;
        default: ;
    };
}

void reserved_intr_handler(struct interrupt_frame* intr_frame) {
    kprintf("Panic : Reserved Interrupt Occurred!..\n");
    kprintf("\n--- INTERRUPT FRAME DUMP ---\n");

    // Segment Registers
    kprintf("ds:  0x%x  es: 0x%x\n", intr_frame->ds, intr_frame->es);
    kprintf("fs:  0x%x  gs: 0x%x\n", intr_frame->fs, intr_frame->gs);
    kprintf("----------------------------\n");

    // General Purpose Registers
    kprintf("eax: 0x%x  ebx: 0x%x\n", intr_frame->eax, intr_frame->ebx);
    kprintf("ecx: 0x%x  edx: 0x%x\n", intr_frame->ecx, intr_frame->edx);
    kprintf("esi: 0x%x  edi: 0x%x\n", intr_frame->esi, intr_frame->edi);
    kprintf("ebp: 0x%x\n", intr_frame->ebp);
    kprintf("----------------------------\n");

    // Interrupt Info
    kprintf("vec: 0x%x\n", intr_frame->vector_no);
    kprintf("err: 0x%x\n", intr_frame->error_code);
    kprintf("----------------------------\n");

    // Hardware State
    kprintf("eip: 0x%x\n", intr_frame->eip);
    kprintf("cs:  0x%x\n", intr_frame->cs);
    kprintf("efl: 0x%x\n", intr_frame->eflags);

    // Check bottom 2 bits of CS (RPL - Requested Privilege Level)
    // If lower 2 bits equal 3, the interrupt originated from user space
    if ((intr_frame->cs & 0x3) == 3) {
        kprintf("----------------------------\n");
        kprintf("usr_esp: 0x%x\n", intr_frame->esp_usr);
        kprintf("usr_ss:  0x%x\n", intr_frame->ss_usr);
    }
    
    kprintf("----------------------------\n\n");

    arch_wait_for_interrupt();
}


