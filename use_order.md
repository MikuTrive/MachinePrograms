# Machine Directive and Build-Header Guide

[中文纵览](./use_order.zh-CN.md) | [Project README](./README.md)

> Scope: the source-header directives that may appear at the top of current Machine `.mne` files.
>
> This guide explains **when to use each directive, when not to use it, how directives combine, and what kind of code each combination is meant for**.

## What these directives control

Machine header directives control different layers of the build pipeline:

- `bin.runtime`
  - controls **where runtime support comes from**
  - tells the compiler to auto-discover Machine runtime files instead of requiring manual runtime copies

- `unsafe.enable`
  - controls **whether low-level and dangerous operations are allowed**
  - allows `unsafe:` blocks and unsafe builtins

- `backend.c`
  - selects the **C backend**

- `backend.x86_64-asm`
  - selects the **x86_64 assembly backend**

- `target.linux-hosted`
  - selects a **normal Linux user-space program**

- `target.freestanding-x86_64`
  - selects a **freestanding x86_64 target** with a lighter runtime model

- `target.baremetal-x86_64`
  - selects a **baremetal x86_64 target** for boot or kernel-style outputs

## Where to place them

Put these directives at the **very top** of the source file, before normal code.

Recommended order:

```machine
bin.runtime
unsafe.enable
backend.x86_64-asm
target.baremetal-x86_64

main:
  ...
```

Do not treat them as mandatory boilerplate. Only include the directives the program actually needs.

## When to use `bin.runtime`

Use `bin.runtime` when the program relies on the standard Machine runtime and you want the compiler to find it automatically.

Typical cases:

- `print`
- `alloc_bytes`
- `free_mem`
- `load_*` / `store_*`
- `mmap_anon`
- `fd_open_*`, `fd_read`, `fd_write`
- terminal, time, window, image, or other hosted runtime features

Example:

```machine
bin.runtime

main:
  print "Hello, Machine"
  ret 0
```

Usually **do not** rely on `bin.runtime` for baremetal targets, because baremetal uses its own target-specific runtime sources and linker script.

## When to use `unsafe.enable`

Use `unsafe.enable` as soon as the program starts performing raw or dangerous operations.

Typical unsafe builtins include:

- `ptr_offset`
- `ptr_to_i64`
- `ptr_from_i64`
- `load_u8/u16/u32/u64`
- `store_u8/u16/u32/u64`
- `volatile_load_*`
- `volatile_store_*`
- `syscall*`
- `mmap_anon`
- `ioctl_i64`
- selected CPU, asm, and port-I/O builtins

Example:

```machine
bin.runtime
unsafe.enable

main:
  unsafe:
    var mem: ptr = alloc_bytes(16)

    store_u32(mem, 111)
    store_u32(ptr_offset(mem, 4), 222)
    store_u32(ptr_offset(mem, 8), 333)

    print load_u32(mem)
    print load_u32(ptr_offset(mem, 4))
    print load_u32(ptr_offset(mem, 8))

    free_mem(mem)

  ret 0
```

If the program only prints text or uses ordinary high-level features, `unsafe.enable` is not needed.

## When to use `backend.c`

Use `backend.c` when you want to make the backend choice explicit in the source, even though the C backend is usually the default.

Example:

```machine
bin.runtime
backend.c

main:
  print "use c backend"
  ret 0
```

## When to use `backend.x86_64-asm`

Use `backend.x86_64-asm` when you explicitly want the x86_64 assembly backend.

Typical cases:

- backend experimentation
- lower-level runtime work
- freestanding experiments
- baremetal outputs

Example:

```machine
bin.runtime
unsafe.enable
backend.x86_64-asm

main:
  unsafe:
    var pid: i64 = syscall0(39)
    print pid
  ret 0
```

For ordinary application programs, the C backend is usually still the simpler default choice.

## When to use `target.linux-hosted`

Use `target.linux-hosted` when you want to state explicitly that the program is a normal Linux user-space program.

Example:

```machine
bin.runtime
backend.c
target.linux-hosted

main:
  print "linux hosted"
  ret 0
```

This directive is optional for normal Linux programs because that remains the default target in common workflows.

## When to use `target.freestanding-x86_64`

Use `target.freestanding-x86_64` for lower-level x86_64 programs that should avoid the full hosted runtime model.

Example:

```machine
unsafe.enable
backend.x86_64-asm
target.freestanding-x86_64

main:
  unsafe:
    var pid: i64 = syscall0(39)
    print pid > 0
    print pid
  ret 0
```

Good fit for:

- syscall-focused experiments
- freestanding runtime validation
- lower-level code that is not yet baremetal

## When to use `target.baremetal-x86_64`

