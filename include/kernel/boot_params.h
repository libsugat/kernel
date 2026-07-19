#ifndef KERNEL_BOOT_PARAMS_H
#define KERNEL_BOOT_PARAMS_H

#include <kernel/types.h>

struct e820_entry {
    u64 base_address;
    u64 length;
    u32 type;

    u32 extended_attributes;    // Be carefull before use can contain nothing
                                // Based the bios
} __attribute__((__packed__));


#endif // _KERNEL_BOOT_PARAMS_H
