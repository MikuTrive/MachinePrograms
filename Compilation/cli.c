/*
 * Annotated reading edition of cli.c
 * ---------------------------------
 * This file is intentionally left functionally unchanged.
 * The only additions in this copy are explanatory comments.
 *
 * High-level purpose:
 *   - Provide the small user-facing command-line text interface for the
 *     Machine compiler.
 *   - Keep help/version messaging separate from the heavier compiler logic
 *     in main.c, which keeps responsibilities clean and reduces clutter in
 *     the true program entry point.
 *
 * Why this separation matters:
 *   - main.c is focused on orchestration: parsing options, reading source,
 *     lexing/parsing, generating output, and invoking the backend pipeline.
 *   - cli.c is focused on presentation: the exact text shown to the user.
 *   - This makes maintenance easier because feature descriptions can be
 *     updated without touching the compilation pipeline itself.
 */

/* Human-facing CLI messages for the machine compiler. */

#include "cli.h"
#include "version.h"

#include <stdio.h>

/*
 * print_help
 * ----------
 * Purpose:
 *   Emit the compiler's help/usage text to standard output.
 *
 * Detailed behavior:
 *   - The function uses puts() repeatedly rather than storing one huge
 *     multi-line string literal.
 *   - Each puts() call appends a trailing newline automatically.
 *   - The message is structured as:
 *       1) product identity
 *       2) one-line summary
 *       3) usage syntax
 *       4) a short code sample
 *       5) a feature list for the current iteration
 *
 * Why puts() is a reasonable choice here:
 *   - The content is static and line-oriented.
 *   - No formatting placeholders are needed.
 *   - The output remains easy to reorder or edit line by line.
 *
 * Design note:
 *   This function does not inspect arguments or environment state.
 *   It is a pure presentation helper called by main.c when the user
 *   requests --help or when no arguments are provided.
 */
void print_help(void)
{
    puts("Machine compiler");
    puts("Compiled, C-backed systems language.");
    puts("");
    puts("Usage:");
    puts("  machine <input.mne> -o <output> [--target <name>] [--backend <name>]");
    puts("  machine --help");
    puts("  machine --version");
    puts("");
    puts("Current syntax highlights:");
    puts("  struct Point:");
    puts("    x: i64");
    puts("    y: i64");
    puts("");
    puts("  module MathBox:");
    puts("    func twice(x: i64) -> i64:");
    puts("      ret x * 2");
    puts("");
    puts("  main:");
    puts("    var p = Point()");
    puts("    p.x = 10");
    puts("    print MathBox.twice(21)");
    puts("    ret 0");
    puts("");
    puts("Features in this iteration:");
    puts("  - native structs with field access and nested field access");
    puts("  - module encapsulation with Name.func(...) calls");
    puts("  - functions and function calls");
    puts("  - if / else / while");
    puts("  - strings, concatenation, len(), indexing, triple-quoted multiline strings");
    puts("  - pointers, alloc_bytes(), free_mem(), load/store helpers");
    puts("  - ptr casts, volatile memory access, syscall/mmap/fd/ioctl helpers");
    puts("  - unsafe: blocks gate low-level builtins when unsafe mode is enabled");
    puts("  - selected inline asm hooks and x86 port I/O helpers");
    puts("  - labels/goto and integer switch/case/default");
    puts("  - compile directives: bin.runtime, target.<name>, backend.<name>, unsafe.enable");
    puts("  - line comments using -- and optional bin.runtime preamble");
    puts("  - per-block 2-space or 4-space indentation (mixed file styles allowed)");
    puts("  - floating-point values and long-double hp helpers");
    puts("  - linked list helpers, array literals, dynamic arrays, 2D grids");
    puts("  - terminal input/time helpers");
    puts("  - window creation, renderer drawing, image loading, first-stage video helpers");
    puts("  - freestanding-x86_64 + backend.c keeps a minimal syscall-only runtime");
    puts("  - x86_64-asm backend now emits a direct scalar/unsafe/control-flow subset");
    puts("  - baremetal-x86_64 now emits a GRUB-loadable early-boot ELF kernel via asm backend");
}

/*
 * print_version
 * -------------
 * Purpose:
 *   Print the current language version and compiler iteration version.
 *
 * Data source:
 *   - MACHINE_LANG_VERSION comes from version.h and describes the language
 *     version understood by this compiler build.
 *   - MACHINE_ITERATION_VERSION comes from version.h and identifies the
 *     compiler's own implementation iteration.
 *
 * Why printf() is used here instead of puts():
 *   - The function needs to interpolate symbolic version constants into
 *     formatted output lines.
 *
 * Output contract:
 *   Two lines are printed, one for the language version and one for the
 *   compiler iteration. This keeps the information human-readable and easy
 *   to parse in scripts if needed.
 */
void print_version(void)
{
    printf("Machine language version %s\n", MACHINE_LANG_VERSION);
    printf("Machine compiler iteration %s\n", MACHINE_ITERATION_VERSION);
}
