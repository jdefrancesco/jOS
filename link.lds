OUTPUT_FORMAT("elf64-x86-64")
ENTRY(start)

SECTIONS
{
    /* Load kernel at 0xffff800000200000 */
    . = 0xffff800000200000;
    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    . = ALIGN(16);
    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }

    PROVIDE(end = .);
}
