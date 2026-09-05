.extern kernel_start

#define PICM_COMMAND 0x20
#define PICM_DATA    0x21
#define PICS_COMMAND 0xA0
#define PICS_DATA    0xA1

.section .text
.globl _entry

_entry:
    // Setup the explicit stack for kernel
    movl $kstack_top, %esp
    push %eax
    push %ebx

    call pic_8259A_disable
    call setup_idt

    pop %ebx
    pop %eax

    // Time to move to kernel_start
    pushl $0        // push 2 zeros just as safety padding
    pushl $0
    push %eax
    push %ebx
    pushl $hang     // kernel should not return but let it be a safety net
    jmp kernel_start       // jump to the the C code

hang:
    cli
    hlt
    jmp hang

// We will need to reprogram the pic so that it does not conflict with
// cpu's intel exceptions
pic_8259A_disable:
    pushl %eax
    // Mask all interrupt of both PICs
    movb $0xFF, %al 
    outb %al, $PICM_DATA
    outb %al, $0x80 // io delay
    outb %al, $PICS_DATA
    popl %eax
    ret

setup_idt:
    lea ignore_int, %edx
    movl $0x00080000, %eax  // kernel code segement selector
    movw %dx, %ax           // load the lower 16-bits of address in eax
    movw $0x8E00, %dx       // load attributes in edx

    movl $idt_table, %edi   // init the memory location
    movl $256, %ecx         // counter
.idt_fill_loop:
    movl %eax, (%edi)
    movl %edx, 4(%edi)      // little endianness
    addl $8, %edi
    decl %ecx
    jnz .idt_fill_loop

    lidt idt_desc           // loed the idt
    ret

ignore_int:
	incb 0xb8000		
	movb $4,0xb8001
    iret

.section .rodata
// IDT descriptor
.globl idt_desc
idt_desc:
	.word 256*8-1		
    .long idt_table

.section .bss
// IDT Table
.globl idt_table
.balign 8
idt_table:
.skip 256*8

.section .bss.kernel_stack, "aw", @nobits
/* Kernel Stack, 4KiB */
.balign 0x1000
kstack_bottom:
.skip 0x1000
kstack_top:
