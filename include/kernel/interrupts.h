#ifndef KERNEL_INTERRUPTS_H
#define KERNEL_INTERRUPTS_H

#include <arch/interrupt.h>

void initialize_interrupts(void);
void register_hwirq_handler();

// void notify_hwirq_complete(/* current pic*/);

struct local_irqc_ops {
    void (*init)(void);
    void (*eoi)(u32 vector);
    void (*mask)(u32 vector);
    void (*unmask)(u32 vector);
};

#endif // KERNEL_INTERRUPTS_H 
