#include <kernel/mm.h>

#include <arch/mm.h>

void init_mm(struct boot_info_struct * boot_info) {
    arch_init_mm(boot_info);
}

