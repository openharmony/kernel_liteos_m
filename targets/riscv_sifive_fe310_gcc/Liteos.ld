OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv", "elf32-littleriscv")
OUTPUT_ARCH("riscv")
ENTRY(_start)
INCLUDE board.ld.S
MEMORY
{
   ram(rwx) : ORIGIN = RAM_ADDR_START, LENGTH = RAM_ADDR_SIZE
   flash(rwx) : ORIGIN = FLASH_LOAD_START, LENGTH = FLASH_LOAD_SIZE
}
SECTIONS
{
  .text : ALIGN(0x20)
  {
    __text_start = .;
    . = ALIGN(0x10);
    *(.start.text)
    *(.interrupt.text)
    *(.text.*)
    . = ALIGN(0x20);
    __rodata_start = .;
    . = ALIGN(0x10);
    *(.rodata.* .srodata.*)
    . = ALIGN(0x20);
    __text_end = .;
  } > ram AT > flash
  .data : ALIGN(0x20)
  {
    __global_pointer$ = . + 0x800;
    __data_load_start = LOADADDR(.data);
    __data_start = .;
    . = ALIGN(0x10);
    *(.data.* .sdata.*)
    . = ALIGN(0x20);
    __data_end = .;
  } > ram AT > flash
  __data_load_size = SIZEOF(.data);
  .bss (NOLOAD): ALIGN(0x20)
  {
    __bss_start = .;
    . = ALIGN(0x10);
    *(.kernel.bss*)
    *(.bss.* .sbss.*)
    . = ALIGN(0x20);
    __bss_end = .;
  } > ram
  .stack (NOLOAD): ALIGN(0x40)
  {
    __stack_bootom = .;
    . += EXCEPT_STACK_SIZE;
    __except_stack_top = .;
    . += START_AND_IRQ_STACK_SIZE;
    __start_and_irq_stack_top = .;
  } > ram
/*
  .heap (NOLOAD): ALIGN(0x40)
  {
    __heap_start = .;
    . += HEAP_SIZE;
    __heap_end = .;
  } > ram
*/
}
