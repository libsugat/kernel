#ifndef ARCH_IA32_APIC_H
#define ARCH_IA32_APIC_H

#ifndef __ASSEMBLER__

#include <kernel/types.h>
#include <kdef.h>

/* LAPIC register: 32-bit value padded to a 16-byte offset */
struct lapic_reg {
    volatile uint32_t val;
    u32 __padding[3];
} ALIGNED(16);

/* MMIO Register Layout (Base: 0xFEE00000, Size: 0x400) */
struct lapic_mmio {
    struct lapic_reg __reserved0[2];  // 0x000
    struct lapic_reg id;              // 0x020: LAPIC ID
    struct lapic_reg version;         // 0x030: Version

    struct lapic_reg __reserved1[4];  // 0x040

    struct lapic_reg tpr;             // 0x080: Task Priority
    struct lapic_reg apr;             // 0x090: Arb Priority
    struct lapic_reg ppr;             // 0x0A0: Proc Priority

    struct lapic_reg eoi;             // 0x0B0: End of Interrupt
    struct lapic_reg rrd;             // 0x0C0: Remote Read
    struct lapic_reg ldr;             // 0x0D0: Logical Dest
    struct lapic_reg dfr;             // 0x0E0: Dest Format
    struct lapic_reg spurious_ivr;    // 0x0F0: Spurious Vector

    struct lapic_reg isr[8];          // 0x100: In-Service
    struct lapic_reg tmr[8];          // 0x180: Trigger Mode
    struct lapic_reg irr[8];          // 0x200: Interrupt Request

    struct lapic_reg error_status_reg;// 0x280: Error Status

    struct lapic_reg __reserved2[6];  // 0x290

    struct lapic_reg lvt_cmci;        // 0x2F0: LVT CMCI
    struct lapic_reg icr[2];          // 0x300: ICR (Low / High)

    struct lapic_reg lvt_timer;       // 0x320: LVT Timer
    struct lapic_reg lvt_tsensr;      // 0x330: LVT Thermal
    struct lapic_reg lvt_perfmc;      // 0x340: LVT Perf
    struct lapic_reg lvt_lint0;       // 0x350: LVT LINT0
    struct lapic_reg lvt_lint1;       // 0x360: LVT LINT1
    struct lapic_reg lvt_err;         // 0x370: LVT Error

    struct lapic_reg timer_init_count;// 0x380: Timer Init
    struct lapic_reg timer_curr_count;// 0x390: Timer Current

    struct lapic_reg __reserved3[4];  // 0x3A0

    struct lapic_reg timer_div_config;// 0x3E0: Timer Divider
    struct lapic_reg __reserved4[1];  // 0x3F0
} ALIGNED(16);

#endif // __ASSEMBLER__
#endif // ARCH_IA32_APIC_H
