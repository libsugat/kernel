#include <arch/interrupt.h>
#include <arch/types.h>
#include <arch/mm.h>

#include <kernel/bits.h>
#include <kernel/boot_info.h>
#include <kernel/mem_symbols.h>

#include <lib/kprint.h>

/**
 * Standard E820 / ACPI Memory Type Codes
 */
enum e820_type {
    /* Standard Legacy & ACPI 1.0+ Types */
    E820_TYPE_RAM           = 1,   /* Usable system memory / RAM */
    E820_TYPE_RESERVED      = 2,   /* Reserved / MMIO / Firmware / Devices */
    E820_TYPE_ACPI          = 3,   /* ACPI Reclaimable tables (RSDP, MADT, etc.) */
    E820_TYPE_NVS           = 4,   /* ACPI Non-Volatile Storage (preserved across S3/S4) */
    E820_TYPE_UNUSABLE      = 5,   /* Defective / Bad RAM blocks */

    /* Modern & Extended Types (ACPI 6.0+ / Virtualization) */
    E820_TYPE_PMEM          = 6,   /* Persistent Memory (NVDIMM) */
    E820_TYPE_UNACCEPTED    = 7,   /* Unaccepted Memory (Intel TDX / AMD SEV-SNP) */
    
    /* Kernel / Vendor Specific */
    E820_TYPE_PRAM          = 12,  /* Persistent RAM (reboot-safe kexec/pstore) */
    E820_TYPE_RESERVED_KERN = 0xDEADBEEF   /* Kernel internal early reservation */
};

static inline int e820_cmp(const struct e820_entry *a, const struct e820_entry *b) {
    if (a->base_address < b->base_address) return -1;
    if (a->base_address > b->base_address) return 1;
    if (a->length < b->length) return -1;
    if (a->length > b->length) return 1;
    return 0;
}

static void sort_e820_insertion(struct e820_entry *entries, size_t count) {
    if (count == 0) return;
    for (size_t i = 1; i < count; i++) {
        struct e820_entry key = entries[i];
        size_t j = i;

        while (j > 0 && e820_cmp(&entries[j - 1], &key) > 0) {
            entries[j] = entries[j - 1];
            j--;
        }
        entries[j] = key;
    }
}

