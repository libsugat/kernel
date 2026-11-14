.extern kernel_start
.globl _entry

// 0xFFFF_8000_0000_0000 // hh canonical start
// 0xFFFF_FFFF_8000_0000 // kernel phystart

.section .bootstrap.bss, "aw", @nobits
/* Kernel Stack, 4KiB */
.balign 0x1000
bootstrap_stack_bottom:
    .skip 0x1000
bootstrap_stack_top:

boot_pml4:
    .skip 0x1000
boot_pdpt0:
    .skip 0x1000
boot_pdpt_k:
    .skip 0x1000
boot_pd:
    .skip 0x1000
boot_pt:
    .skip 0x1000

.section .bootstrap.data, "aw"
.balign 8
temp_gdt_start:
    .quad 0x0000000000000000 // NULL
    .quad 0x00cf9a000000ffff // 32-bit code (0x08)
    .quad 0x00cf92000000ffff // 32-bit data (0x10)
    .quad 0x00af9a000000ffff // 64-bit code (0x18)
    .quad 0x00af92000000ffff // 64-bit data (0x20)
temp_gdt_end:

temp_gdt_desc:
    .short temp_gdt_end - temp_gdt_start - 1
    .long temp_gdt_start
    .long 0

#define PICM_COMMAND 0x20
#define PICM_DATA    0x21
#define PICS_COMMAND 0xA0
#define PICS_DATA    0xA1

#define CR0_PG      31
#define CR4_PAE     5
#define EFER_MSR    0xC0000080
#define EFER_LME    8
#define EFER_LMA    10

.section .bootstrap.text, "ax", @progbits

.code32
_entry:
    // Setup the explicit stack for kernel, the bootstrap stack
    movl $bootstrap_stack_top, %esp
    pushl %eax
    pushl %ebx

    call pic_8259A_disable
    call create_paging_map
    call load_temp_gdt

    popl %ebx
    popl %eax

    jmp enable_long_mode


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

hang: // just incase
    cli
    hlt
    jmp hang

create_paging_map:
    /* --- create pml4 --- */
    movl $boot_pdpt0, %eax
    orl $0x3, %eax              // Present and R/W
    // identity map
    movl %eax, boot_pml4        // upper already zeroed

    // higher half for kernel
    movl $boot_pdpt_k, %eax
    orl $0x3, %eax              // Present and R/W
    movl %eax, boot_pml4 + (511 * 8)

    /* --- create pdpe --- */
    movl $boot_pd, %eax
    orl $0x3, %eax              // Present and R/W
    movl %eax, boot_pdpt0
    movl %eax, boot_pdpt_k + 510*8

    /* --- create pdpe --- */
    // 2 page entries make it 4mb
    movl $0x83, %eax
    movl $boot_pd, %edi
    movl %eax, (%edi) // first 2 MiB

    addl $0x200000, %eax
    addl $8, %edi
    movl %eax, (%edi) // 2-4 MiB
    ret

load_temp_gdt:
    lgdt temp_gdt_desc

    movl $0x10, %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    ljmp $0x08, $1f
1:
    ret

enable_long_mode:
    /* Very important */
    movl %eax, %esi // Save bootloader signature
    movl %ebx, %edi // Save boot_info address

    // Ref: AMD64 AMD64 Architecture Programmer’s Manual 
    //      Volume 2, page 513
    // oh what a page number just off by one from 2^9 :)

    // Step 1. Enable PAE (physical address extension)
    movl %cr4, %eax
    orl $(1 << CR4_PAE), %eax
    movl %eax, %cr4

    // Step 2. Load CR3 with the physical base-address of PML4
    movl $boot_pml4, %eax
    movl %eax, %cr3

    // Step 3. Enable long mode by setting EFER.LME to 1.
    movl $EFER_MSR, %ecx
    rdmsr
    orl $(1 << EFER_LME), %eax
    wrmsr

    // Step 4. Enter compatibility mode by enabling paging
    movl %cr0, %eax
    orl $(1 << CR0_PG), %eax
    movl %eax, %cr0

    // far jump, compatibility mode to long mode
    ljmp $0x18, $entered_long_mode

.code64
entered_long_mode:
    movw $0x20, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    // retore bootloader signature and boot_info ptr
    movq %rsi, %rax
    movq %rdi, %rbx

    // Jump to higher half kernel code
    movabs $higher_half_kernel_entry, %rcx
    jmp %rcx

/*
* =====================================================================
*               HIGHER HALF CODE
* =====================================================================
*/
.section .text
higher_half_kernel_entry:
    addl %esi, %edi
    hlt
    jmp higher_half_kernel_entry

// TODO: move this to proper file
// void switch_to(struct task_struct *curr, struct task_struct *next);
.globl switch_to
switch_to:
    /* 1. Save callee-saved registers on Task A's stack */
    // Old switch to when kernel code was 32-bit
    /*
    pushfl // find a way to put this in schedule function 
    cli
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    */

    // movl 24(%esp), %eax      /* eax = curr (pointer to task_struct) */
    // movl 28(%esp), %edx      /* edx = next (pointer to task_struct) */

    // movl %esp, (%eax)        /* curr->esp = esp (freeze Task A) */
    // movl (%edx), %esp        /* esp = next->esp (switch to Task B) */

    /*
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    popfl
    */
    ret


.section .rodata
// IDT descriptor
.globl idt_desc
idt_desc:
	.word 256*16-1		
    .quad idt_table

.section .bss
// IDT Table
.globl idt_table
.balign 8
idt_table:
.skip 256*8

.section .bss.kernel_stack, "aw", @nobits
/* Kernel Stack, 8KiB */
.balign 0x1000
.globl kstack_bottom
kstack_bottom:
    .skip 0x2000
kstack_top:
