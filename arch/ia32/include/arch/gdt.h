#ifndef ARCH_IA32_GDT_H
#define ARCH_IA32_GDT_H

#ifndef __ASSEMBLER__
#include <kernel/types.h>
#include <kdef.h>

struct gdt_segment_descriptor {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access_byte;
    uint8_t flags_limit; // Flags (high 4 bits) + Limit 19:16 (low 4 bits)
    uint8_t base_high;
} PACKED;

#endif // __ASSEMBLER__

#endif // ARCH_IA32_GDT_H
