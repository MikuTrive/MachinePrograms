/*
 * Annotated reading edition of common.h
 *
 * This file keeps the original code intact and only adds explanatory comments.
 * The goal of this edition is to explain the role of the header, the meaning of its
 * declarations, and how it fits into the Machine compiler / runtime architecture.
 */

/* Shared compiler types and diagnostics. */
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#ifndef MACHINE_COMMON_H
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_COMMON_H

/*
 * Dependency include.
 *
 * This brings in declarations required by the current header.
 */
#include <stdbool.h>
/*
 * Dependency include.
 *
 * This brings in declarations required by the current header.
 */
#include <stddef.h>

/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_MAX_TOKENS 32768
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_MAX_ERRORS 256
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_MAX_FUNCTIONS 256
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_MAX_PARAMS 32
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_MAX_STATEMENTS 4096
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_MAX_SYMBOLS 2048
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_MAX_LINE_TEXT 512
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_MAX_EXPR_POOL 8192
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_MAX_STRUCTS 128
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_MAX_FIELDS 64

/*
 * Type kinds known by the v0.7 front-end.
 * TYPE_STRUCT means "look at the attached struct name too".
 */
/*
 * Enumeration declaration.
 *
 * Enums usually define token kinds, AST node categories, type tags, or other fixed symbolic values.
 */
typedef enum
{
    TYPE_VOID,
    TYPE_I64,
    TYPE_F64,
    TYPE_HP,
    TYPE_STR,
    TYPE_PTR,
    TYPE_LIST,
    TYPE_ARRAY,
    TYPE_BOOL,
    TYPE_STRUCT,
    TYPE_UNKNOWN,
    TYPE_INVALID
} MachineType;

/*
 * Structure declaration.
 *
 * Structures in this project carry parser state, token records, AST nodes, type information, or runtime-facing data.
 */
typedef struct
{
    const char *path;
    const char *source;
    size_t source_length;
} SourceFile;

/*
 * Structure declaration.
 *
 * Structures in this project carry parser state, token records, AST nodes, type information, or runtime-facing data.
 */
typedef struct
{
    int line;
    int column;
    char message[256];
    char line_text[MACHINE_MAX_LINE_TEXT];
} Diagnostic;

/*
 * Structure declaration.
 *
 * Structures in this project carry parser state, token records, AST nodes, type information, or runtime-facing data.
 */
typedef struct
{
    Diagnostic items[MACHINE_MAX_ERRORS];
    size_t count;
} DiagnosticList;

void diagnostics_add(const SourceFile *src,
                     DiagnosticList *list,
                     int line,
                     int column,
                     const char *fmt,
                     ...);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void diagnostics_print(const char *kind, const char *path, const DiagnosticList *list);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
const char *machine_type_name(MachineType type);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
const char *machine_type_to_c(MachineType type);

/*
 * Preprocessor directive.
 *
 * Directives here usually define compile-time constants, feature switches, or version identifiers.
 */
#endif