Use `target.baremetal-x86_64` only when the program is meant for baremetal boot or kernel-style execution.

Typical cases:

- baremetal hello-world outputs
- VGA memory experiments
- paging, GDT, IDT, and interrupt experiments
- QEMU, GRUB, or real boot-chain tests

Example:

```machine
unsafe.enable
backend.x86_64-asm
target.baremetal-x86_64

main:
  print "Machine baremetal hello"
  ret 0
```

Example with fixed VGA memory writes:

```machine
unsafe.enable
backend.x86_64-asm
target.baremetal-x86_64

main:
  print "Machine baremetal hello"

  unsafe:
    var text: ptr = ptr_from_i64(753664)
    volatile_store_u16(text, 8013)
    volatile_store_u16(ptr_offset(text, 2), 8014)
    volatile_store_u16(ptr_offset(text, 4), 8015)

  ret 0
```

Important: do **not** expect a baremetal target to run as a normal Linux user program with `./program`. A baremetal build is not a normal hosted executable.

## Common combinations

### 1. Normal Linux application

```machine
bin.runtime

main:
  print "hello"
  ret 0
```

### 2. Linux low-level experiment

```machine
bin.runtime
unsafe.enable

main:
  unsafe:
    var mem: ptr = mmap_anon(16)
    store_u32(mem, 123)
    print load_u32(mem)
    munmap_mem(mem, 16)
  ret 0
```

### 3. Linux program using the asm backend

```machine
bin.runtime
unsafe.enable
backend.x86_64-asm

main:
  unsafe:
    var pid: i64 = syscall0(39)
    print pid
  ret 0
```

### 4. Freestanding program

```machine
unsafe.enable
backend.x86_64-asm
target.freestanding-x86_64

main:
  unsafe:
    var pid: i64 = syscall0(39)
    print pid > 0
  ret 0
```

### 5. Baremetal program

```machine
unsafe.enable
backend.x86_64-asm
target.baremetal-x86_64

main:
  print "baremetal hello"
  ret 0
```

## Practical decision guide

If you just want a normal Linux program, start with:

```machine
bin.runtime
```

If you start using pointers, raw memory, `mmap`, `syscall`, `volatile`, or other low-level builtins, add:

```machine
unsafe.enable
```

If you explicitly want assembly output, add:

```machine
backend.x86_64-asm
```

If the program is not meant for normal Linux execution and should instead target a freestanding or baremetal environment, switch to the matching `target.*` directive.

## Common mistakes

### Treating all directives as mandatory

They are not. Use the smallest correct set for the current program.

### Running a baremetal target with `./program`

A baremetal build is not a normal Linux hosted executable.

### Forgetting `unsafe.enable`

Many raw pointer and memory builtins are intentionally gated behind the unsafe mode.

### Assuming the asm backend is always the right choice

The C backend is still the simpler and more complete path for many ordinary programs.


## Pointer address formatting: binary and hexadecimal

Machine now provides two pointer-formatting builtins for address display:

- `ptr_hex(pointer_value)`
  - returns the pointer address as a hexadecimal string
  - example output: `0x00007ffd1c2a4b80`

- `ptr_bin(pointer_value)`
  - returns the pointer address as a binary string
  - example output: `0b000000000000000001111111...`

These builtins are meant for low-level inspection, memory-layout experiments, and address tracing.

### When to use them

Use these builtins when you want to:

- inspect stack variable addresses
- inspect heap or `mmap` addresses
- compare address spacing in bytes
- display an address in a readable base without manually converting decimal output

Because they expose raw pointer information, they are treated as unsafe builtins.

### Required directives

Use them with:

```machine
bin.runtime
unsafe.enable
```

and place the actual calls inside an `unsafe:` block.

### Example: hexadecimal pointer output

```machine
bin.runtime
unsafe.enable

main:
  var x: i64 = 123

  unsafe:
    print ptr_hex(@x)

  ret 0
```

### Example: binary pointer output

```machine
bin.runtime
unsafe.enable

main:
  var x: i64 = 123

  unsafe:
    print ptr_bin(@x)

  ret 0
```

### Example: heap layout in hexadecimal

```machine
bin.runtime
unsafe.enable

main:
  unsafe:
    var mem: ptr = alloc_bytes(16)

    print ptr_hex(mem)
    print ptr_hex(ptr_offset(mem, 4))
    print ptr_hex(ptr_offset(mem, 8))

    free_mem(mem)

  ret 0
```

### Notes

- `ptr_hex(...)` and `ptr_bin(...)` return strings, so they are meant to be used directly with `print`.
- They do not change the address itself; they only change how the address is presented.
- If you still need the numeric decimal form, continue to use `ptr_to_i64(...)`.
