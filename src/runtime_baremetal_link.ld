/*
 * Annotated reading copy of runtime_baremetal_link.ld
 *
 * Purpose of this linker script:
 * - Define the bare-metal linker layout, memory organization, section placement, and entry symbol expectations.
 *
 * Reading guidance:
 * - Start with the entry symbol and output format expectations.
 * - Then examine how sections such as .text, .rodata, .data, and .bss are ordered.
 * - Finally, note any alignment or address assumptions because those directly affect
 *   boot-time correctness for low-level targets.
 */

OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)
SECTIONS
/* Section block overview: this block controls one region of the final output image. */
{
  . = 1M;
/* Section block overview: this block controls one region of the final output image. */
  .text ALIGN(4K) : {
    *(.multiboot)
    *(.text*)
  }
  .rodata ALIGN(4K) : { *(.rodata*) }
  .data ALIGN(4K) : { *(.data*) }
/* Section block overview: this block controls one region of the final output image. */
  .bss ALIGN(4K) : {
    *(COMMON)
    *(.bss*)
  }
  _end = .;
}
