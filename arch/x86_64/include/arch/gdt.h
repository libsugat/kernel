#ifndef ARCH_IA32_GDT_H
#define ARCH_IA32_GDT_H

#ifndef __ASSEMBLER__
#include <kernel/types.h>
#include <kdef.h>

struct gdt_segment_descriptor {
    union {
        struct {
            uint16_t limit_low;
            uint16_t base_low;
            uint8_t base_mid;
            uint8_t access_byte;
            uint8_t flags_limit; // Flags (high 4 bits) + Limit 19:16 (low 4 bits)
            uint8_t base_high;
        };
        uint64_t raw;
    };
} PACKED ALIGNED(8); // 16byte alignement was a bug

STATIC_ASSERT(sizeof(struct gdt_segment_descriptor) == 8, 
               "Descriptor size must be exactly 8 bytes");

struct gdtr {
    uint16_t limit;
    uintptr_t base;
} PACKED;

STATIC_ASSERT(sizeof(struct gdtr) == 10, "gdtr struct has incorrect size (must be 10 bytes)!");

#endif // __ASSEMBLER__

#endif // ARCH_IA32_GDT_H
