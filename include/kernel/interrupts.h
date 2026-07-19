#ifndef KERNEL_INTERRUPTS_H
#define KERNEL_INTERRUPTS_H

#include <arch/interrupt.h>

void initialize_interrupts(void);
void register_hwirq_handler();

// void notify_hwirq_complete(/* current pic*/);

#endif // KERNEL_INTERRUPTS_H 
