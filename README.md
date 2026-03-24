# Machine

[zh_CN](./README.zh-CN.md) | [Machine](./Machine.md)

Machine is a compiled systems language project implemented in C17. It is not an interpreter, and it is not a mixed interpreter/JIT runtime. The current compiler front-end reads `.mne` source, performs lexical analysis, parsing, semantic checks, generates C code, and then invokes a system C compiler to produce a native executable.

## Build

### Requirements

On Fedora:

```bash
sudo dnf install gcc make SDL2-devel SDL2_image-devel
```

Window, image, and related runtime features depend on SDL2/SDL2_image being available when the runtime object is built.

### Build the Machine compiler

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
./machine ceshi/struct.mne -o struct
```

## What kind of language is Machine?

Machine is a **compiled language implementation**.

More precisely:

- the language front-end is written in C17
- Machine source is compiled to generated C
- the generated C is compiled by the system C compiler
- the result is a native executable

So Machine is **not interpreted**. It is also **not a half-interpreted runtime language**. Its current implementation strategy is **source-to-source compilation with a native C backend**.

## Directory structure

```text
Machine_project_v17/
├── Compilation/          # CLI entry points and user-facing command handling
├── test/                 # test-source programs used by m.sh
├── include/              # public/internal headers used by the compiler/runtime
├── src/                  # compiler and runtime source code
├── build/                # generated runtime object and other build outputs
├── LICENSE               # GPL-3.0 license text
├── Machine.md            # bilingual language tutorial
├── README.md             # English project overview and build guide
├── README.zh-CN.md       # Chinese project overview and build guide
└── m.sh                  # test helper script for test/
```

## Functional modules

The project is currently organized around these major modules:

- **lexer**: tokenization of Machine source
- **parser**: expressions, statements, top-level declarations, tables, and constant literals
- **semantic analysis**: symbol resolution, type checking, const checks, container checks, diagnostics
- **code generator**: emits C code for user programs
- **runtime**: memory helpers, arrays, lists, grids, math helpers, terminal helpers, window/media helpers
- **CLI**: `machine --help`, `machine --version`, input/output handling, final compilation pipeline
- **test helper**: `m.sh` automates compile/run/cleanup for files in `ceshi/`

## Source code style and repository conventions

This repository follows these practical rules:

- C17 is the baseline language standard for compiler/runtime code.
- Header files live under `include/`.
- Compiler/runtime implementation files live under `src/`.
- CLI-specific entry files live under `Compilation/`.
- Teaching or repository documents live in the project root.
- Test `.mne` files for scripted checks live under `ceshi/`.
- A single source file should stay comfortably below the project line-count ceiling; split files before they become hard to read.
- Prefer direct names, shallow helper functions, and explicit boundary checks over compact but unclear code.
- Keep diagnostics readable and consistent.

## Test helper script

`m.sh` is the project-level test helper.

### Show help

```bash
./m.sh
```

### Compile and run all predefined tests in `ceshi/`

```bash
./m.sh -c
```

### Remove compiled test binaries only

```bash
./m.sh -d
```

This does **not** remove `./machine` itself.

### Choose output language for the script

```bash
./m.sh -l
```

The script currently supports:

- `en_US`
- `zh_CN`

## License

This open-source project uses the **GPL-3.0** license.

This project includes participation from **ChatGPT 5.4**.

This project is jointly maintained by **MikuTrive and GPT**.
