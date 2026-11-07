#include <asm/msr.h>
#include <asm/msr-index.h>

#include <arch/idt.h>
#include <arch/apic.h>

#include <kernel/interrupts.h>
#include <kernel/bits.h>
#include <kernel/sched.h>
#include <lib/kprint.h>

#include <arch/apic.h>

extern idt_t idt_table;      // entry.S
extern struct idtr idt_desc; // entry.S

extern struct local_irqc_ops lapic_ops; // apic.c

// isr_stub.s
extern void (*isr_stub_list[256])(void); // Do not touch

void initialize_interrupts(void) {
    for (int i = 0; i < 256; i++) {
        set_int_gate(&idt_table, i, isr_stub_list[i]);
    }

    reg_t eax=0, ebx=0, ecx=0, edx = 0;
    __asm__ __volatile__(
        "cpuid\n\t"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1)
    );
    kprintf("CPUID leaf 1: %x %x %x %x\n", eax, ebx, ecx, edx);

    if (!(edx & 0x200)) {
        kputs("APIC NOT AVAILABLE..\n");
        arch_disable_interrupts();
        arch_wait_for_interrupt();
    }

    if (!lapic_ops.init) {
        arch_disable_interrupts();
        arch_wait_for_interrupt();
    }
    lapic_ops.init();

    // This is for timer
    volatile struct lapic_mmio* lapic_mmio_ptr = get_lapic_mmio_ptr();
    lapic_mmio_ptr->timer_div_config.val = 0x3;       // 1. Set the Divide Configuration Register (e.g., divide by 16)

    // 2. Set the LVT Timer Register: Periodic Mode (bit 17) + Vector 32
    uint32_t timer_lvt = lapic_mmio_ptr->lvt_timer.val;
    FIELD_ASSIGN(timer_lvt, APIC_LVT_VECTOR_MASK,       0x20);                     // Set vector to 32 
    FIELD_ASSIGN(timer_lvt, APIC_LVT_TIMER_MODE_MASK,   APIC_TIMER_MODE_PERIODIC); // timer mode periodic
    FIELD_ASSIGN(timer_lvt, APIC_LVT_MASKED_MASK,       APIC_MASKING_UNMASKED);    // unmask the interrupt
    lapic_mmio_ptr->lvt_timer.val = timer_lvt;
    lapic_mmio_ptr->timer_init_count.val = 10000000;  // 3. Set initial count to start the ticks

    // Hardcoded I/O APIC Registers for QEMU
    // Glue for keyboard
    struct io_apic apic = {
        .phy_base = 0xFEC00000,
        .id = 0,
        .gsi_base = 0
    };
    write_io_apic(&apic, 0x13, 0x00); // Destination: CPU LAPIC ID 0
    write_io_apic(&apic, 0x12, 0x21); // Vector 0x21, Unmasked, Physical, Fixed
}


void higher_irq_handler(struct interrupt_frame * intr_frame) {
    if (intr_frame->vector_no == 0xFF) {
        kputs("Spurious Interrupt...\n");
        return; // Return immediately WITHOUT sending EOI
    }

    lapic_ops.eoi(intr_frame->vector_no);
    // Glue code, i dont want vga to clutter now
    // Hence do not show timer
    if (intr_frame->vector_no == 32) {
        sched();
    }
    else {
        kprintf("hardware irq: %u\n", intr_frame->vector_no);
    }
}

void master_fault_handler(struct interrupt_frame * intr_frame) {
    kprintf("Interrupt: %x; Error Code: %x; Flags: %x\n", 
            intr_frame->vector_no,
            intr_frame->error_code,
            intr_frame->eflags
           );

    switch (intr_frame->vector_no) {
        case 0x0D:
            kputs("==========================\n");
            kputs(" General Protection Fault\n");
            kputs("==========================\n");
            arch_wait_for_interrupt();
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
