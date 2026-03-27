# -----------------------------------------------------------------------------
# Annotated reading copy of runtime_baremetal_entry.S
#
# Purpose of this file:
# - Provide the low-level assembly entry sequence for the bare-metal target, including processor setup and transfer into C runtime code.
#
# Reading guidance:
# - Follow section directives first to understand where code/data will land.
# - Then identify exported labels, early boot/setup work, and transfer points into C.
# - Watch for ABI assumptions: register usage, stack discipline, and handoff contracts.
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# Annotated copy of runtime_baremetal_entry.S
#
# Label overview: # Important note
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
# Important note:
# - The original assembly instructions and directives are preserved exactly.
# - This file only adds detailed English comments around the existing code.
# - The purpose of this file is to explain the bare-metal bootstrap path used by
#   the Machine runtime for x86_64 targets.
#
# Label overview: # High-level responsibilities of this file
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
# High-level responsibilities of this file:
# 1. Provide a Multiboot-compatible header so a loader such as GRUB can load the
#    kernel image.
# 2. Reserve early boot stacks for 32-bit and 64-bit execution stages.
# 3. Build static page-table data so the CPU can enter long mode.
# 4. Define a minimal 64-bit Global Descriptor Table (GDT).
# 5. Enter 64-bit long mode from a 32-bit bootstrap entry point.
# 6. Call the C runtime initialization routine and then the Machine user entry.
# 7. Provide interrupt/trap entry stubs and a common interrupt dispatch path.
# -----------------------------------------------------------------------------

# Tell GNU assembler to parse instructions using Intel syntax without register
# prefixes like '%' and immediate prefixes like '$'.
.intel_syntax noprefix

# -----------------------------------------------------------------------------
# Multiboot header
#
# This section is placed in the image so a Multiboot-aware bootloader can detect
# Label overview: # the kernel. The three long values are
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
# the kernel. The three long values are:
# - magic number
# - flags
# - checksum so the three values sum to zero modulo 2^32
# -----------------------------------------------------------------------------
.section .multiboot
# Align the Multiboot header on a 4-byte boundary as expected by loaders.
.align 4
# Standard Multiboot magic value.
.long 0x1BADB002
# Multiboot flags used by this image.
.long 0x00000003
# Checksum chosen so magic + flags + checksum == 0 (32-bit wraparound).
.long -(0x1BADB002 + 0x00000003)

# -----------------------------------------------------------------------------
# Early bootstrap stacks
#
# The .bss section reserves stack memory without storing explicit bytes in the
# final image. Two stacks are kept: one for the 32-bit bootstrap stage and one
# for the later 64-bit long-mode stage.
# -----------------------------------------------------------------------------

.section .bss
.align 16
# Label marking the base of the temporary 32-bit boot stack.
# Label overview: boot_stack32
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
boot_stack32:
# Reserve 16 KiB for the 32-bit bootstrap stack.
.skip 16384
# The top label is the address loaded into ESP during 32-bit startup.
# Label overview: boot_stack32_top
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
boot_stack32_top:
# Label marking the base of the 64-bit stack used after long mode is entered.
# Label overview: boot_stack64
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
boot_stack64:
# Reserve 32 KiB for the 64-bit runtime stack.
.skip 32768
# The top label is the address loaded into RSP in 64-bit mode.
# Label overview: boot_stack64_top
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
boot_stack64_top:

# -----------------------------------------------------------------------------
# Static paging structures
#
# These tables are prepared ahead of time so the bootstrap code can enable
# paging and long mode without needing a dynamic memory allocator.
#
# Label overview: # Layout summary
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
# Layout summary:
# - pml4_table: top-level page map level 4 table
# - pdpt_table: page-directory-pointer table with four entries
# - machine_boot_pd_table0..3: page directories using 2 MiB large pages
#
# With four page directories of 512 entries each, and each entry mapping 2 MiB,
# this setup identity-maps the first 4 GiB of physical memory.
# -----------------------------------------------------------------------------

