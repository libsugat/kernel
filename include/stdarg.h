#ifndef _STDARG_H
#define _STDARG_H

#ifndef __ASSEMBLER__

typedef __builtin_va_list k_va_list;

#define k_va_start(ap, last) __builtin_va_start(ap, last)
#define k_va_arg(ap, type)   __builtin_va_arg(ap, type)
#define k_va_end(ap)         __builtin_va_end(ap)

#endif // __ASSEMBLER__
#endif // _STDARG_H
