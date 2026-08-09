OUTPUT_ARCH(arm)

MEMORY
{
    /* The wholesale title map occupies 0x08800000-0x088004FF. Keep compiled
       gameplay at the beginning of expanded ROM so later Armips autoregions
       grow away from it with plenty of room before 0x09000000. */
    ROM (rx) : ORIGIN = 0x08800800, LENGTH = 0x00100000
}

SECTIONS
{
    .text : ALIGN(4)
    {
        c_code_start = .;
        *(.text .text.*)
        . = ALIGN(4);
        c_code_text_end = .;
        *(.rodata .rodata.*)
        . = ALIGN(4);
        c_code_end = .;
    } > ROM

    .data : ALIGN(4)
    {
        *(.data .data.*)
    } > ROM

    .bss (NOLOAD) : ALIGN(4)
    {
        *(.bss .bss.* COMMON)
    } > ROM

    /DISCARD/ :
    {
        *(.bn6_metadata)
        *(.comment)
        *(.ARM.attributes)
        *(.note*)
        *(.eh_frame*)
    }
}

ASSERT(SIZEOF(.data) == 0, "C runtime must not require initialized writable data")
ASSERT(SIZEOF(.bss) == 0, "C runtime must not require zero-initialized writable data")
