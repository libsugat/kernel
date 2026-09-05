#ifndef ACPI_MADT_H
#define ACPI_MADT_H

#ifndef __ASSEMBLER__

#include <kdef.h>

#include <kernel/types.h>
#include <kernel/bits.h>

#include <acpi/common.h>

struct madt {
    struct acpi_header header;
    uint32_t local_apic_header;
    uint32_t flags; // bit 0: PCAT_COMPAT (1 indicate system also has a PC-AT-compatible dual-8259 setup)
    
    uint8_t entries[]; // Written as a buffer, where you need explicit pointer arithmatic
} PACKED;

struct madt_entry_header {
    uint8_t type;
    uint8_t length;
} PACKED;

enum madt_entries_types {
    MADT_ENTRY_TYPE_LOCAL_APIC  = 0,
    MADT_ENTRY_TYPE_IO_APIC     = 1,
    MADT_ENTRY_TYPE_ISO         = 2,
    MADT_ENTRY_TYPE_NMI_SOURCE  = 3,
    MADT_ENTRY_TYPE_LAPIC_NMI   = 4,
    // More exist but will add when needed
};

struct madt_entry_lapic {
    struct madt_entry_header header;
    uint8_t apic_processor_uid;
    uint8_t apic_id;
    uint32_t flags;
} PACKED;

#define MADT_LAPIC_FLAGS_ENABLED_MASK        BIT(0)
#define MADT_LAPIC_FLAGS_ONLINE_CAPABLE_MASK BIT(1)

struct madt_entry_ioapic {
    struct madt_entry_header header;
    uint8_t ioapic_id;
    uint8_t __reserved;
    uint32_t ioapic_address;
    uint32_t gsi_base;          // GSI: Global system interrupt
} PACKED;


struct madt_entry_iso {
    struct madt_entry_header header;
    uint8_t bus;                    // 0 Constant, meaning ISA
    uint8_t source;                 // Bus-relative interrupt source (IRQ)
    uint32_t gsi;                   // The Global System Interrupt that this bus-relative 
                                    // interrupt source will signal.
    uint16_t flags;                 // MPS INTI flags
} PACKED;

#define MADT_ISO_POLARITY_MASK       GETMASK_U16(1, 0)
#define MADT_ISO_TRIGGER_MODE_MASK   GETMASK_U16(3, 1)

enum madt_iso_flags_polarity {
    MADT_ISO_POLARITY_CONFORMS = 0b00,
    MADT_ISO_POLARITY_ACTIVE_HIGH = 0b01,
    MADT_ISO_POLARITY_ACTIVE_LOW = 0b11,
};

enum madt_iso_flags_trigger_mode {
    MADT_ISO_TRIGGER_CONFORMS = 0b00,
    MADT_ISO_TRIGGER_ACTIVE_HIGH = 0b01,
    MADT_ISO_TRIGGER_ACTIVE_LOW = 0b11,
};

struct madt_entry_nmi_source {
    struct madt_entry_header header;
    uint16_t flags;                 // Same as MPS INTI flags
    uint32_t gsi;                   // The Global System Interrupt that this NMI will signal.
} PACKED;

struct madt_entry_lapic_nmi {
    struct madt_entry_header header;
    uint8_t apic_processor_uid;
    uint16_t flags;                 // MPS INTI flags
    uint8_t lapic_lint_num;         // Local APIC interrupt input LINTn to which NMI is connected.
} PACKED;


#endif // __ASSEMBLER__
#endif // ACPI_MADT_H
