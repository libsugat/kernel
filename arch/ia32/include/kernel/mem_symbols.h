#ifndef KERNEL_MEMORY_SYMBOLS_H
#define KERNEL_MEMORY_SYMBOLS_H

#ifndef __ASSEMBLER__

#include <kernel/types.h>

extern char __k_phy_start[];
// extern char text_end[];
// extern char bss_start[];
extern char __k_phy_end[];

#endif /* !__ASSEMBLER__ */

#endif /* KERNEL_MEMORY_SYMBOLS_H */
