#ifndef KERNEL_MEMORY_SYMBOLS_H
#define KERNEL_MEMORY_SYMBOLS_H

#ifndef __ASSEMBLER__

extern char __k_phy_start[];
extern char __k_phy_end[];
extern char kstack_bottom[];

#endif /* !__ASSEMBLER__ */

#endif /* KERNEL_MEMORY_SYMBOLS_H */
