ENTRY(_entry)

SECTIONS {
    /* Currently the paging is not set up yet so keep this 
    * equal to the physical address
    */
    . = 0x100000;
    PROVIDE(__k_phy_start = .);
    PROVIDE(__k_text_start = .);

    .text : AT(0x100000)
    {
        *(.text .text.*)
    }

    PROVIDE(__k_text_end = .);

    .rodata :
    {
        *(.rodata .rodata.*)
    }

    . = ALIGN(4K);

    PROVIDE(__k_data_start = .);

    .data :
    {
        *(.data .data.*)
    }
    PROVIDE(__k_data_end = .);

    .bss :
    {
        . = ALIGN(4K);
        *(.bss.kernel_stack)
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }
    PROVIDE(__k_end = .);
    PROVIDE(__k_phy_end = .);

    /DISCARD/ : {
		*(.eh_frame )
        *(.note.*)
	}

}
