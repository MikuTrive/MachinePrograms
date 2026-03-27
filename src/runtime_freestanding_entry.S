# -----------------------------------------------------------------------------
# Annotated reading copy of runtime_freestanding_entry.S
#
# Purpose of this file:
# - Provide the minimal assembly entry point for freestanding x86_64 programs and exit through raw syscalls.
#
# Reading guidance:
# - Follow section directives first to understand where code/data will land.
# - Then identify exported labels, early boot/setup work, and transfer points into C.
# - Watch for ABI assumptions: register usage, stack discipline, and handoff contracts.
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# Annotated copy of runtime_freestanding_entry.S
#
# Label overview: # Important note
# - This label marks a control-transfer anchor inside runtime_freestanding_entry.S.
# - Read the instructions immediately after it as one logical phase.
# Important note:
# - The original assembly instructions are preserved exactly.
# - This file only adds detailed English comments.
#
# Label overview: # Purpose of this file
# - This label marks a control-transfer anchor inside runtime_freestanding_entry.S.
# - Read the instructions immediately after it as one logical phase.
# Purpose of this file:
# - Provide the absolute minimal entry point for the freestanding Linux-flavored
#   runtime variant.
# - Call the generated Machine user entry function.
# - Use the raw Linux x86_64 exit syscall to terminate the process with the
#   return value produced by machine_user_main.
# -----------------------------------------------------------------------------

# Export the ELF entry symbol expected by the linker.
    .global _start
# machine_user_main is provided by generated user code / runtime glue.
    .extern machine_user_main
# Process entry point. No libc startup code runs here.
# Label overview: _start
# - This label marks a control-transfer anchor inside runtime_freestanding_entry.S.
# - Read the instructions immediately after it as one logical phase.
_start:
# Clear RBP to mark the outermost frame and avoid inheriting garbage frame state.
    xor %rbp, %rbp
# Transfer control directly to the Machine program main entry.
    call machine_user_main
# SysV return value from machine_user_main becomes the first syscall argument: exit status.
    mov %rax, %rdi
# Linux x86_64 syscall number 60 = exit.
    mov $60, %rax
# Invoke the kernel directly without libc.
    syscall
# Safety stop: execution should never reach here if the exit syscall succeeds.
    hlt
