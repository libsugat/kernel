#include "arch/interrupt.h"
#include <arch/idt.h>

#include <kernel/interrupts.h>
#include <kernel/types.h>
#include <lib/kprint.h>

// entry.S
extern idt_t idt_table;
extern struct idtr idt_desc;

// isr_stub.s
extern void (*isr_stub_list[256])(void);
void initialize_interrupts(void) {
    for (int i = 0; i < 256; i++) {
        set_int_gate(&idt_table, i, isr_stub_list[i]);
    }

    *(volatile uint32_t*)(0xFEE00000 + 0xF0) = 0x1FF; // Set Spurious Interrupt Vector 

    // Temperory Testing setup timer its a copy paste
    volatile uint32_t* lapic_timer      = (uint32_t*)(0xFEE00000 + 0x320);
    volatile uint32_t* lapic_timer_dcr  = (uint32_t*)(0xFEE00000 + 0x3E0);
    volatile uint32_t* lapic_timer_init = (uint32_t*)(0xFEE00000 + 0x380);

    // 1. Set the Divide Configuration Register (e.g., divide by 16)
    *lapic_timer_dcr = 0x3; 

    // 2. Set the LVT Timer Register: Periodic Mode (bit 17) + Vector 0x30
    *lapic_timer = (1 << 17) | 0x20; 

    // 3. Set initial count to start the ticks
    *lapic_timer_init = 10000000;

    // Select the IMCR register
    __asm__ __volatile__ ("outb %%al, %%dx" : : "a"((uint8_t)0x70), "d"((uint16_t)0x22));
    // Force all interrupts to go through the APIC bus directly
    __asm__ __volatile__ ("outb %%al, %%dx" : : "a"((uint8_t)0x01), "d"((uint16_t)0x23));

    // Hardcoded I/O APIC Registers for QEMU
    *(volatile uint32_t*)(0xFEC00000 + 0x00) = 0x13; // Select high 32-bits of IRQ 1 redirection entry
    *(volatile uint32_t*)(0xFEC00000 + 0x10) = 0x00; // Destination: CPU LAPIC ID 0

    *(volatile uint32_t*)(0xFEC00000 + 0x00) = 0x12; // Select low 32-bits of IRQ 1 redirection entry
    *(volatile uint32_t*)(0xFEC00000 + 0x10) = 0x21; // Vector 0x21, Unmasked, Physical, Fixed
    
    uint32_t edx, eax;
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
    kputs("Local APIC Version: ");
    kprint_hex(*(volatile uint32_t*)(0xFEE00000 + 0x30));
    kputchar('\n');

    // Enable interrupts
    arch_enable_interrupts();
}


void higher_irq_handler(struct interrupt_frame * intr_frame) {
    kprintf("hardware irq: %x\n", intr_frame->vector_no);

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
}


