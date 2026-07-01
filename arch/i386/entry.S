.extern start

.section .text
.globl _entry

_entry:
    // Setup the explicit stack for kernel
    movl $kstack_top, %esp
    pushl $0        // push 3 zeros just as safety padding
    pushl $0
    pushl $0
    pushl $hang     // kernel should not return but let it be a safety net
    jmp start       // jump to the the C code

hang:
    cli
    hlt

.section .bss
.skip 0x1000
kstack_top:
