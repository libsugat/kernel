#include <arch/interrupt.h>
#include <arch/types.h>
#include <arch/mm.h>

#include <kernel/bits.h>
#include <kernel/boot_info.h>
#include <kernel/mem_symbols.h>
#include <kernel/mm.h>

#include <lib/kprint.h>

#include <kdef.h>

// Beacuse it is ia32, we cannot do anything beyond this
#define MEMORY_LIMIT_ADDR 0xFFFFFFFF

/**
 * Standard E820 / ACPI Memory Type Codes
 * Plus, Some custom types
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
    E820_TYPE_CUST_RESERVED_KERN = 0xDEADBEEF,   /* Kernel internal early reservation */
    E820_TYPE_CUST_MEMORY_HOLE = 0xDEADB01D   /* Kernel internal early reservation */
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

    // Sort
    sort_e820_insertion(entries, count);

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

    // Add holes
    size_t write_count = count;
    for (size_t curr = 1; curr < count; curr++) {
        size_t prev = curr - 1;

        uint64_t last_end = entries[prev].base_address + entries[prev].length;
        uint64_t curr_start = entries[curr].base_address;
        if (last_end != curr_start) {
            struct e820_entry new;
            new.base_address = last_end;
            new.length = curr_start - last_end;
            new.extended_attributes = 1;
            new.type = E820_TYPE_CUST_MEMORY_HOLE;

            entries[write_count++] = new;
        }
    }
    count = write_count;
    sort_e820_insertion(entries, count);
    return count;
}

void arch_preprocess_memory_map(struct boot_info_struct *boot_info_ptr)
{
    struct e820_entry * e820t = boot_info_ptr->e820_table;
    size_t entries = boot_info_ptr->e820_entries;

    kprintf("Bootloader found has %d e820 entries\n", boot_info_ptr->e820_entries);
    kprintf("Kernel starts at %x end at %x\n", (reg_t)__k_phy_start, (reg_t)__k_phy_end);

    size_t u_entries = santize_e820_inplace(e820t, entries);
    boot_info_ptr->e820_entries = u_entries;
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

    uint64_t total_available_space = 0;
    uint64_t last_free_ram_mark = 0;
    for (size_t i = 0; i < u_entries; i++) {
        uint64_t end = e820t[i].base_address + e820t[i].length - 1;
        kprintf("%d |", i);
        kprint_hex64(e820t[i].base_address);
        kputc('|');
        kprint_hex64(end);
        kputc('|');
        kprint_hex64(e820t[i].length);
        if (e820t[i].type == E820_TYPE_RAM) {
            total_available_space += e820t[i].length;
            last_free_ram_mark = end;
        }
        kputc('|');
        kprint_hex32(e820t[i].type);
        kputc('|');
        kprint_hex32(e820t[i].extended_attributes);
        kputs("|\n");
    }

    kprintf("Total available size: ");
    kprint_hex64(total_available_space);
    kputc('\n');
    kprintf("Last addr of ram: ");
    kprint_hex64(last_free_ram_mark);
    kputc('\n');
}

