.section .text

// from idt.c
.extern master_fault_handler
.extern reserved_intr_handler
.extern higher_irq_handler

// to idt.c
.globl isr_stub_list

// Lets define some macors 
// this x86 interrupt are tedious
#define DEFINE_ISR_NO_ERR(num) \
    .globl isr_##num;  \
    isr_wraper_##num: \
        pushl $0; \
        pushl $num; \
        jmp master_fault_isr_stub

#define DEFINE_ISR_ERR(num) \
    .globl isr_##num;  \
    isr_wraper_##num: \
        pushl $num; \
        jmp master_fault_isr_stub

#define DEFINE_ISR_COM_OUTL(num, handler) \
    .globl isr_##num;  \
    isr_wraper_##num: \
        pushl $0; \
        pushl $num; \
        jmp handler


// this expects that the every handler being used has pushed value zero 
// if the interrupt does not pust an error value
.macro SAVE_UNSAVED_REGS
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx  
    pushl %ecx
    pushl %ebx
    pushl %eax

    pushl %gs
    pushl %fs
    pushl %es
    pushl %ds
.endm

.macro RESTORE_AFTER_STD_INTR
    popl %ds
    popl %es
    popl %fs
    popl %gs

    popl %eax
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp

    addl $8, %esp // pop vector number
    // pop error code
.endm

.macro GENERATE_STUB handler_func
    SAVE_UNSAVED_REGS

    movl $0x10, %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    movl %esp, %eax
    pushl %eax
    cld                 // clear the direction flag
    call \handler_func
    addl $4, %esp       // remove the esp passed as argument

    RESTORE_AFTER_STD_INTR
    // pop eip
    // pop cs
    // pop eflags
    iret
.endm

master_fault_isr_stub:
    GENERATE_STUB master_fault_handler


// Future Proofing
// Interrupt that are reserved for future use
reserved_intr_stub:
    GENERATE_STUB reserved_intr_handler

higher_irq_stub:
   GENERATE_STUB higher_irq_handler

.macro DEFINE_ISR_COM_GAS num, handler
    .globl isr_wraper_\num
    isr_wraper_\num:
        pushl $0
        pushl $\num
        jmp \handler
.endm

// The tedious work
// TODO: Later replace those with dedicated 
//       Handler for each fault
DEFINE_ISR_NO_ERR(0)
DEFINE_ISR_NO_ERR(1)
DEFINE_ISR_NO_ERR(2)
DEFINE_ISR_NO_ERR(3)
DEFINE_ISR_NO_ERR(4)
DEFINE_ISR_NO_ERR(5)
DEFINE_ISR_NO_ERR(6)
DEFINE_ISR_NO_ERR(7)
DEFINE_ISR_ERR(8)
DEFINE_ISR_NO_ERR(9)
DEFINE_ISR_ERR(10)
DEFINE_ISR_ERR(11)
DEFINE_ISR_ERR(12)
DEFINE_ISR_ERR(13)
DEFINE_ISR_ERR(14)
DEFINE_ISR_COM_OUTL(15, reserved_intr_stub)
DEFINE_ISR_NO_ERR(16)
DEFINE_ISR_ERR(17)
DEFINE_ISR_NO_ERR(18)
DEFINE_ISR_NO_ERR(19)
DEFINE_ISR_NO_ERR(20)
DEFINE_ISR_ERR(21)
DEFINE_ISR_COM_OUTL(22, reserved_intr_stub)
DEFINE_ISR_COM_OUTL(23, reserved_intr_stub)
DEFINE_ISR_COM_OUTL(24, reserved_intr_stub)
DEFINE_ISR_COM_OUTL(25, reserved_intr_stub)
DEFINE_ISR_COM_OUTL(26, reserved_intr_stub)
DEFINE_ISR_COM_OUTL(27, reserved_intr_stub)
DEFINE_ISR_COM_OUTL(28, reserved_intr_stub)
DEFINE_ISR_COM_OUTL(29, reserved_intr_stub)
DEFINE_ISR_COM_OUTL(30, reserved_intr_stub)
DEFINE_ISR_COM_OUTL(31, reserved_intr_stub)

.irp n,\
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,\
48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,\
64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,\
80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,\
96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,\
112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,\
128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,\
144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,\
160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,\
176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,\
192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,\
208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,\
224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,\
240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
    DEFINE_ISR_COM_GAS \n, higher_irq_stub
.endr


isr_stub_list:
.irp n,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,\
16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,\
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,\
48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,\
64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,\
80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,\
96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,\
112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,\
128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,\
144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,\
160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,\
176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,\
192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,\
208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,\
224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,\
240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
    .long isr_wraper_\n
.endr
