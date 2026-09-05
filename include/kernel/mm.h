#ifndef KERNEL_MM_H
#define KERNEL_MM_H

#ifndef __ASSEMBLER__

struct boot_info_struct;

void init_mm(struct boot_info_struct *);

#endif // __ASSEMBLER__
#endif // KERNEL_MM_H 
