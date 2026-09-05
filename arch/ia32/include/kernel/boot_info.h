#ifndef KERNEL_BOOT_PARAMS_H
#define KERNEL_BOOT_PARAMS_H

#include <kernel/types.h>

#include <acpi/rsdt.h>

#ifndef __ASSEMBLER__

#include <kdef.h>

struct e820_entry {
    u64 base_address;
    u64 length;
    u32 type;
    u32 extended_attributes;    // Be carefull before use, can contain nothing
                                // Bios may or may not set anything
} PACKED;

struct boot_info_struct {
    struct e820_entry * e820_table;
    uint32_t e820_entries;
    struct rsdp * rsdp_ptr_struct;
} PACKED;

#endif // __ASSEMBLER__
#endif // _KERNEL_BOOT_PARAMS_H
