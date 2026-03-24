/* Human-facing CLI messages for the machine compiler. */

#include "cli.h"
#include "version.h"

#include <stdio.h>

void print_help(void) {
    puts("Machine compiler");
    puts("Compiled, C-backed, Python-like systems language.");
    puts("");
    puts("Usage:");
    puts("  machine <input.mne> -o <output>");
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
    puts("  - labels/goto and integer switch/case/default");
    puts("  - line comments using --");
    puts("  - floating-point values and long-double hp helpers");
    puts("  - linked list helpers, array literals, dynamic arrays, 2D grids");
    puts("  - terminal input/time helpers");
    puts("  - window creation, renderer drawing, image loading, first-stage video helpers");
}

void print_version(void) {
    printf("Machine language version %s\n", MACHINE_LANG_VERSION);
    printf("Machine compiler iteration %s\n", MACHINE_ITERATION_VERSION);
}
