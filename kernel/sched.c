#include <kernel/types.h>
#include <kernel/sched.h>

#define N_TASKS 5

struct task_struct idle;
struct task_struct tasks[N_TASKS];
uint32_t task_len = 0;


void create_new_task(void (*entry)(void), uint8_t *stack)
{
    if (task_len >= N_TASKS) return;
    tasks[task_len++] = _create_new_task(entry, stack);
}

int current_task = -1;

void init_sched(void) {
    arch_init_sched(&idle);
}

void sched(void) {
    struct task_struct *current, *next;
    if (current_task == -1) {
        current = &idle;
        next = &tasks[0];
        current_task = 0;
    }
    else {
        current = &tasks[current_task];
        current_task = (current_task + 1) % task_len;
        next = &tasks[current_task];
    }
    switch_to(current, next);
}
