#ifndef ARCH_INIT_H
#define ARCH_INIT_H

#include <kernel/boot_info.h>

#ifndef __ASSEMBLER__

void cpu_init_early(void);
void arch_hardware_init(struct boot_info_struct * boot_info_ptr);

#endif  // __ASSEMBLER__
#endif // ARCH_INIT_H
