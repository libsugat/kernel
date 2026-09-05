#ifndef ARCH_IA32_APIC_H
#define ARCH_IA32_APIC_H

#ifndef __ASSEMBLER__

#include <kernel/types.h>
#include <kernel/bits.h>
#include <kdef.h>

/* LAPIC register: 32-bit value padded to a 16-byte offset */
struct lapic_reg {
    volatile uint32_t val;
    u32 __padding[3];
} ALIGNED(16);

/* MMIO Register Layout (Base: generally 0xFEE00000, Size: 0x1000) */
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

#define APIC_VERSION_VERSION_MASK       GETMASK(7, 0)
#define APIC_VERSION_MAXLVTE_MASK       GETMASK(23, 16)
#define APIC_VERSION_EOI_BROADCAST_MASK BIT(24)

#define APIC_LVT_VECTOR_MASK            GETMASK(7, 0)
#define APIC_LVT_DELIVERY_MODE_MASK     GETMASK(10, 8)
#define APIC_LVT_DELIVERY_STATUS_MASK   BIT(12)
#define APIC_LVT_INTPOLARITY_MAS        BIT(13)
#define APIC_LVT_REMOTE_IRR_MASK        BIT(14)
#define APIC_LVT_TRIGGER_MODE_MASK      BIT(15)
#define APIC_LVT_MASKED_MASK            BIT(16)

#define APIC_LVT_TIMER_MODE_MASK        GETMASK(18, 17)

enum apic_delivery_modes {
    APIC_DELIVERY_MODE_FIXED        = 0b000,
    APIC_DELIVERY_MODE_LOWESTPRIO   = 0b001,
    APIC_DELIVERY_MODE_SMI          = 0b010,
    APIC_DELIVERY_MODE_NMI          = 0b100,
    APIC_DELIVERY_MODE_INIT         = 0b101,
    APIC_DELIVERY_MODE_EXTINT       = 0b111,
};

enum apic_delivery_status {
    APIC_DELIVERY_STATUS_IDLE       = 0, /* No interrupt transmission in progress */
    APIC_DELIVERY_STATUS_PENDING    = 1, /* Interrupt sent, waiting for target APIC acceptance */
};

enum apic_pin_polarity {
    APIC_POLARITY_HIGH = 0,
    APIC_POLARITY_LOW  = 1,
};

enum apic_remote_irr {
    APIC_REMOTE_IRR_INACTIVE = 0, /* EOI received or no interrupt pending */
    APIC_REMOTE_IRR_ACTIVE   = 1, /* Interrupt accepted by Local APIC, awaiting EOI */
};

enum apic_trigger_mode {
    APIC_TRIGGER_MODE_EDGE      = 0,
    APIC_TRIGGER_MODE_LEVEL     = 1,
};

enum apic_masking {
    APIC_MASKING_UNMASKED = 0,
    APIC_MASKING_MASKED   = 1,
};

enum apic_timer_mode {
    APIC_TIMER_MODE_ONE_SHOT = 0b00,
    APIC_TIMER_MODE_PERIODIC = 0b01,
    APIC_TIMER_MODE_TSC_DEADLINE = 0b10,
};

volatile struct lapic_mmio *get_lapic_mmio_ptr(void);

/*
================================
            I/O APIC
================================
*/

struct io_apic {
    uint8_t id;
    uint8_t version;

    uint8_t max_entries;

    uintptr_t phy_base;            /* Physical base address from MADT */
    volatile uint32_t *virt_base;

    uint32_t gsi_base;              /* First GSI assigned to this IO-APIC */
    uint32_t gsi_end;               /* Last GSI (gsi_base + max_entries) */

    // spin_lock
};

#define IOAPIC_REG_IOAPICID                   0x00
#define IOAPIC_REG_IOAPICVER                  0x01
#define IOAPIC_REG_IOAPICARB                  0x02

#define IOAPIC_IOAPICID_ID_MASK             GETMASK(27, 24)

#define IOAPIC_IOAPICVER_VERSION_MASK       GETMASK(7, 0)
#define IOAPIC_IOAPICVER_MAX_ENTRIES_MASK   GETMASK(23, 16)

#define IOREGSEL_PTR(base) ((volatile uint32_t *)((uintptr_t)(base) + 0x00))
#define IOREGWIN_PTR(base) ((volatile uint32_t *)((uintptr_t)(base) + 0x10))

ALWAYS_INLINE uint32_t read_io_apic(struct io_apic * ioapic, const uint32_t reg) {
    *IOREGSEL_PTR(ioapic->phy_base) = reg;
    return *IOREGWIN_PTR(ioapic->phy_base);
}

ALWAYS_INLINE void write_io_apic(struct io_apic * ioapic, const uint32_t reg, uint32_t val) {
    *IOREGSEL_PTR(ioapic->phy_base) = reg;
    *IOREGWIN_PTR(ioapic->phy_base) = val;
}

// Replace with a proper driver system
#define IOAPIC_REGISTRATION_SUCCESS         0
#define IOAPIC_REGISTRATION_ERROR           1
#define IOAPIC_REGISTRATION_EEXIST          2

int register_ioapic(uint8_t id, uintptr_t base, uint32_t gsi_base);

#endif // __ASSEMBLER__
#endif // ARCH_IA32_APIC_H
