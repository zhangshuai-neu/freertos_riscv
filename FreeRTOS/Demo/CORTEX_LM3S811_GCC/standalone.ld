MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 64K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 8K
}

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text)
        *(.rodata*)
        _etext = .;
    } > FLASH
    
    .ARM.exidx :
     {
        *(.ARM.exidx*)
       *(.gnu.linkonce.armexidx.*)
    } > FLASH
    
    _begin_data = .;
    .data : AT ( _begin_data )
    {
        _data = .;
        *(vtable)
        *(.data)
        _edata = .;
    } > SRAM

    .bss :
    {
        _bss = .;
        *(.bss)
        *(COMMON)
        _ebss = .;
    } > SRAM
}
