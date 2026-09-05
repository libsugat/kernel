#ifndef ARCH_MM_H
#define ARCH_MM_H

#include <kernel/boot_info.h>

#define PAGE_SIZE 4096
#define IS_PAGE_AIGNED(addr) (!((addr) & (PAGE_SIZE - 1)))

#ifndef __ASSEMBLER__


void arch_init_mm(struct boot_info_struct * boot_info_ptr);
void arch_build_memory_bitmap(struct boot_info_struct * boot_info_ptr);

#endif // __ASSEMBLER__
#endif // ARCH_MM_H
