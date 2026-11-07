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
} PACKED ALIGNED(8); // 16byte alignement was a bug

_Static_assert(sizeof(struct gdt_segment_descriptor) == 8, 
               "Descriptor size must be exactly 8 bytes");

struct gdtr {
    uint16_t limit;
    uint32_t base;
} PACKED;

_Static_assert(sizeof(struct gdtr) == 6, "gdtr struct has incorrect size (must be 6 bytes)!");

#endif // __ASSEMBLER__

#endif // ARCH_IA32_GDT_H
