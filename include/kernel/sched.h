#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H

#include <arch/sched.h>

void create_new_task(void (*entry)(void), uint8_t *stack);
void init_sched(void);
void sched(void);

#endif // KERNEL_SCHED_H
