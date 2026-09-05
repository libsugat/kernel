#include <arch/init.h>
#include <arch/apic.h>

#include <acpi/common.h>
#include <acpi/madt.h>
#include <acpi/fadt.h>

#include <kernel/interrupts.h>
#include <kernel/types.h>

#include <lib/kprint.h>

static void handle_fadt(struct fadt * fadt_ptr) {
    kprintf("\t\tPreferred PM Profile : %x\n", fadt_ptr->preferred_pm_profile);
    kprintf("\t\tIAPC BOOT ARCH : %x\n", fadt_ptr->arm_boot_architecture_flags);
    kprintf("\t\tDSDT Address : %x\n", fadt_ptr->dsdt);
}

static void handle_madt(struct madt * madt_ptr) {

    // Time to traverse the madt table entires
    // uint8_t * entry_ptr = &madt_ptr->entries[0];
    uint8_t *entry_ptr = madt_ptr->entries;
    uint8_t *entry_end = (uint8_t *)madt_ptr + madt_ptr->header.length;

    while (entry_ptr < entry_end) {
        struct madt_entry_header *head = (struct madt_entry_header *) entry_ptr;

        if (head->length < sizeof(struct madt_entry_header) 
                || entry_ptr + head->length > entry_end) {
            break;
        }
        
        switch (head->type) {
            case MADT_ENTRY_TYPE_LOCAL_APIC: {
                struct madt_entry_lapic *lapic_e = (struct madt_entry_lapic *)entry_ptr;
                kprintf("lapic:: apic_p_uid: %x; id: %x; flags: %x\n", 
                        lapic_e->apic_processor_uid, lapic_e->apic_id, lapic_e->flags);
                break;
            }
            case MADT_ENTRY_TYPE_IO_APIC: {
                struct madt_entry_ioapic *ioapic_e = (struct madt_entry_ioapic *)entry_ptr;
                kprintf("ioapic:: id: %x; address : %x; gsi_base: %u\n", 
                        ioapic_e->ioapic_id, ioapic_e->ioapic_address, ioapic_e->gsi_base);
                if (register_ioapic(ioapic_e->ioapic_id,
                        ioapic_e->ioapic_address, ioapic_e->gsi_base)) {
                    kprintf("ioapic reg failure:: id: %x; address : %x; gsi_base: %x\n", 
                        ioapic_e->ioapic_id, ioapic_e->ioapic_address, ioapic_e->gsi_base);
                }
                else {
                    kputs("APIC registration Success\n");
                }
                break;
            }
            case MADT_ENTRY_TYPE_ISO: {
                struct madt_entry_iso *iso_e = (struct madt_entry_iso *)entry_ptr;
                kprintf("iso:: bus: %x; source : %u; gsi: %u; flags: %x\n", 
                        iso_e->bus, iso_e->source, iso_e->gsi, iso_e->flags);
                break;
            }
            case MADT_ENTRY_TYPE_NMI_SOURCE: {
                struct madt_entry_nmi_source *nmi_src_e = (struct madt_entry_nmi_source *)entry_ptr;
                kprintf("nmi_src:: gsi: %x; flags: %x\n", 
                        nmi_src_e->gsi, nmi_src_e->flags);
                break;
            }
            case MADT_ENTRY_TYPE_LAPIC_NMI: {
                struct madt_entry_lapic_nmi *lapic_nmi_e = (struct madt_entry_lapic_nmi *)entry_ptr;
                kprintf("lapic_nmi:: apic_p_uid: %x; flags: %x; LINT#: %x\n", 
                        lapic_nmi_e->apic_processor_uid, lapic_nmi_e->flags, lapic_nmi_e->lapic_lint_num);
                break;
            }
            default:;
        }

        entry_ptr += head->length;
    }
}

void parse_acpi(struct rsdp *found_rsdp)
{
    // RSDP verification
    uint8_t * rsdp_ptr = (uint8_t *)found_rsdp;
    uint32_t sum = 0;
    for (size_t i = 0; i < 20; i++) {
        sum += rsdp_ptr[i];
    }
    if (sum % 256 == 0) {
        kputs("RSDP pointer valid\n");
    }
    else {
        // TODO: KERNEL PANIC
        kputs("RSDP pointer invalid... :(\n");
        arch_disable_interrupts();
        arch_wait_for_interrupt();
    }

    struct rsdp rsdp = *found_rsdp;

    // Some loging, again glue
    kprintf("RSDP revivion: %x\n", rsdp.revision);
    kputs("RSDP oemid : ");
    kputsn(rsdp.oemid, 6);
    kprintf("\n");

    size_t rsdt_entries = rsdp.rsdt->header.length - sizeof(struct acpi_header);
    rsdt_entries /= sizeof(reg_t);
    kprintf("RSDT entries: %x\n\n", rsdt_entries);

    kputs("Tables found: \n");
    for (size_t i = 0; i < rsdt_entries; i++) {
        struct acpi_header * entry_header = (struct acpi_header *)rsdp.rsdt->sdt_pointers[i];
        kputs("\t");
        kputsn(entry_header->signature.str, 4);
        kprintf(" at %x (sign : %x)\n", entry_header, entry_header->signature.val);

        switch (entry_header->signature.val)
        {
            case ACPI_SIG_MADT:
                handle_madt((struct madt *)entry_header);
                break;
            case ACPI_SIG_FADT:
                handle_fadt((struct fadt *)entry_header);
                break;
            default:
                kputs("\t\tI dont know how to handle ");
                kputsn(entry_header->signature.str, 4);
                kputs(" :(\n");
        }
    }
    kprintf("\n");
}