#define SANITIZATION_ERROR 0xDEADBEEF
static size_t santize_e820_inplace(struct e820_entry *entries, size_t count){
    // Replace the whole function with a good tested algorithm
    if (count <= 0) return SANITIZATION_ERROR;
    
    // Perform page alignment
    for (size_t i = 0; i < count; i++) {
        uint64_t base = entries[i].base_address;
        uint64_t end = base + entries[i].length;

        switch(entries[i].type)
        {
            case E820_TYPE_RAM:
                if (!IS_PAGE_AIGNED(base)) {
                    // mov to next alligned address
                    base = FIELD_MODIFY(base, PAGE_SIZE - 1, 0); 
                    base += PAGE_SIZE;
                }
                if (!IS_PAGE_AIGNED(end)) {
                    // mov to previous alligned address
                    end = FIELD_MODIFY(end, PAGE_SIZE - 1, 0); 
                }
                break;
            default:
                if (!IS_PAGE_AIGNED(base)) {
                    // mov to previous alligned address
                    base = FIELD_MODIFY(base, PAGE_SIZE - 1, 0); 
                }
                if (!IS_PAGE_AIGNED(end)) {
                    // mov to next alligned address
                    end = FIELD_MODIFY(end, PAGE_SIZE - 1, 0); 
                    end += PAGE_SIZE;
                }
        }

        entries[i].base_address = base;
        entries[i].length = end - base;
    }

    sort_e820_insertion(entries, count); // table can be unsorted too
    size_t write_idx = 0;
    for (size_t i = 1; i < count; i++) {
        uint64_t prev_end = entries[write_idx].base_address + entries[write_idx].length;
        uint64_t curr_base = entries[i].base_address;
        uint64_t curr_end = curr_base + entries[i].length;

        // Merge only if same type AND overlapping/adjacent
        if (entries[write_idx].type == entries[i].type && curr_base <= prev_end) {
            if (curr_end > prev_end) {
                entries[write_idx].length = curr_end - entries[write_idx].base_address;
            }
            // If curr_end <= prev_end, entry[i] is completely subsumed; length stays the same
        } else {
            write_idx++;
            entries[write_idx] = entries[i];
        }
    }
    count = write_idx + 1;
    sort_e820_insertion(entries, count);

    // Fix overlapping types that are of different types
    // by breaking them
    write_idx = count;
    for (size_t i = 1; i < count; i++) {
        size_t prev = i - 1;
        uint64_t prev_start = entries[prev].base_address;
        uint64_t prev_end = entries[prev].base_address + entries[prev].length - 1;
        uint64_t curr_start = entries[i].base_address;
        uint64_t curr_end = entries[i].base_address + entries[i].length - 1;

        // Completely inside
        if (prev_start > curr_start && prev_end < curr_end) {
            struct e820_entry new_ent1, new_ent3;
            new_ent1 = entries[prev];
            new_ent1.length = curr_start - prev_start;

            // new_ent2 = entries[i]; 
            new_ent3 = entries[prev];
            new_ent3.base_address = curr_end + 1;
            new_ent3.length = prev_end - curr_end;

            entries[prev] = new_ent1;
            // entries[i] = new_ent2;
            entries[write_idx] = new_ent3;
            write_idx++;
        } else if (prev_start == curr_start) {
            // previous is smaller and current is larger
            // also its prev is smaller than curr, thank to sort
            entries[i].base_address = prev_end + 1;
            entries[i].length = entries[i].length - entries[prev].length;
        }
        else if (prev_end == curr_end) {
            entries[prev].length = entries[prev].length - entries[i].length;
        }
        // TODO: 2 more edge, i.e of type [1 [2 ]1 ]2
    }
    count = write_idx;

    // Sort
    sort_e820_insertion(entries, count);
    return count;
}

void arch_init_mm(struct boot_info_struct *boot_info_ptr)
{
    struct e820_entry * e820t = boot_info_ptr->e820_table;
    size_t entries = boot_info_ptr->e820_entries;

    kprintf("Bootloader found has %d e820 entries\n", boot_info_ptr->e820_entries);
    kprintf("Kernel starts at %x end at %x\n", (reg_t)__k_phy_start, (reg_t)__k_phy_end);

    // Add kernel reserved
    struct e820_entry kernel_entry;
    kernel_entry.base_address = (uint64_t)(uintptr_t)__k_phy_start;
    kernel_entry.length       = (uint64_t)(ptrdiff_t)(__k_phy_end - __k_phy_start);
    kernel_entry.type = E820_TYPE_RESERVED_KERN;                                 // Not decided signature yet 
    kernel_entry.extended_attributes = 1;
    e820t[entries++] = kernel_entry;

    size_t u_entries = santize_e820_inplace(e820t, entries);
    if (u_entries == SANITIZATION_ERROR) {
        kputs("Panci.....");
        for (;;) {
            arch_disable_interrupts();
            arch_wait_for_interrupt();
        }
    }
    kprintf("Sanatized to %d e820 entries\n", u_entries);

    kputs("No| Base addr      "
                "| Till Addr      "
                "| Length         "
                "| Type   "
                "| Ext FLg|\n"
                "--|----------------|----------------"
                "|----------------|--------|--------+\n");

    for (size_t i = 0; i < u_entries; i++) {
        kprintf("%d |", i);
        kprint_hex64(e820t[i].base_address);
        kputc('|');
        kprint_hex64(e820t[i].base_address + e820t[i].length - 1);
        kputc('|');
        kprint_hex64(e820t[i].length);
        kputc('|');
        kprint_hex32(e820t[i].type);
        kputc('|');
        kprint_hex32(e820t[i].extended_attributes);
        kputs("|\n");
    }

}

