#ifndef _KERNEL_TYPES_H
#define _KERNEL_TYPES_H

#ifndef __ASSEMBLER__

#include <arch/types.h>

#ifndef __ARCH_HAS_BASIC_TYPES__
typedef __UINT8_TYPE__   u8;
typedef __UINT16_TYPE__  u16;
typedef __UINT32_TYPE__  u32;
typedef __UINT64_TYPE__  u64;

typedef __INT8_TYPE__    s8;
typedef __INT16_TYPE__   s16;
typedef __INT32_TYPE__   s32;
typedef __INT64_TYPE__   s64;

typedef __UINTPTR_TYPE__ uintptr_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;

#endif  // __ARCH_HAS_BASIC_TYPES__

typedef reg_t virt_addr_t;
typedef reg_t phys_addr_t;

typedef reg_t           size_t;

/* --- something we all love --- */ 
typedef u8  uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;

typedef s8 int8_t;
typedef s16 int16_t;
typedef s32 int32_t;
typedef s64 int64_t;

typedef u32 uintmax_t;
typedef s32 intmax_t;

/* --- domain specific types --- */
typedef u32             pid_t;       /* process id stays 32-bit everywhere */
typedef u64             offset_t;    /* file system offset stays 64-bit */

_Static_assert(sizeof(virt_addr_t) == sizeof(void*), "vaddr_t size mismatch with pointer width!");
_Static_assert(sizeof(u64) == 8, "u64 must be exactly 8 bytes!");

#endif // __ASSEMBLER__
#endif 
