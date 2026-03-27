/*
 * Annotated reading copy of common.c
 *
 * What this file is for:
 * - Provide shared diagnostics, source-location utilities, and small cross-module helpers used throughout the compiler.
 *
 * How to read this file:
 * - First scan the includes to see which data structures and declarations this unit depends on.
 * - Then identify the major helper layers: low-level primitives, transformation helpers,
 *   public entry points, and any error-reporting or cleanup paths.
 * - Pay attention to stateful objects such as source files, token streams, AST nodes,
 *   emitted output buffers, runtime data, or target-specific configuration.
 *
 * Annotation policy:
 * - The original code body is preserved.
 * - Only explanatory comments are added.
 * - These comments are intended for learning and code-reading, not as a behavioral change.
 */

/* Diagnostic helpers and type-name utilities. */

#include "common.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Copy the textual contents of one source line for caret-style diagnostics. */
/*
 * Function overview: capture_line_text
 *
 * High-level purpose:
 * - This routine belongs to common.c.
 * - It exists to provide shared diagnostics, source-location utilities, and small cross-module helpers used throughout the compiler.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "capture line text".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
static void capture_line_text(const SourceFile *src, int target_line, char *out, size_t out_size)
{
    if (!src || !src->source || !out || out_size == 0)
    {
        return;
    }
    out[0] = '\0';
    int line = 1;
    const char *start = src->source;
    /* we implement a function to capture the textual contents of
     *       a specific line from the source file,
     *       which is used for caret-style diagnostics.
     *       this function takes the source file, the target line number, and an output buffer as input,
     *       and it extracts the text of the specified line from the
     *       source code, storing it in the output buffer for later use in error messages and diagnostics. */
    const char *cursor = src->source;
    while (*cursor)
    {
        if (line == target_line)
        {
            const char *end = cursor;
            while (*end && *end != '\n')
            {
                ++end;
            }
            size_t len = (size_t)(end - start);
            if (len >= out_size)
            {
                len = out_size - 1;
            }
            memcpy(out, start, len);
            out[len] = '\0';
            return;
        }
        if (*cursor == '\n')
        {
            ++line;
            start = cursor + 1;
        }
        ++cursor;
    }
    /* if the target line number is beyond the end of the source file,
     *       we return an empty string in the output buffer. */
    if (line == target_line)
    {
        size_t len = strlen(start);
        if (len >= out_size)
        {
            len = out_size - 1;
        }
        memcpy(out, start, len);
        out[len] = '\0';
    }
}

/*
 * Function overview: diagnostics_add
 *
 * High-level purpose:
 * - This routine belongs to common.c.
 * - It exists to provide shared diagnostics, source-location utilities, and small cross-module helpers used throughout the compiler.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "diagnostics add".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void diagnostics_add(const SourceFile *src,
                     DiagnosticList *list,
                     int line,
                     int column,
                     const char *fmt,
                     ...)
{
    if (!list || list->count >= MACHINE_MAX_ERRORS)
    {
        return;
    }
    Diagnostic *diag = &list->items[list->count++];
    diag->line = line;
    diag->column = column;

    va_list args;
    va_start(args, fmt);
    vsnprintf(diag->message, sizeof(diag->message), fmt, args);
    va_end(args);

    capture_line_text(src, line, diag->line_text, sizeof(diag->line_text));
}
/* we implement a function to add a diagnostic message to a list of diagnostics,
 *   which includes the line and column information, as well as a formatted message describing the error or warning.
 *   this function takes the source file for context, the diagnostic list to which the message will be added,
 *   the line and column numbers where the issue occurred,
 *   and a format string with additional arguments to create a detailed message.
 *   by centralizing diagnostic message creation in this function,
 *   we can ensure consistent formatting and handling of diagnostics throughout the compiler,
 *   making it easier to report errors and warnings to developers in a clear and informative manner. */

