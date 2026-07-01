ENTRY(_entry)

SECTIONS {
    /* Currently the paging is not set up yet so keep this 
    * equal to the physical address
    */
    . = 0x100000;
    PROVIDE(text_start = .);

    .text : AT(0x100000)
    {
        *(.text .text.*)
    }

    PROVIDE(text_end = .);

    .rodata :
    {
        *(.rodata .rodata.*)
    }

    . = ALIGN(4K);

    PROVIDE(data_start = .);

    .data :
    {
        *(.data .data.*)
    }
    PROVIDE(data_end = .);

    .bss :
    {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }
    PROVIDE(end = .);

    /DISCARD/ : {
		*(.eh_frame )
        *(.note.*)
	}

}