.section .data
.align 4096
# Root page-map-level-4 table used when paging is turned on.
# Label overview: pml4_table
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
pml4_table:
# First PML4 entry points to the PDPT and marks it present + writable.
.quad pdpt_table + 0x003
# Zero all remaining PML4 entries.
.fill 511, 8, 0
.align 4096
# PDPT with four entries, each pointing to one page directory.
# Label overview: pdpt_table
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
pdpt_table:
# PDPT entry points at large-page directory table 0 and marks it present + writable.
.quad machine_boot_pd_table0 + 0x003
# PDPT entry points at large-page directory table 1 and marks it present + writable.
.quad machine_boot_pd_table1 + 0x003
# PDPT entry points at large-page directory table 2 and marks it present + writable.
.quad machine_boot_pd_table2 + 0x003
# PDPT entry points at large-page directory table 3 and marks it present + writable.
.quad machine_boot_pd_table3 + 0x003
# Zero the rest of the PDPT entries.
.fill 508, 8, 0
.align 4096
# Export the first page-directory table symbol for use elsewhere if needed.
.global machine_boot_pd_table0
# Page directory 0 maps 0 GiB .. 1 GiB using 2 MiB pages.
# Label overview: machine_boot_pd_table0
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
machine_boot_pd_table0:
# Start the generated page-frame index at 0 for the first GiB range.
.set i,0
# Emit 512 entries, each mapping one 2 MiB large page.
.rept 512
# Large-page PDE: physical base = i * 2 MiB; flags mark present/writable/page-size.
.quad (i * 0x200000) + 0x083
# Advance to the next 2 MiB frame.
.set i,i+1
# End the repeated page-directory entry generation block.
.endr
.align 4096
# Export the second page-directory table symbol.
.global machine_boot_pd_table1
# Page directory 1 maps 1 GiB .. 2 GiB.
# Label overview: machine_boot_pd_table1
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
machine_boot_pd_table1:
# Continue mapping at page-frame index 512, which corresponds to 1 GiB.
.set i,512
.rept 512
# Large-page PDE: physical base = i * 2 MiB; flags mark present/writable/page-size.
.quad (i * 0x200000) + 0x083
# Advance to the next 2 MiB frame.
.set i,i+1
# End the repeated page-directory entry generation block.
.endr
.align 4096
# Export the third page-directory table symbol.
.global machine_boot_pd_table2
# Page directory 2 maps 2 GiB .. 3 GiB.
# Label overview: machine_boot_pd_table2
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
machine_boot_pd_table2:
# Continue mapping at page-frame index 1024, which corresponds to 2 GiB.
.set i,1024
.rept 512
# Large-page PDE: physical base = i * 2 MiB; flags mark present/writable/page-size.
.quad (i * 0x200000) + 0x083
# Advance to the next 2 MiB frame.
.set i,i+1
# End the repeated page-directory entry generation block.
.endr
.align 4096
# Export the fourth page-directory table symbol.
.global machine_boot_pd_table3
# Page directory 3 maps 3 GiB .. 4 GiB.
# Label overview: machine_boot_pd_table3
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
machine_boot_pd_table3:
# Continue mapping at page-frame index 1536, which corresponds to 3 GiB.
.set i,1536

# -----------------------------------------------------------------------------
# 64-bit Global Descriptor Table (GDT)
#
# Long mode largely ignores segment base/limit for normal code/data access, but
# a valid GDT is still required in order to switch into 64-bit mode cleanly and
# to load code/data segment selectors.
# -----------------------------------------------------------------------------
.rept 512
# Large-page PDE: physical base = i * 2 MiB; flags mark present/writable/page-size.
.quad (i * 0x200000) + 0x083
# Advance to the next 2 MiB frame.
.set i,i+1
# End the repeated page-directory entry generation block.
.endr

.section .rodata
.align 8
# Start of the minimal GDT used during the transition into long mode.
# Label overview: gdt64
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
gdt64:
# Null descriptor: descriptor 0 must be all zeros.
.quad 0x0000000000000000
# 64-bit kernel code segment descriptor.
.quad 0x00AF9A000000FFFF
# 64-bit kernel data segment descriptor.
.quad 0x00AF92000000FFFF
# End marker used to compute the GDT limit field.
# Label overview: gdt64_end
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
gdt64_end:
# Pseudo-descriptor consumed by LGDT: limit followed by base address.
# Label overview: gdt64_ptr
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
gdt64_ptr:
# Size of the GDT in bytes minus one, as required by LGDT.
.word gdt64_end - gdt64 - 1
# 32-bit base address of the GDT used during the bootstrap stage.
.long gdt64

# -----------------------------------------------------------------------------
# Bootstrap entry point and long-mode transition
#
# Execution starts in 32-bit protected mode under a Multiboot loader. The code
# disables interrupts, establishes a temporary stack, loads the GDT, enables PAE,
# installs the PML4 into CR3, enables long mode via EFER.LME, enables paging,
# and performs a far jump into 64-bit code.
# -----------------------------------------------------------------------------

.section .text
# The following instructions are assembled for 32-bit mode.
.code32
# Export the bootstrap entry point expected by the linker/bootloader.
.global _start
# C helper that performs early bare-metal runtime initialization.
.extern machine_baremetal_init
# C entry point generated for the user program.
.extern machine_user_main
# C interrupt dispatcher called from the common interrupt stub.
.extern machine_interrupt_dispatch
# First instruction executed after the bootloader transfers control to us.
# Label overview: _start
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
_start:
# Disable interrupts during the critical transition setup.
    cli
