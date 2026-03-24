/* Shared compiler types and diagnostics. */
#ifndef MACHINE_COMMON_H
#define MACHINE_COMMON_H

#include <stdbool.h>
#include <stddef.h>

#define MACHINE_MAX_TOKENS 32768
#define MACHINE_MAX_ERRORS 256
#define MACHINE_MAX_FUNCTIONS 256
#define MACHINE_MAX_PARAMS 32
#define MACHINE_MAX_STATEMENTS 4096
#define MACHINE_MAX_SYMBOLS 2048
#define MACHINE_MAX_LINE_TEXT 512
#define MACHINE_MAX_EXPR_POOL 8192
#define MACHINE_MAX_STRUCTS 128
#define MACHINE_MAX_FIELDS 64

/*
 * Type kinds known by the v0.7 front-end.
 * TYPE_STRUCT means "look at the attached struct name too".
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

typedef struct
{
    const char *path;
    const char *source;
    size_t source_length;
} SourceFile;

typedef struct
{
    int line;
    int column;
    char message[256];
    char line_text[MACHINE_MAX_LINE_TEXT];
} Diagnostic;

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
void diagnostics_print(const char *kind, const char *path, const DiagnosticList *list);
const char *machine_type_name(MachineType type);
const char *machine_type_to_c(MachineType type);

#endif
