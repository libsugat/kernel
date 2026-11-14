#ifndef ASM_MSR_H
#define ASM_MSR_H

#ifndef __ASSEMBLER__

#include <kdef.h>
#include <kernel/types.h>

static ALWAYS_INLINE u64 rdmsr(u32 index) {
    u32 low, high;
    __asm__ __volatile__(
            "rdmsr"
            : "=a" (low), "=d" (high)
            : "c" (index)
            : "memory"
            );
    return ((uint64_t) high << 32) | low;
}

static ALWAYS_INLINE u64 wrmsr(u32 index, u64 val) {
    u32 low = (u32) val;
    u32 high = (u32) (val >> 32);
    __asm__ __volatile__(
            "wrmsr"
            :: "a" (low), "d" (high), "c" (index)
            : "memory"
            );
    return ((uint64_t) high << 32) | low;
}

#endif // __ASSEMBLER__

#endif // ASM_MSR_H
