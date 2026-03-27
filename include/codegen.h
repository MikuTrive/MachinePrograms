/*
 * Annotated reading edition of codegen.h
 *
 * This file keeps the original code intact and only adds explanatory comments.
 * The goal of this edition is to explain the role of the header, the meaning of its
 * declarations, and how it fits into the Machine compiler / runtime architecture.
 */

/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#ifndef MACHINE_CODEGEN_H
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_CODEGEN_H

/* Code generation interface. */

/*
 * Dependency include.
 *
 * This brings in declarations required by the current header.
 */
#include "common.h"
/*
 * Dependency include.
 *
 * This brings in declarations required by the current header.
 */
#include "parser.h"

/*
 * Enumeration declaration.
 *
 * Enums usually define token kinds, AST node categories, type tags, or other fixed symbolic values.
 */
typedef enum
{
    MACHINE_TARGET_LINUX_HOSTED,
    MACHINE_TARGET_FREESTANDING_X86_64,
    MACHINE_TARGET_BAREMETAL_X86_64
} MachineTarget;

/*
 * Enumeration declaration.
 *
 * Enums usually define token kinds, AST node categories, type tags, or other fixed symbolic values.
 */
typedef enum
{
    MACHINE_BACKEND_C,
    MACHINE_BACKEND_X86_64_ASM
} MachineBackend;

/*
 * Structure declaration.
 *
 * Structures in this project carry parser state, token records, AST nodes, type information, or runtime-facing data.
 */
typedef struct
{
    int prefer_system_runtime;
    int allow_unsafe;
    MachineTarget target;
    MachineBackend backend;
} MachineCompileOptions;

/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
bool generate_c_file(const Program *program, const char *output_path, DiagnosticList *errors);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
bool generate_asm_file(const Program *program, const char *output_path, DiagnosticList *errors);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
bool compile_c_to_binary(const char *c_path, const char *binary_path, const char *source_path, const MachineCompileOptions *options, DiagnosticList *errors);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
bool compile_asm_to_binary(const char *asm_path, const char *binary_path, const char *source_path, const MachineCompileOptions *options, DiagnosticList *errors);

/*
 * Preprocessor directive.
 *
 * Directives here usually define compile-time constants, feature switches, or version identifiers.
 */
#endif