void arch_init_mem_map(struct boot_info_struct *boot_info_ptr, struct mem_map *mmap) {
    struct e820_entry * e820t = boot_info_ptr->e820_table;
    size_t entries = boot_info_ptr->e820_entries;

    uint64_t kernel_start = (uintptr_t)__k_phy_start;
    if (!IS_PAGE_AIGNED(kernel_start)) {
        kernel_start = ALIGN_DOWN(kernel_start, PAGE_SIZE);
    }
    uint64_t kernel_end = (uintptr_t)__k_phy_end;
    if (!IS_PAGE_AIGNED(kernel_end)) {
        kernel_end = ALIGN_UP(kernel_end, PAGE_SIZE);
    }

    // TODO: 
    // Step 1: find the number of pages

    uint64_t last_ram_addr = 0;
    for (size_t i = 0; i < entries; i++) {
        if (e820t[i].type == E820_TYPE_RAM) {
            last_ram_addr = e820t[i].base_address + e820t[i].length - 1;
        }
    }

    uint64_t total_pages = FIELD_GET(last_ram_addr, ~(PAGE_SIZE - 1));
    size_t total_size_meta = (size_t)total_pages * sizeof(struct page_meta);
    uint64_t total_pages_for_meta = FIELD_GET(total_size_meta, ~(PAGE_SIZE - 1));
    // if nPAGE_SIZE < total_size < (n+1)PAGE_SIZE
    if (total_size_meta & (PAGE_SIZE -1)) {
        total_pages_for_meta++;
    }
    kprintf("Total meta size : %x\n", total_size_meta);
    kprintf("Total meta pages: %x\n", total_pages_for_meta);

    // TODO:
    // Step 2: Find the appropriate place to place the meta data array 
    size_t required_space = total_pages_for_meta * PAGE_SIZE;
    for (size_t i = 0; i < entries; i++) {
        if (e820t[i].type != E820_TYPE_RAM) {
            continue;
        }

        uint64_t start = e820t[i].base_address;
        uint64_t size = e820t[i].length;
        uint64_t end = start + size - 1;

        if (kernel_start >= start && kernel_end <= end) {
            uint64_t size_bk = kernel_start - start;
            uint64_t size_ak = end - kernel_end;

            if (size_bk >= required_space) {
                mmap->page_meta_arr = (struct page_meta *)(uintptr_t)start;
                break;
            }
            else if (size_ak >= required_space ) 
            {
                mmap->page_meta_arr = (struct page_meta *)(uintptr_t)kernel_end;
                break;
            }
        }
        else {
            if (size >= required_space) {
                mmap->page_meta_arr = (struct page_meta *)(uintptr_t)start;
                break;
            }
        }
    }
    mmap->n_pages = total_pages;
    kprintf("kernel_start: %x\n", (uintptr_t)kernel_start);
    kprintf("kernel_end: %x\n", (uintptr_t)kernel_end);
    kprintf("Memory space choosen: %x\n", (uintptr_t)mmap->page_meta_arr);


    // Step 3: Put the the meta there and mark each used
    struct page_meta * mmap_meta = mmap->page_meta_arr;
    for (uint64_t i = 0; i < total_pages; i++) {
        mmap_meta[i].order = 0;
        mmap_meta[i].status = PAGE_STATUS_USED;
        mmap_meta[i].next = NULL;
        mmap_meta[i].next = NULL;
        mmap_meta[i].ref_cnt = 1;
    }
    for (size_t i = 0; i < entries; i++) {
        if (e820t[i].type == E820_TYPE_RAM) continue;

        uint64_t e820_start = e820t[i].base_address;
        uint64_t e820_end = e820_start + e820t[i].length;
        if (last_ram_addr <= e820_start) break;

        uint64_t page_start_no = e820_start / PAGE_SIZE;
        // uint64_t page_end =  e820_end / PAGE_SIZE;
        // Gemini suggested
        uint64_t page_end   = (e820_end + PAGE_SIZE - 1) / PAGE_SIZE;

        for (uint64_t p = page_start_no; p < page_end; p++) {
            mmap_meta[p].ref_cnt++;
        }
    }
    // Do the same for kernel
    {
        uint64_t page_start_no = kernel_start / PAGE_SIZE;
        uint64_t page_end = kernel_end / PAGE_SIZE;

        for (uint64_t i = page_start_no; i < page_end; i++) {
            mmap_meta[i].ref_cnt++;
        }
    }
    // Do the same for alloc meta
    {
        uint64_t page_start_no = (uintptr_t)mmap_meta / PAGE_SIZE;
        uint64_t page_end = page_start_no + total_pages_for_meta;

        for (uint64_t i = page_start_no; i < page_end; i++) {
            mmap_meta[i].ref_cnt++;
        }
    }


    // Step 4: Free the pages backword except the non ram blocks, kernel and the meta itself
    for (int i = 0; i < MAX_ORDER_ALLOC; i++) {
        mmap->freelist[i] = NULL;
    }
    mmap_meta[0].ref_cnt++; // Keep page 0 permanently reserved (Suggested by gemini)
    for (int i = total_pages - 1; i >= 0; i--) {
        mmap_meta[i].ref_cnt--;
        if (mmap_meta[i].ref_cnt == 0) {
            free_page_frame(&mmap_meta[i]);
        }
    }
}

