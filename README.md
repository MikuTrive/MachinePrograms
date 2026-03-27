# Machine

[中文](./README.zh-CN.md) | [Machine](./use_order.md)

Machine is a compiled systems language project implemented in C17. The current compiler front-end reads `.mne` source, performs lexical analysis, parsing, semantic checks, and then emits either generated C or x86_64 assembly depending on the selected backend. The generated output is then compiled or linked into the final program.

## Quick links

- [Build and install](#build-and-install)
- [Compiler usage](#compiler-usage)
- [Directive guide](./use_order.md)
- [Chinese overview](./README.zh-CN.md)

## Build and install

### Requirements

On Fedora/RHEL/CentOS:

```bash
sudo dnf install -y gcc make SDL2-devel SDL2_image-devel
```

On Debian/Ubuntu/Kali：
```bash
sudo apt install -y gcc make libsdl2-dev libsdl2-image-dev nasm
```

Window, image, and related hosted runtime features depend on SDL2 and SDL2_image being available when the hosted runtime object is built.

### Build the compiler

```bash
make Compilation
```

This produces:

- `./machine`
- `build/machine_runtime.o`


### Clean build outputs

```bash
make clean
```

### Install

```bash
sudo make install
```

The install target now installs:

- the `machine` compiler
- hosted, freestanding, and baremetal runtime support files
- Vim syntax files for `.mne` / `.machine`
- bundled example `.mne` programs
- project documentation, including the directive guide

### Uninstall

```bash
sudo make uninstall
```

## Compiler usage

### Show help

```bash
./machine --help
```

### Show version

```bash
./machine --version
```

### Compile a Machine program

```bash
./machine path/to/file.mne -o output_name
```

Example:

```bash
./machine test/struct.mne -o struct
```

## What kind of language is Machine?

Machine is a compiled language implementation.

More precisely:

- the compiler front-end is written in C17
- Machine source can be lowered to generated C or x86_64 assembly
- the selected backend is then compiled or linked into a native output
- the result is a native executable or baremetal ELF, depending on the selected target

So Machine is not an interpreter, and it is not a mixed interpreter/JIT runtime.

## Runtime discovery with `bin.runtime`

When the first meaningful line of a source file is:

```machine
bin.runtime
```

the compiler switches runtime discovery into installation/project mode. It will try, in order:

1. `/usr/local/lib/machine/machine_runtime.o` + `/usr/local/include/machine/machine_runtime.h`
2. a project-local `build/` runtime bundle next to the `.mne` file
3. a project-local runtime bundle in the project root
4. current-working-directory fallback copies

This allows application projects to compile without manually copying runtime files into every project directory.

## Current source tree

```text
MachinePrograms/
├── Compilation/                # CLI entry points and command handling
├── include/                    # public and internal headers
├── src/                        # compiler and runtime implementation
├── test/                       # bundled example and test .mne programs
├── vim/                        # Vim syntax, ftdetect, and ftplugin files
├── build/                      # generated runtime object and other build outputs
├── LICENSE
├── README.md
├── README.zh-CN.md
├── use_order.md
├── use_order.zh-CN.md
├── Makefile
└── m.sh
```

## Major modules

- **lexer**: tokenization of Machine source
- **parser**: expressions, statements, top-level declarations, and directives
- **semantic analysis**: symbol resolution, type checks, and diagnostics
- **C backend**: emits generated C
- **x86_64 asm backend**: emits GNU-style x86_64 assembly
- **runtime layers**:
  - `runtime.c` for Linux hosted programs
  - `runtime_freestanding.c` + entry assembly for freestanding targets
  - `runtime_baremetal.c` + entry assembly + linker script for baremetal targets
- **CLI**: command-line parsing, backend/target selection, final compilation pipeline

## Indentation rules

Machine accepts both 2-space and 4-space block indentation. A file may mix them across different blocks. Each nested block may increase indentation by either 2 or 4 spaces, and dedents must return to a previously established indentation column.

## Vim syntax highlighting

For a user-local install:

```bash
make vim-install
```

For a system-wide install, `sudo make install` now also installs the syntax files into a system Vim runtime location.

## Example and documentation install layout

After `sudo make install`, these locations are populated:

- `/usr/local/bin/machine`
- `/usr/local/lib/machine/`
- `/usr/local/include/machine/`
- `/usr/local/share/machine/examples/`
- `/usr/local/share/doc/machine/`
- `/usr/local/share/vim/vimfiles/`


This project involved the use of the AI ​​tool ChatGPT 5.4
MikuTrive assisted in the development of this Machine programming language project.


## License

Machine is distributed under GPL-3.0. See [LICENSE](./LICENSE).
