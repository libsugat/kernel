#ifndef _kernel_types_h
#define _kernel_types_h

#include <arch_types.h>

typedef __UINT8_TYPE__   u8;
typedef __UINT16_TYPE__  u16;
typedef __UINT32_TYPE__  u32;
typedef __UINT64_TYPE__  u64;

typedef __INT8_TYPE__    s8;
typedef __INT16_TYPE__   s16;
typedef __INT32_TYPE__   s32;
typedef __INT64_TYPE__   s64;

typedef __UINTPTR_TYPE__ uintptr_t;
typedef __SIZE_TYPE__    size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;

typedef uintptr_t reg_t;
typedef reg_t virt_addr_t;
typedef reg_t phys_addr_t;

typedef reg_t           size_t;

/* --- something we all love --- */ 
typedef u8  uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;

/* --- domain specific types --- */
typedef u32             pid_t;       /* process id stays 32-bit everywhere */
typedef u64             offset_t;    /* file system offset stays 64-bit */

#endif 
