#ifndef ARCH_MM_H
#define ARCH_MM_H

#include <kernel/boot_info.h>

#define PAGE_SIZE 4096
#define IS_PAGE_AIGNED(addr) (!((addr) & (PAGE_SIZE - 1)))

#ifndef __ASSEMBLER__

void arch_preprocess_memory_map(struct boot_info_struct * boot_info_ptr);

struct mem_map; // Forward declaration
void arch_init_mem_map(struct boot_info_struct * boot_info_ptr, struct mem_map *mmap);

#endif // __ASSEMBLER__
#endif // ARCH_MM_H