# Install the temporary 32-bit stack.
    mov esp, offset boot_stack32_top
# Load the minimal GDT so valid segment descriptors are available.
    lgdt [gdt64_ptr]
# Read CR4 so required control bits can be enabled.
    mov eax, cr4
# Set CR4.PAE (bit 5), a prerequisite for long mode paging structures.
    or eax, 0x20
# Write the updated CR4 value back to the processor.
    mov cr4, eax
# Load the physical address of the top-level page table.
    mov eax, offset pml4_table
# Activate the new page-table hierarchy by storing it in CR3.
    mov cr3, eax
# Select the IA32_EFER model-specific register for RDMSR/WRMSR.
    mov ecx, 0xC0000080
# Read the current EFER value into EDX:EAX.
    rdmsr
# Set EFER.LME (Long Mode Enable).
    or eax, 0x00000100
# Write the updated EFER value back.
    wrmsr
# Read CR0 so paging and protected mode bits can be enforced.
    mov eax, cr0
# Set CR0.PG (paging) and ensure CR0.PE (protected mode) remains enabled.
    or eax, 0x80000001
# Commit the CR0 update; paging becomes active with the prepared tables.
    mov cr0, eax
# Far jump through the 64-bit code segment selector to complete the mode switch.
    ljmp 0x08, offset long_mode_entry

# -----------------------------------------------------------------------------
# 64-bit runtime entry
# -----------------------------------------------------------------------------

# The following instructions are assembled for 64-bit mode.
.code64
# First instructions executed after entering 64-bit long mode.
# Label overview: long_mode_entry
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
long_mode_entry:
# Load the kernel data-segment selector (GDT entry 2).
    mov ax, 0x10
# Initialize remaining segment registers to the data selector.
    mov ds, ax
# Initialize remaining segment registers to the data selector.
    mov es, ax
# Initialize remaining segment registers to the data selector.
    mov ss, ax
# Initialize remaining segment registers to the data selector.
    mov fs, ax
# Initialize remaining segment registers to the data selector.
    mov gs, ax
# Switch to the reserved 64-bit stack.
    mov rsp, offset boot_stack64_top
# Run early C-side platform setup: IDT, PIC/PIT, console, allocator, and similar work.
    call machine_baremetal_init
# Transfer control to the Machine program entry point.
    call machine_user_main
# the If the user entry returns, fall into a halt loop instead of continuing into garbage.
.hang:
# Disable interrupts during the critical transition setup.
    cli
# Halt the CPU until the next interrupt; combined with CLI this becomes a hard stop.
    hlt
# Loop forever in the halted state.
    jmp .hang

# -----------------------------------------------------------------------------
# Interrupt/trap entry stubs
#
# Each exported ISR label is a tiny front-end that normalizes the stack layout
# before jumping to a single common dispatcher. Some x86 exceptions push an
# some x86 exceptions push a hardware error code automatically; others do not, so two macros are used.
# -----------------------------------------------------------------------------

# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_default
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_irq0
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_irq1
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector0
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector1
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector2
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector3
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector4
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector5
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector6
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector7
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector8
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector9
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector10
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector11
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector12
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector13
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector14
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector15
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector16
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector17
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector18
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector19
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector20
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector21
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector22
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector23
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector24
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector25
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector26
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector27
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector28
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector29
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector30
# Export an interrupt/trap stub symbol so C code can place it into the IDT.
.global machine_isr_vector31

# Macro for exceptions/IRQs that do not receive a hardware-pushed error code.
.macro ISR_NOERR label, num
# Emit the concrete ISR label name supplied by the macro invocation.
# Label overview: \label
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
\label:
# Synthesize a zero error code so the common path sees a uniform frame.
    push 0
    push \num
    jmp machine_interrupt_common
# End macro definition.
.endm

# Macro for exceptions where the CPU already pushed an error code.
.macro ISR_ERR label, num
# Emit the concrete ISR label name supplied by the macro invocation.
# Label overview: \label
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
\label:
    push \num
# Reuse the shared interrupt prologue/epilogue logic.
    jmp machine_interrupt_common
# End macro definition.
.endm

# Fallback ISR stub used when no specific vector handler is installed.
# Label overview: machine_isr_default
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
machine_isr_default:
    push 0
# Use vector number 255 as an internal sentinel for the default handler.
    push 255
    jmp machine_interrupt_common

# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector0, 0
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector1, 1
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector2, 2
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector3, 3
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector4, 4
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector5, 5
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector6, 6
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector7, 7
# Instantiate one specific ISR front-end stub.
ISR_ERR   machine_isr_vector8, 8
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector9, 9
# Instantiate one specific ISR front-end stub.
ISR_ERR   machine_isr_vector10, 10
# Instantiate one specific ISR front-end stub.
ISR_ERR   machine_isr_vector11, 11
# Instantiate one specific ISR front-end stub.
ISR_ERR   machine_isr_vector12, 12
# Instantiate one specific ISR front-end stub.
ISR_ERR   machine_isr_vector13, 13
# Instantiate one specific ISR front-end stub.
ISR_ERR   machine_isr_vector14, 14
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector15, 15
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector16, 16
# Instantiate one specific ISR front-end stub.
ISR_ERR   machine_isr_vector17, 17
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector18, 18
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector19, 19
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector20, 20
# Instantiate one specific ISR front-end stub.
ISR_ERR   machine_isr_vector21, 21
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector22, 22
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector23, 23
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector24, 24
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector25, 25
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector26, 26
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector27, 27
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector28, 28
# Instantiate one specific ISR front-end stub.
ISR_ERR   machine_isr_vector29, 29
# Instantiate one specific ISR front-end stub.
ISR_ERR   machine_isr_vector30, 30
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_vector31, 31
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_irq0, 32
# Instantiate one specific ISR front-end stub.
ISR_NOERR machine_isr_irq1, 33


# -----------------------------------------------------------------------------
# Common interrupt entry/exit path
#
# Label overview: # Stack contract on entry
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
# Stack contract on entry:
# - topmost values are synthesized or hardware-provided error code + vector
# - the CPU has already pushed RIP, CS, RFLAGS, and possibly more state
#
# This routine saves general-purpose registers, extracts the vector number and
# copy the vector number and hardware error code into the first two SysV AMD64 argument registers, passes a pointer
# to the saved register frame in RDX, calls the C dispatcher, then restores the
# machine state and returns with IRETQ.
# -----------------------------------------------------------------------------
# Label overview: machine_interrupt_common
# - This label marks a control-transfer anchor inside runtime_baremetal_entry.S.
# - Read the instructions immediately after it as one logical phase.
machine_interrupt_common:
# Clear the direction flag so string operations in C always run forward.
    cld
# Save a general-purpose register onto the interrupt stack frame.
    push r15
# Save a general-purpose register onto the interrupt stack frame.
    push r14
# Save a general-purpose register onto the interrupt stack frame.
    push r13
# Save a general-purpose register onto the interrupt stack frame.
    push r12
# Save a general-purpose register onto the interrupt stack frame.
    push r11
# Save a general-purpose register onto the interrupt stack frame.
    push r10
# Save a general-purpose register onto the interrupt stack frame.
    push r9
# Save a general-purpose register onto the interrupt stack frame.
    push r8
# Save a general-purpose register onto the interrupt stack frame.
    push rsi
# Save a general-purpose register onto the interrupt stack frame.
    push rdi
# Save a general-purpose register onto the interrupt stack frame.
    push rbp
# Save a general-purpose register onto the interrupt stack frame.
    push rdx
# Save a general-purpose register onto the interrupt stack frame.
    push rcx
# Save a general-purpose register onto the interrupt stack frame.
    push rbx
# Save a general-purpose register onto the interrupt stack frame.
    push rax
# First argument to C dispatcher: vector number extracted from the stacked frame.
    mov rdi, [rsp + 120]
# Second argument to C dispatcher: error code extracted from the stacked frame.
    mov rsi, [rsp + 128]
# Third argument to C dispatcher: pointer to the saved register frame.
    mov rdx, rsp
# Maintain 16-byte stack alignment before calling into C.
    sub rsp, 8
# Let higher-level runtime code inspect and handle the interrupt.
    call machine_interrupt_dispatch
# Undo the temporary alignment adjustment.
    add rsp, 8
# Restore a previously saved general-purpose register.
    pop rax
# Restore a previously saved general-purpose register.
    pop rbx
# Restore a previously saved general-purpose register.
    pop rcx
# Restore a previously saved general-purpose register.
    pop rdx
# Restore a previously saved general-purpose register.
    pop rbp
# Restore a previously saved general-purpose register.
    pop rdi
# Restore a previously saved general-purpose register.
    pop rsi
# Restore a previously saved general-purpose register.
    pop r8
# Restore a previously saved general-purpose register.
    pop r9
# Restore a previously saved general-purpose register.
    pop r10
# Restore a previously saved general-purpose register.
    pop r11
# Restore a previously saved general-purpose register.
    pop r12
# Restore a previously saved general-purpose register.
    pop r13
# Restore a previously saved general-purpose register.
    pop r14
# Restore a previously saved general-purpose register.
    pop r15
# Discard the synthetic/normalized error-code and vector-number pair.
    add rsp, 16
# Return from the interrupt, restoring RIP/CS/RFLAGS (and privilege frame if any).
    iretq
