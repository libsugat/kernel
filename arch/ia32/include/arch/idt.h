#ifndef ARCH_IA32_IDT_H
#define ARCH_IA32_IDT_H

#include <kernel/types.h>
#include <kdef.h>

// Defining type attribute values
#define IDT_ATTR_PRESENT      0x80 // 1000 0000

#define IDT_ATTR_RING0        0x00 // 0000 0000
#define IDT_ATTR_RING3        0x60 // 0110 0000

#define IDT_ATTR_INT_GATE_32  0x0E // 0000 1110
#define IDT_ATTR_TRAP_GATE_32 0x0F // 0000 1111

struct idt_entry {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t  reserved;          // Should be always set to 0 and not used
    uint8_t  type_attributes;   
    uint16_t  offset_high;
} PACKED;

typedef struct {
    struct idt_entry entries[256];
} idt_t;

union idt_entry_union {
    struct idt_entry idt_entries;
    uint32_t raw_dwords[2];
};

struct idtr {
    uint16_t size;
    uintptr_t offset;
} PACKED;

ALWAYS_INLINE void set_gate(
    idt_t * idt, 
    uint8_t n, 
    const uint8_t gate_type_attr, 
    const uint8_t dpl_attr, 
    void (*handler) (void)
)
{
    union idt_entry_union * gate_union = (union idt_entry_union *) &idt->entries[n];
    union idt_entry_union new_gate;
    new_gate.idt_entries.reserved = 0;
    new_gate.idt_entries.offset_low = (uint16_t) ((uintptr_t)handler & 0xFFFF);
    new_gate.idt_entries.offset_high = (uint16_t) (((uintptr_t)handler >> 16) & 0xFFFF);
    new_gate.idt_entries.segment_selector = 0x08;
    new_gate.idt_entries.type_attributes = gate_type_attr | dpl_attr | IDT_ATTR_PRESENT;

    // Not interested in atomics right now
    // Lets do it as fast as possible so that interrupt doesnt fire up
    __asm__ __volatile__(
            "movl %2, %0\n\t" 
            "movl %3, %1"
            :"=m" (gate_union->raw_dwords[0]),
             "=m" (gate_union->raw_dwords[1])
            :"r" (new_gate.raw_dwords[0]),
             "r" (new_gate.raw_dwords[1])
            :"memory"
        );
}

#define set_int_gate(idt, n, handler) \
    set_gate(idt, n, IDT_ATTR_INT_GATE_32, IDT_ATTR_RING0, handler)

#define set_trap_gate(idt, n, handler) \
    set_gate(idt, n, IDT_ATTR_TRAP_GATE_32, IDT_ATTR_RING0, handler)

#define set_system_gate(idt, n, handler) \
    set_gate(idt, n, IDT_ATTR_TRAP_GATE_32, IDT_ATTR_RING3, handler)

#endif // ARC_IA32_IDT_H
