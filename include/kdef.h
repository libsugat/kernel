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
#define BARRIER()       asm volatile("" ::: "memory")

#define OFFSET_OF(type, member)  __builtin_offsetof(type, member)

#if defined(__has_builtin) && __has_builtin(__builtin_align_up)
    #define ALIGN_UP(value, align)   __builtin_align_up(value, align)
    #define ALIGN_DOWN(value, align) __builtin_align_down(value, align)
    #define IS_ALIGNED(value, align) __builtin_is_aligned(value, align)
#else
    /* Fallback for GCC, MSVC, etc. (align must be a power of two) */
    #define ALIGN_DOWN(val, align) \
        ((__typeof__(val))((uintptr_t)(val) & ~((uintptr_t)(align) - 1)))

    #define ALIGN_UP(val, align) \
        ((__typeof__(val))(((uintptr_t)(val) + ((uintptr_t)(align) - 1)) & ~((uintptr_t)(align) - 1)))

    #define IS_ALIGNED(val, align) \
        (((uintptr_t)(val) & ((uintptr_t)(align) - 1)) == 0)
#endif

// Static compiler checks
#define STATIC_ASSERT(cond, msg) _Static_assert((cond), msg)

#endif // __ASSEMBLER__
#endif // KDEF_H
