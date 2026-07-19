#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H

#include <kernel/types.h>

void tty_init(void);
void tty_clear(void);
void tty_putchar(char);
void tty_write(const char *, size_t);
void tty_write_str(const char *);

#endif // _KERNEL_TTY_H
