#include <kdef.h>

#include <kernel/interrupts.h>
#include <kernel/bits.h>

#include <asm/msr.h>
#include <asm/msr-index.h>
#include <arch/idt.h>
#include <arch/apic.h>

#include <lib/kprint.h>

static volatile struct lapic_mmio * lapic_mmio_ptr;

volatile struct lapic_mmio * get_lapic_mmio_ptr(void) {
    return lapic_mmio_ptr;
}

static void init_lapic(void) {
    // Check lapic capabilities
    // TODO

    // Enable lapic
    u64 msr_data = rdmsr(MSR_IA32_APIC_BASE) | 0x800;
    wrmsr(0x1b, msr_data);
    
    u32 lapic_mmio_base = (u32) msr_data;
    lapic_mmio_base &= (~0xFFF);
    lapic_mmio_ptr = 
            (volatile struct lapic_mmio *) lapic_mmio_base;

    // Disable Lint0 i.e legacy iqr controllers
    // lapic_mmio_ptr->lvt_lint0.val |= BIT(16); // Mask off lint0
    FIELD_ASSIGN(lapic_mmio_ptr->lvt_lint0.val, APIC_LVT_MASKED_MASK, APIC_MASKING_MASKED);
    // Setup Spurios Register
    lapic_mmio_ptr->spurious_ivr.val = 0x1FF;   // Set Spurious Interrupt Vector 

    // mics testings
    kprintf("\nAPIC version      : %x\n",
            FIELD_GET(lapic_mmio_ptr->version.val, APIC_VERSION_VERSION_MASK));
    kprintf("     max lvt entry: %x\n",
            FIELD_GET(lapic_mmio_ptr->version.val, APIC_VERSION_MAXLVTE_MASK));
    kprintf("     eoi broadcast: %x\n\n",
            FIELD_GET(lapic_mmio_ptr->version.val, APIC_VERSION_EOI_BROADCAST_MASK));
}

static void eoi_lapic(u32 UNUSED _vec) {
    // (void)_vec; // Signal unused to compiler; can also use UNUSED from kdef.h
    lapic_mmio_ptr->eoi.val = 0;
}

const struct local_irqc_ops lapic_ops = {
    .init = init_lapic,
    .eoi = eoi_lapic,
    .mask = NULL,
    .unmask = NULL
};

// IO APIC
#define MAX_IOAPIC 64

static struct io_apic ioapics[MAX_IOAPIC];
static uint32_t ioapic_count = 0;

static void print_ioapic(struct io_apic * apic) {
    kprintf("IOAPIC:: id: %x; version: %x; max: %u;\n"
            "         addr: %x; gsi_start: %u; gsi_end: %u\n",
            apic->id, apic->version, apic->max_entries,
            apic->phy_base, apic->gsi_base, apic->gsi_end
           );
}


int register_ioapic(uint8_t id, uintptr_t phys_base, uint32_t gsi_base) {
    if (!phys_base) return -IOAPIC_REGISTRATION_ERROR;
    if (ioapic_count >= MAX_IOAPIC) return -IOAPIC_REGISTRATION_ERROR;

    struct io_apic *apic = &ioapics[ioapic_count];
    // Check if already registered
    for (size_t i = 0; i < ioapic_count; i ++) {
        if (ioapics[i].phy_base == phys_base) {
            kprintf("address 0x%x conflicts with IOAPIC%d\n", phys_base, i);
            return -IOAPIC_REGISTRATION_EEXIST;
        }
    }

    // apic->virt_base = mmio_map(phys_base, PAGE_SIZE);
    // if (!apic->virt_base) return IOAPIC_REGISTRATION_ERROR;
    uint32_t id_reg = read_io_apic(apic, IOAPIC_REG_IOAPICID);
    FIELD_ASSIGN(id_reg, IOAPIC_IOAPICID_ID_MASK, 1);
    write_io_apic(apic, IOAPIC_REG_IOAPICID, id_reg);

    apic->phy_base = phys_base;
    apic->gsi_base = gsi_base;

    // TODO : Verify IOAPIC base

    uint32_t ver_reg = read_io_apic(apic, IOAPIC_REG_IOAPICVER);
    uint8_t max_redirection_entries = FIELD_GET(ver_reg, IOAPIC_IOAPICVER_MAX_ENTRIES_MASK);

    apic->id = id;
    apic->version = FIELD_GET(ver_reg, IOAPIC_IOAPICVER_VERSION_MASK);
    apic->max_entries = max_redirection_entries + 1;
    apic->gsi_end = gsi_base + max_redirection_entries;

    ioapic_count++;
    print_ioapic(apic);
    return IOAPIC_REGISTRATION_SUCCESS;
}
