ENTRY(_entry)

KERNEL_VMA_BASE  = 0xFFFFFFFF80000000;

SECTIONS {
    /* Currently the paging is not set up yet so keep this 
    * equal to the physical address
    */
    . = 0x100000;
    PROVIDE(__bootstrap_phys_start = .);
    .bootstrap.text : AT(0x100000) {
        *(.bootstrap.text)
    }
    .bootstrap.data :
    {
        *(.bootstrap.data)
    }
    .bootstrap.bss :
    {
        *(.bootstrap.bss)
    }
    PROVIDE(__bootstrap_phys_end = .);

    . = 0x200000;
    . += KERNEL_VMA_BASE;

    .text : AT(0x200000)
    {
        *(.text .text.*)
    }

    .rodata :
    {
        *(.rodata .rodata.*)
    }

    .data ALIGN(4k):
    {
        *(.data .data.*)
    }

    .bss ALIGN(4k) :
    {
        *(.bss.kernel_stack)
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }

    PROVIDE(__k_phy_start = LOADADDR(.text));
    PROVIDE(__k_phy_end = LOADADDR(.bss) + SIZEOF(.bss));

    /DISCARD/ : {
		*(.eh_frame )
        *(.note.*)
	}

}
