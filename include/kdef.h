#ifndef KDEF_H
#define KDEF_H

#ifndef __ASSEMBLER__

#define NULL            (void*)(0x0)

#define PACKED          __attribute__((__packed__))
#define UNUSED          __attribute__((__unused__))
#define ALIGNED(x)      __attribute__((aligned(x)))
#define NORETURN        __attribute__((__noreturn__))
#define ALWAYS_INLINE   inline __attribute__((__always_inline__))

#define SECTION(sec)    __attribute__((section(sec)))

// Critical memory access barrier
#define barrier()       asm volatile("" ::: "memory")

#endif // __ASSEMBLER__
#endif // KDEF_H