/*
 * Function overview: diagnostics_print
 *
 * High-level purpose:
 * - This routine belongs to common.c.
 * - It exists to provide shared diagnostics, source-location utilities, and small cross-module helpers used throughout the compiler.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "diagnostics print".
 *
 * Reading guidance:
 * - Start by identifying the incoming state, context object, or buffers used as inputs.
 * - Then follow how this routine transforms, validates, emits, or forwards that data.
 * - Finally, look at how results are returned: direct values, mutated structures,
 *   emitted text, diagnostics, or control-flow side effects.
 *
 * Maintenance notes:
 * - Be careful with ownership, temporary buffers, and error-reporting paths.
 * - In parser/codegen/runtime files especially, changes here usually affect multiple
 *   later stages, so trace callers before changing behavior.
 */
void diagnostics_print(const char *kind, const char *path, const DiagnosticList *list)
{
    /* we implement a function to print a list of diagnostics to the
     *       standard error stream, formatting each diagnostic message with the appropriate kind (e.g., error, warning, note),
     *       file path, line and column information, and the message itself.
     *       this function also includes color coding for different kinds of diagnostics to
     *       enhance readability and help developers quickly identify the severity of each message.
     *       by centralizing diagnostic printing in this function, we can maintain a consistent
     *       format for all diagnostics and provide clear and informative output that helps developers
     *       understand and address issues in their code effectively. */
    const char *prefix = "NOTE";
    const char *prefix_color = "\033[1;37m";
    const char *colon_color = "\033[37m";
    const char *caret_color = "\033[94m";
    const char *reset = "\033[0m";

    if (kind && strcmp(kind, "error") == 0)
    {
        prefix = "ERROR";
        prefix_color = "\033[1;31m";
    }
    else if (kind && (strcmp(kind, "warning") == 0 || strcmp(kind, "warn") == 0))
    {
        prefix = "WARN";
        prefix_color = "\033[1;33m";
    }

    for (size_t i = 0; i < list->count; ++i)
    {
        const Diagnostic *d = &list->items[i];
        fprintf(stderr,
                "%s%s%s:%s %s:%d:%d: %s\n",
                prefix_color,
                prefix,
                colon_color,
                reset,
                path,
                d->line,
                d->column,
                d->message);
        if (d->line_text[0] != '\0')
        {
            fprintf(stderr, "    %s\n", d->line_text);
            fprintf(stderr, "    ");
            for (int c = 1; c < d->column; ++c)
            {
                fputc(' ', stderr);
            }
            fprintf(stderr, "%s^%s\n", caret_color, reset);
        }
    }
}

/* we implement a function to get the string representation of a machine type,
 *   which is useful for error messages and debugging.
 *   this function takes a MachineType enum value as input and returns a
 *   string that represents the name of the machine type.
 *   by using this function, we can provide more informative error messages that
 *   include the specific machine type involved in an error, which helps
 *   developers understand and fix issues in their code more effectively. */
const char *machine_type_name(MachineType type)
{
    switch (type)
    {
    case TYPE_VOID:
        return "void";
    case TYPE_I64:
        return "i64";
    case TYPE_F64:
        return "f64";
    case TYPE_HP:
        return "hp";
    case TYPE_STR:
        return "str";
    case TYPE_PTR:
        return "ptr";
    case TYPE_LIST:
        return "list";
    case TYPE_ARRAY:
        return "array";
    case TYPE_BOOL:
        return "bool";
    case TYPE_STRUCT:
        return "struct";
    case TYPE_UNKNOWN:
        return "unknown";
    default:
        return "invalid";
    }
}

/* we implement a function to get the C language representation of a machine type,
 *   which is useful for code generation and debugging.
 *   this function takes a MachineType enum value as input and returns a
 *   string that represents the corresponding C type for that machine type.
 *   by using this function, we can ensure that the generated C code uses the
 *   correct type names for the various machine types defined in our programming language,
 *   which helps maintain consistency and correctness in the code generation process. */
const char *machine_type_to_c(MachineType type)
{
    switch (type)
    {
    case TYPE_VOID:
        return "void";
    case TYPE_I64:
        return "long long";
    case TYPE_F64:
        return "double";
    case TYPE_HP:
        return "long double";
    case TYPE_STR:
        return "char *";
    case TYPE_PTR:
        return "void *";
    case TYPE_LIST:
        return "MachineList *";
    case TYPE_ARRAY:
        return "MachineArray *";
    case TYPE_BOOL:
        return "int";
    case TYPE_STRUCT:
        return "/* struct */";
    default:
        return "/* invalid */";
    }
}
