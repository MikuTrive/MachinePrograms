/*
 * Annotated reading copy of codegen.c
 *
 * What this file is for:
 * - Generate C output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
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

#include "codegen.h"
#include "util.h"
#include "machine_runtime.h"
#include "version.h"

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_codegen_target_is_freestanding = 0;

/*
 * Function overview: indent
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "indent".
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
static void indent(FILE *out, int level)
{
    for (int i = 0; i < level; ++i)
    {
        fputs("    ", out);
    }
}

/* we implement a set of helper functions for code generation, including functions to emit type definitions,
 *   global variable declarations, expressions, and statements in C code.
 *   these functions provide a convenient interface for generating C code from our intermediate representation of
 *   the program, allowing us to produce readable and efficient C code that corresponds to the semantics of our programming language. */
static const char *c_type(TypeRef t, char *buf, size_t n)
{
    switch (t.kind)
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
        copy_cstr(buf, n, t.struct_name);
        return buf;
    default:
        return "/* invalid */";
    }
}

/*
 * Function overview: emit_zero_init
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit zero init".
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
static void emit_zero_init(FILE *out, TypeRef t)
{
    /* we implement a function to emit the zero initialization value for a given type,
     *       which is used in code generation to initialize variables and fields to a default value.
     *       this function takes a TypeRef as input and writes the appropriate zero initialization value to
     *       the output file stream, handling different types such as integers, floating-point numbers, strings,
     *       pointers, lists, arrays, and structs accordingly. */
    char buf[128];
    if (t.kind == TYPE_STRUCT)
        fprintf(out, "(%s){0}", c_type(t, buf, sizeof(buf)));
    else if (t.kind == TYPE_STR || t.kind == TYPE_PTR || t.kind == TYPE_LIST || t.kind == TYPE_ARRAY)
        fputs("0", out);
    else if (t.kind == TYPE_F64 || t.kind == TYPE_HP)
        fputs("0.0", out);
    else
        fputs("0", out);
}

/*
 * Function overview: emit_structs
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit structs".
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
static void emit_structs(FILE *out, const Program *program)
{
    char tbuf[128];
    for (size_t i = 0; i < program->struct_count; ++i)
    {
        const StructDecl *sd = &program->structs[i];
        fprintf(out, "typedef struct %s {\n", sd->name);
        for (size_t j = 0; j < sd->field_count; ++j)
        {
            fprintf(out, "    %s %s;\n", c_type(sd->fields[j].type, tbuf, sizeof(tbuf)), sd->fields[j].name);
            /* we emit the C struct definitions for each struct declared in our programming language, including the field types and names.
             *               this function iterates through the list of struct declarations in the program and
             *               generates a corresponding C struct definition for each one, ensuring that the field types are
             *               correctly translated to their C equivalents using the c_type helper function. */
        }
        fprintf(out, "} %s;\n\n", sd->name);
    }
}

/*
 * Function overview: emit_globals
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit globals".
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
static void emit_globals(FILE *out, const Program *program)
{
    char tbuf[128];
    for (size_t i = 0; i < program->global_count; ++i)
    {
        const GlobalVarDecl *g = &program->globals[i];
        fprintf(out, "static %s %s;\n", c_type(g->declared_type, tbuf, sizeof(tbuf)), g->name);
    }
    if (program->global_count > 0)
        fputc('\n', out);
}

static void emit_expr(FILE *out, const Expr *e);

/*
 * Function overview: call_name_from_expr
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "call name from expr".
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
static int call_name_from_expr(const Expr *callee, char *buf, size_t n)
{
    if (callee->kind == EXPR_IDENTIFIER)
        return copy_cstr(buf, n, callee->as.text);
    /* we implement a helper function to extract the name of a function being called from a call expression,
     *       which is used in code generation to determine how to emit calls to built-in functions and user-defined functions.
     *       this function takes an expression representing the callee of a call expression and
     *       attempts to extract the function name from it,
     *       handling cases where the callee is a simple identifier or a field access (e.g., struct method call).
     *       by using this function, we can identify calls to built-in functions and generate the
     *       appropriate C code for them, while also supporting user-defined functions and struct method calls in our programming language. */
    if (callee->kind == EXPR_FIELD && callee->as.field.base->kind == EXPR_IDENTIFIER)
        return join_cstr3(buf, n, callee->as.field.base->as.text, "__", callee->as.field.field);
    return 0;
}

/*
 * Function overview: is_machine_builtin_name
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "is machine builtin name".
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
static int is_machine_builtin_name(const char *name)
{
    static const char *builtins[] = {
        /* we implement a function to check if a given function name corresponds to a built-in function provided by the machine runtime,
         *           which is used in code generation to determine how to emit calls to these built-in functions.
         *           this function takes a function name as input and checks it against a list of known built-in function names,
         *           returning true if the name matches one of the built-ins and false otherwise.
         *           by using this function, we can ensure that calls to built-in functions are correctly identified and
         *           emitted with the appropriate C code that corresponds to their functionality in the machine runtime. */
        "hp_add", "hp_sub", "hp_mul", "hp_div", "hp_sqrt", "hp_pow",
        "alloc_bytes", "free_mem", "store_i64", "load_i64", "store_f64", "load_f64", "store_str", "load_str",
        "ptr_from_i64", "ptr_to_i64", "ptr_offset", "ptr_hex", "ptr_bin",
        "store_u8", "store_u16", "store_u32", "store_u64",
        "load_u8", "load_u16", "load_u32", "load_u64",
        "volatile_store_u8", "volatile_store_u16", "volatile_store_u32", "volatile_store_u64",
        "volatile_load_u8", "volatile_load_u16", "volatile_load_u32", "volatile_load_u64",
        "syscall0", "syscall1", "syscall2", "syscall3", "syscall4", "syscall5", "syscall6",
        "mmap_anon", "mmap_anon_exec", "munmap_mem",
        "fd_open_ro", "fd_open_wo", "fd_open_rw", "fd_close", "fd_read", "fd_write", "fd_seek",
        "ioctl_i64", "asm_nop", "asm_pause", "asm_mfence", "asm_lfence", "asm_sfence", "asm_rdtsc",
        "cpu_in8", "cpu_out8",
        "pmm_alloc_page", "pmm_alloc_pages", "pmm_total_bytes", "pmm_used_bytes",
        "page_identity_map_2m", "apic_supported", "apic_enable", "apic_eoi",
        "list_new", "list_push_back", "list_get", "list_size", "list_free",
        "array_new", "array_push", "array_get", "array_set", "array_len", "array_free",
        "grid_new", "grid_get", "grid_set", "grid_rows", "grid_cols", "grid_fill", "grid_free",
        "term_enable_raw", "term_disable_raw", "term_key_available", "term_read_key", "term_enable_mouse", "term_disable_mouse",
        "term_poll_event", "term_last_key", "term_mouse_x", "term_mouse_y", "term_mouse_button", "term_clear", "term_flush",
        "term_move_cursor", "term_hide_cursor", "term_show_cursor", "term_draw_text", "sleep_ms", "tick_ms", "timer_reset", "timer_elapsed_ms",
        "win_create", "win_destroy", "win_is_open", "win_poll_event", "win_last_key", "win_mouse_x", "win_mouse_y", "win_mouse_button",
        "win_clear", "win_present", "win_set_title", "win_draw_rect", "win_fill_rect", "win_draw_line", "win_draw_pixel", "win_draw_text", "win_draw_text",
        "image_load", "image_draw", "image_draw_scaled", "image_width", "image_height", "image_free",
        "video_play", "video_stop", "video_is_running"};
    for (size_t i = 0; i < sizeof(builtins) / sizeof(builtins[0]); ++i)
        if (strcmp(name, builtins[i]) == 0)
            return 1;
    return 0;
}

/*
 * Function overview: emit_call
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit call".
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
static void emit_call(FILE *out, const Expr *e)
{
    char name[128];
    if (!call_name_from_expr(e->as.call.callee, name, sizeof(name)))
    {
        fputs("/* unsupported call target */0", out);
        return;
        /* we implement a function to emit a function call expression in C code,
         *           which handles calls to built-in functions, user-defined functions, and struct constructors.
         *           this function first attempts to extract the function name from the call expression using the
         *           call_name_from_expr helper function, and if it fails to get a name,
         *           it emits a placeholder comment indicating an unsupported call target.
         *           if the function name is successfully extracted, the function checks if it matches any
         *           known built-in functions or struct constructors and emits the appropriate C code for those cases.
         *           for user-defined functions, it emits a standard function call using the extracted name and the emitted arguments.
         *           by handling different types of function calls in this way, we can generate correct and
         *           efficient C code that corresponds to the semantics of our programming language. */
    }
    if (!strcmp(name, "len"))
    {
        fputs("machine_len(", out);
        emit_expr(out, e->as.call.args[0]);
        fputc(')', out);
        return;
    }
    if (!strcmp(name, "index"))
    {
        fputs("machine_index_str(", out);
        emit_expr(out, e->as.call.args[0]);
        fputs(", ", out);
        emit_expr(out, e->as.call.args[1]);
        fputc(')', out);
        return;
    }
    if (!strcmp(name, "hp"))
    {
        fputs("machine_hp_from_text(", out);
        emit_expr(out, e->as.call.args[0]);
        fputc(')', out);
        return;
    }
    if (!strcmp(name, "addr"))
    {
        fputs("(&(", out);
        emit_expr(out, e->as.call.args[0]);
        fputs("))", out);
        return;
    }
    if (is_machine_builtin_name(name))
    {
        fprintf(out, "machine_%s(", name);
        for (size_t i = 0; i < e->as.call.arg_count; ++i)
        {
            if (i)
                fputs(", ", out);
            emit_expr(out, e->as.call.args[i]);
        }
        fputc(')', out);
        return;
    }
    if (!strcmp(name, "sqrt") || !strcmp(name, "sin") || !strcmp(name, "cos") || !strcmp(name, "pow"))
    {
        fprintf(out, "%s(", name);
        for (size_t i = 0; i < e->as.call.arg_count; ++i)
        {
            if (i)
                fputs(", ", out);
            emit_expr(out, e->as.call.args[i]);
        }
        fputc(')', out);
        return;
    }
    if (e->inferred_type.kind == TYPE_STRUCT && e->as.call.arg_count == 0 && e->as.call.callee->kind == EXPR_IDENTIFIER)
    {
        /* Emit a C struct initializer for a struct constructor call. */
        fprintf(out, "((%s){0})", name);
        return;
    }
    fprintf(out, "%s(", name);
    for (size_t i = 0; i < e->as.call.arg_count; ++i)
    {
        if (i)
            fputs(", ", out);
        emit_expr(out, e->as.call.args[i]);
    }
    fputc(')', out);
}

/*
 * Function overview: emit_expr
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit expr".
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
static void emit_expr(FILE *out, const Expr *e)
{
    switch (e->kind)
    {
    case EXPR_INT:
        fprintf(out, "%lldLL", e->as.int_value);
        break;
    case EXPR_FLOAT:
        fprintf(out, "%.17g", e->as.float_value);
        break;
    case EXPR_STRING:
        fprintf(out, "\"%s\"", e->as.text);
        break;
    case EXPR_BOOL:
        fputs(e->as.bool_value ? "1" : "0", out);
        break;
    case EXPR_IDENTIFIER:
        fputs(e->as.text, out);
        break;
    case EXPR_UNARY:
        /* we implement a function to emit an expression in C code,
         *           which handles different kinds of expressions such as literals, identifiers,
         *           unary operations, binary operations, function calls, indexing, array literals, and field accesses.
         *           this function uses a switch statement to determine the kind of expression
         *           and emits the corresponding C code for each case, recursively calling itself to emit sub-expressions as needed.
         *           by handling each kind of expression appropriately, we can generate correct and
         *           efficient C code that represents the semantics of the original expression in our programming language. */
        if (!strcmp(e->as.unary.op, "@"))
        {
            fputs("(&(", out);
            emit_expr(out, e->as.unary.operand);
            fputs("))", out);
        }
        else if (!strcmp(e->as.unary.op, "^"))
        {
            fputs("(*((long long *)", out);
            emit_expr(out, e->as.unary.operand);
            fputs("))", out);
        }
        else
        {
            fprintf(out, "(%s", e->as.unary.op);
            emit_expr(out, e->as.unary.operand);
            fputc(')', out);
        }
        break;
    case EXPR_INDEX:
        if (e->inferred_type.kind == TYPE_STR)
        {
            fputs("machine_index_str(", out);
            emit_expr(out, e->as.index.base);
            fputs(", ", out);
            emit_expr(out, e->as.index.index);
            fputc(')', out);
        }
        else if (e->as.index.base->inferred_type.kind == TYPE_ARRAY)
        {
            if (e->inferred_type.kind == TYPE_ARRAY)
            {
                fputs("((MachineArray *)machine_array_get(", out);
                emit_expr(out, e->as.index.base);
                fputs(", ", out);
                emit_expr(out, e->as.index.index);
                fputs("))", out);
            }
            else
            {
                fputs("machine_array_get(", out);
                emit_expr(out, e->as.index.base);
                fputs(", ", out);
                emit_expr(out, e->as.index.index);
                fputc(')', out);
            }
        }
        else
        {
            fputs("machine_list_get(", out);
            emit_expr(out, e->as.index.base);
            fputs(", ", out);
            emit_expr(out, e->as.index.index);
            fputc(')', out);
        }
        break;
    case EXPR_ARRAY:
        fputs("({ MachineArray *_tmp = machine_array_new(); ", out);
        for (size_t i = 0; i < e->as.array.item_count; ++i)
        {
            fputs("machine_array_push(_tmp, (long long)(", out);
            emit_expr(out, e->as.array.items[i]);
            fputs(")); ", out);
        }
        fputs("_tmp; })", out);
        break;
    case EXPR_FIELD:
        emit_expr(out, e->as.field.base);
        fprintf(out, ".%s", e->as.field.field);
        break;
    case EXPR_CALL:
        emit_call(out, e);
        break;
    case EXPR_BINARY:
        if (!strcmp(e->as.binary.op, "+") && e->inferred_type.kind == TYPE_STR)
        {
            fputs("machine_concat(", out);
            emit_expr(out, e->as.binary.left);
            fputs(", ", out);
            emit_expr(out, e->as.binary.right);
            fputc(')', out);
            break;
        }
        if ((!strcmp(e->as.binary.op, "==") || !strcmp(e->as.binary.op, "!=")) && e->as.binary.left->inferred_type.kind == TYPE_STR && e->as.binary.right->inferred_type.kind == TYPE_STR)
        {
            fputs("(strcmp(", out);
            emit_expr(out, e->as.binary.left);
            fputs(", ", out);
            emit_expr(out, e->as.binary.right);
            fprintf(out, ") %s 0)", !strcmp(e->as.binary.op, "==") ? "==" : "!=");
            break;
        }
        fputc('(', out);
        emit_expr(out, e->as.binary.left);
        fprintf(out, " %s ", e->as.binary.op);
        emit_expr(out, e->as.binary.right);
        fputc(')', out);
        break;
    }
}

/* we implement a function to emit a print statement in C code,
 *   which handles printing values of different types using the appropriate format specifiers.
 *   this function takes an expression representing the value to be printed and emits a
 *   printf statement that formats the output according to the inferred type of the expression,
 *   ensuring that integers, floating-point numbers, booleans, and other types are printed correctly.
 *   by using this function, we can generate C code that produces the expected
 *   output when executing print statements in our programming language. */
/*
 * Function overview: emit_print
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit print".
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
static void emit_print(FILE *out, const Expr *e, int level)
{
    indent(out, level);
    if (g_codegen_target_is_freestanding)
    {
        switch (e->inferred_type.kind)
        {
        case TYPE_I64:
        case TYPE_BOOL:
            fputs("machine_print_i64((long long)", out);
            emit_expr(out, e);
            fputs(");\n", out);
            break;
        case TYPE_F64:
            fputs("machine_print_f64((double)", out);
            emit_expr(out, e);
            fputs(");\n", out);
            break;
        case TYPE_HP:
            fputs("machine_print_hp((long double)", out);
            emit_expr(out, e);
            fputs(");\n", out);
            break;
        default:
            fputs("machine_print_str(", out);
            emit_expr(out, e);
            fputs(");\n", out);
            break;
        }
        return;
    }
    switch (e->inferred_type.kind)
    {
    case TYPE_I64:
    case TYPE_BOOL:
        fputs("printf(\"%lld\\n\", (long long)", out);
        emit_expr(out, e);
        fputs(");\n", out);
        break;
    case TYPE_F64:
        fputs("printf(\"%.17g\\n\", (double)", out);
        emit_expr(out, e);
        fputs(");\n", out);
        break;
    case TYPE_HP:
        fputs("printf(\"%.21Lg\\n\", (long double)", out);
        emit_expr(out, e);
        fputs(");\n", out);
        break;
    default:
        fputs("printf(\"%s\\n\", ", out);
        emit_expr(out, e);
        fputs(");\n", out);
        break;
    }
}

static void emit_stmt(FILE *out, const Statement *s, int level);

/*
 * Function overview: emit_block
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit block".
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
static void emit_block(FILE *out, const Statement *block, size_t count, int level)
{
    for (size_t i = 0; i < count; ++i)
        emit_stmt(out, &block[i], level);
}

/* we implement a function to emit a statement in C code, which handles different
 *   kinds of statements such as variable declarations, assignments, print statements, return statements,
 *   if statements, while loops, expression statements, labels, goto statements, and switch statements.
 *   this function uses a switch statement to determine the kind of
 *   statement and emits the corresponding C code for each case, recursively calling itself to
 *   emit sub-statements as needed (e.g., for if and while blocks).
 *   by handling each kind of statement appropriately, we can generate correct and
 *   efficient C code that represents the semantics of the original statement in our programming language. */
/*
 * Function overview: emit_stmt
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit stmt".
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
static void emit_stmt(FILE *out, const Statement *s, int level)
{
    char tbuf[128];
    switch (s->kind)
    {
    case STMT_VAR:
    case STMT_CONST:
        indent(out, level);
        fprintf(out, "%s %s = ", c_type(s->as.var_stmt.declared_type, tbuf, sizeof(tbuf)), s->as.var_stmt.name);
        if (s->as.var_stmt.has_initializer)
            emit_expr(out, s->as.var_stmt.initializer);
        else
            emit_zero_init(out, s->as.var_stmt.declared_type);
        fputs(";\n", out);
        break;
    case STMT_ASSIGN:
        indent(out, level);
        /* we handle assignment statements in C code, including special handling for array
         *           element assignments to use the appropriate machine runtime function for setting array values.
         *           if the target of the assignment is an index expression on an array, we emit a
         *           call to machine_array_set with the base array, index, and value. Otherwise,
         *           we emit a standard assignment statement. */
        if (s->as.assign_stmt.target->kind == EXPR_INDEX && s->as.assign_stmt.target->as.index.base->inferred_type.kind == TYPE_ARRAY)
        {
            fputs("machine_array_set(", out);
            emit_expr(out, s->as.assign_stmt.target->as.index.base);
            fputs(", ", out);
            emit_expr(out, s->as.assign_stmt.target->as.index.index);
            fputs(", ", out);
            emit_expr(out, s->as.assign_stmt.value);
            fputs(");\n", out);
        }
        else
        {
            emit_expr(out, s->as.assign_stmt.target);
            fputs(" = ", out);
            emit_expr(out, s->as.assign_stmt.value);
            fputs(";\n", out);
        }
        break;
    case STMT_PRINT:
        emit_print(out, s->as.print_stmt.value, level);
        break;
    case STMT_RETURN:
        indent(out, level);
        if (s->as.return_stmt.value)
        {
            fputs("return ", out);
            emit_expr(out, s->as.return_stmt.value);
            fputs(";\n", out);
        }
        else
            fputs("return;\n", out);
        break;
    case STMT_EXPR:
        indent(out, level);
        emit_expr(out, s->as.expr_stmt.expr);
        fputs(";\n", out);
        break;
    case STMT_LABEL:
        if (level > 0)
            indent(out, level - 1);
        fprintf(out, "%s:\n", s->as.label_stmt.name);
        break;
    case STMT_GOTO:
        indent(out, level);
        fprintf(out, "goto %s;\n", s->as.goto_stmt.name);
        break;
    /* we handle control flow statements such as if statements, while loops,
     *       and switch statements, emitting the appropriate C code for each case.
     *       for if statements, we emit the condition and then recursively emit the
     *       then and else blocks with proper indentation and braces. For while loops,
     *       we emit the condition and the loop body similarly. For switch statements,
     *       we emit the switch value and then iterate through the cases, emitting each case label and
     *       its corresponding block of statements, including a break statement at the end of each case. */
    case STMT_IF:
        indent(out, level);
        fputs("if (", out);
        emit_expr(out, s->as.if_stmt.condition);
        fputs(") {\n", out);
        emit_block(out, s->as.if_stmt.then_block, s->as.if_stmt.then_count, level + 1);
        indent(out, level);
        fputs("}", out);
        if (s->as.if_stmt.else_count > 0)
        {
            fputs(" else {\n", out);
            emit_block(out, s->as.if_stmt.else_block, s->as.if_stmt.else_count, level + 1);
            indent(out, level);
            fputs("}\n", out);
        }
        else
            fputs("\n", out);
        break;
    case STMT_WHILE:
        indent(out, level);
        fputs("while (", out);
        emit_expr(out, s->as.while_stmt.condition);
        fputs(") {\n", out);
        emit_block(out, s->as.while_stmt.body, s->as.while_stmt.body_count, level + 1);
        indent(out, level);
        fputs("}\n", out);
        break;
    case STMT_UNSAFE:
        indent(out, level);
        fputs("/* unsafe */ {\n", out);
        emit_block(out, s->as.unsafe_stmt.body, s->as.unsafe_stmt.body_count, level + 1);
        indent(out, level);
        fputs("}\n", out);
        break;
    case STMT_SWITCH:
        indent(out, level);
        fputs("switch ((long long)(", out);
        emit_expr(out, s->as.switch_stmt.value);
        fputs(")) {\n", out);
        for (size_t i = 0; i < s->as.switch_stmt.case_count; ++i)
        {
            const SwitchCase *c = &s->as.switch_stmt.cases[i];
            indent(out, level);
            if (c->is_default)
                fputs("default:\n", out);
            else
            {
                fputs("case ", out);
                emit_expr(out, c->match);
                fputs(":\n", out);
            }
            emit_block(out, c->body, c->body_count, level + 1);
            indent(out, level + 1);
            fputs("break;\n", out);
            /* we emit the cases for a switch statement, including the case labels (or default label) and
             *            the corresponding blocks of statements for each case, ensuring that each case
             *            ends with a break statement to prevent fall-through. */
        }
        indent(out, level);
        fputs("}\n", out);
        break;
    }
}

/* we implement a function to generate a C file from the given program,
 *   which includes emitting the necessary type definitions, global variable declarations,
 *   and function definitions in C code.
 *   this function takes the program representation, the output file path for the generated C code,
 *   and a list to collect any diagnostics (errors) that occur during code generation.
 *   it opens the output file for writing, emits the required C code for structs, globals,
 *   and functions, and handles any errors that may arise during file operations or code generation,
 *   adding appropriate diagnostic messages to the provided list. */
/*
 * Function overview: generate_c_file
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "generate c file".
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
bool generate_c_file(const Program *program, const char *output_path, DiagnosticList *errors)
{
    FILE *out = fopen(output_path, "w");
    char tbuf[128];
    if (!out)
    {
        diagnostics_add(NULL, errors, 1, 1, "failed to open generated C file '%s'", output_path);
        return false;
    }
    g_codegen_target_is_freestanding = (program->target_id == MACHINE_TARGET_FREESTANDING_X86_64);
    if (g_codegen_target_is_freestanding)
        fputs("#include \"machine_runtime_freestanding.h\"\n\n", out);
    else
        fputs("#include \"machine_runtime.h\"\n\n", out);
    emit_structs(out, program);
    emit_globals(out, program);
    if (program->global_count > 0)
    {
        fputs("static void machine_init_globals(void) {\n", out);
        for (size_t i = 0; i < program->global_count; ++i)
        {
            const GlobalVarDecl *g = &program->globals[i];
            fputs("    ", out);
            fprintf(out, "%s = ", g->name);
            if (g->has_initializer)
                emit_expr(out, g->initializer);
            else
                emit_zero_init(out, g->declared_type);
            fputs(";\n", out);
        }
        fputs("}\n\n", out);
    }
    for (size_t i = 0; i < program->function_count; ++i)
    {
        const FunctionDecl *f = &program->functions[i];
        fprintf(out, "%s %s(", (f->is_main && g_codegen_target_is_freestanding) ? "long long" : (f->is_main ? "int" : c_type(f->return_type, tbuf, sizeof(tbuf))), f->is_main ? (g_codegen_target_is_freestanding ? "machine_user_main" : "main") : f->name);
        if (f->is_main)
        {
            fputs("void", out);
        }
        else
        {
            for (size_t j = 0; j < f->param_count; ++j)
            {
                if (j)
                    fputs(", ", out);
                fprintf(out, "%s %s", c_type(f->params[j].type, tbuf, sizeof(tbuf)), f->params[j].name);
            }
        }
        fputs(") {\n", out);
        if (f->is_main && program->global_count > 0)
        {
            indent(out, 1);
            fputs("machine_init_globals();\n", out);
        }
        emit_block(out, f->body, f->body_count, 1);
        if (f->return_type.kind == TYPE_I64 && f->is_main && !g_codegen_target_is_freestanding)
        {
            indent(out, 1);
            fputs("return 0;\n", out);
        }
        fputs("}\n\n", out);
    }
    /* we iterate through the list of function declarations in the program and emit the
     *       corresponding C function definitions for each one, including the function
     *       signature (return type and parameters) and the function body.
     *       for the main function, we ensure that it has a return type of int and takes no parameters,
     *       and we also emit a call to initialize global variables if there are any.
     *       by generating C code for each function in this way, we can produce a
     *       complete C source file that represents the functionality of the original program in our programming language. */
    fclose(out);
    return true;
}

typedef struct
{
    char object_path[PATH_MAX];
    char include_dir[PATH_MAX];
    char runtime_source[PATH_MAX];
    int has_source;
} RuntimeBundle;

typedef struct
{
    char include_dir[PATH_MAX];
    char runtime_source[PATH_MAX];
    char entry_source[PATH_MAX];
} FreestandingBundle;

static char *format_alloc(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int needed = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (needed < 0)
        return NULL;
    char *buf = (char *)malloc((size_t)needed + 1);
    if (!buf)
        return NULL;
    va_start(ap, fmt);
    vsnprintf(buf, (size_t)needed + 1, fmt, ap);
    va_end(ap);
    return buf;
}

static char *shell_quote_single(const char *value)
{
    size_t extra = 2;
    for (const char *p = value; *p; ++p)
        extra += (*p == '\'') ? 4 : 1;
    char *out = (char *)malloc(extra + 1);
    char *w = out;
    if (!out)
        return NULL;
    *w++ = '\'';
    for (const char *p = value; *p; ++p)
    {
        if (*p == '\'')
        {
            memcpy(w, "'\\''", 4);
            w += 4;
        }
        else
            *w++ = *p;
    }
    *w++ = '\'';
    *w = '\0';
    return out;
}

/*
 * Function overview: file_contains_text
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "file contains text".
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
static int file_contains_text(const char *path, const char *needle)
{
    char *buffer = NULL;
    size_t length = 0;
    int found = 0;
    if (!read_text_file(path, &buffer, &length))
        return 0;
    (void)length;
    if (strstr(buffer, needle) != NULL)
        found = 1;
    free_text_file(buffer);
    return found;
}

/*
 * Function overview: runtime_header_is_compatible
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "runtime header is compatible".
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
static int runtime_header_is_compatible(const char *include_dir)
{
    char header_path[PATH_MAX];
    char version_line[128];
    static const char *required_symbols[] = {
        "machine_ptr_to_i64(",
        "machine_ptr_offset(",
        "machine_ptr_hex(",
        "machine_ptr_bin(",
        "machine_volatile_store_u32(",
        "machine_volatile_load_u32(",
        "machine_mmap_anon(",
        "machine_munmap_mem(",
        "machine_asm_rdtsc(",
        NULL};
    if (!join_cstr3(header_path, sizeof(header_path), include_dir, "/", "machine_runtime.h"))
        return 0;
    if (!file_exists(header_path))
        return 0;
    snprintf(version_line, sizeof(version_line), "#define MACHINE_RUNTIME_API_VERSION %d", MACHINE_RUNTIME_API_VERSION);
    if (file_contains_text(header_path, version_line))
        return 1;
    for (size_t i = 0; required_symbols[i]; ++i)
        if (!file_contains_text(header_path, required_symbols[i]))
            return 0;
    return 1;
}

/*
 * Function overview: run_compiler_command
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "run compiler command".
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
static int run_compiler_command(DiagnosticList *errors, const char *binary_path, const char *message, const char *command)
{
    if (!command)
    {
        diagnostics_add(NULL, errors, 1, 1, "failed to build compiler command while building '%s'", binary_path);
        return 0;
    }
    if (system(command) != 0)
    {
        diagnostics_add(NULL, errors, 1, 1, "%s", message ? message : "compiler command failed");
        return 0;
    }
    return 1;
}

/*
 * Function overview: set_runtime_bundle
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "set runtime bundle".
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
static int set_runtime_bundle(RuntimeBundle *bundle, const char *obj, const char *incdir, const char *source)
{
    if (!bundle || !obj || !incdir)
        return 0;
    if (!copy_cstr(bundle->object_path, sizeof(bundle->object_path), obj))
        return 0;
    if (!copy_cstr(bundle->include_dir, sizeof(bundle->include_dir), incdir))
        return 0;
    bundle->has_source = 0;
    bundle->runtime_source[0] = '\0';
    if (source && *source)
    {
        if (!copy_cstr(bundle->runtime_source, sizeof(bundle->runtime_source), source))
            return 0;
        bundle->has_source = 1;
    }
    return 1;
}

/*
 * Function overview: choose_runtime_candidate
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "choose runtime candidate".
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
static int choose_runtime_candidate(RuntimeBundle *bundle,
                                    const char *obj_path,
                                    const char *include_dir,
                                    const char *runtime_source)
{
    if (!file_exists(obj_path))
    {
        if (!runtime_source || !*runtime_source || !file_exists(runtime_source))
            return 0;
    }
    if (!file_exists(obj_path) && (!runtime_source || !*runtime_source || !file_exists(runtime_source)))
        return 0;
    if (!runtime_header_is_compatible(include_dir))
        return 0;
    return set_runtime_bundle(bundle, obj_path, include_dir, runtime_source);
}

/*
 * Function overview: choose_runtime_bundle
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "choose runtime bundle".
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
static int choose_runtime_bundle(RuntimeBundle *bundle, const char *source_path, int prefer_system_runtime)
{
    char cwd[PATH_MAX] = {0};
    char source_dir[PATH_MAX] = {0};
    char self_path[PATH_MAX] = {0};
    char self_dir[PATH_MAX] = {0};
    char installed_obj[] = "/usr/local/lib/machine/machine_runtime.o";
    char installed_inc[] = "/usr/local/include/machine";
    char paths[10][3][PATH_MAX];
    size_t count = 0;

    if (!getcwd(cwd, sizeof(cwd)))
        copy_cstr(cwd, sizeof(cwd), ".");
    if (!path_dirname(source_path, source_dir, sizeof(source_dir)))
        copy_cstr(source_dir, sizeof(source_dir), ".");
    if (readlink("/proc/self/exe", self_path, sizeof(self_path) - 1) > 0)
        path_dirname(self_path, self_dir, sizeof(self_dir));

#define ADD(obj, inc, src)                                          \
    do                                                              \
    {                                                               \
        copy_cstr(paths[count][0], sizeof(paths[count][0]), (obj)); \
        copy_cstr(paths[count][1], sizeof(paths[count][1]), (inc)); \
        copy_cstr(paths[count][2], sizeof(paths[count][2]), (src)); \
        ++count;                                                    \
    } while (0)

    if (prefer_system_runtime)
    {
        ADD(installed_obj, installed_inc, "");
        {
            char obj[PATH_MAX], inc[PATH_MAX], src[PATH_MAX];
            join_cstr3(obj, sizeof(obj), source_dir, "/build/", "machine_runtime.o");
            join_cstr3(inc, sizeof(inc), source_dir, "/build", "");
            join_cstr3(src, sizeof(src), source_dir, "/machine_runtime.c", "");
            ADD(obj, inc, src);
            join_cstr3(obj, sizeof(obj), source_dir, "/machine_runtime.o", "");
            join_cstr3(inc, sizeof(inc), source_dir, "", "");
            ADD(obj, inc, src);
            join_cstr3(obj, sizeof(obj), cwd, "/build/", "machine_runtime.o");
            join_cstr3(inc, sizeof(inc), cwd, "/build", "");
            join_cstr3(src, sizeof(src), cwd, "/src/runtime.c", "");
            ADD(obj, inc, src);
            join_cstr3(obj, sizeof(obj), cwd, "/build/", "machine_runtime.o");
            join_cstr3(inc, sizeof(inc), cwd, "/include", "");
            ADD(obj, inc, src);
            if (self_dir[0])
            {
                join_cstr3(obj, sizeof(obj), self_dir, "/build/", "machine_runtime.o");
                join_cstr3(inc, sizeof(inc), self_dir, "/include", "");
                join_cstr3(src, sizeof(src), self_dir, "/src/runtime.c", "");
                ADD(obj, inc, src);
            }
        }
    }
    else
    {
        {
            char obj[PATH_MAX], inc[PATH_MAX], src[PATH_MAX];
            join_cstr3(obj, sizeof(obj), cwd, "/build/", "machine_runtime.o");
            join_cstr3(inc, sizeof(inc), cwd, "/include", "");
            join_cstr3(src, sizeof(src), cwd, "/src/runtime.c", "");
            ADD(obj, inc, src);
            join_cstr3(obj, sizeof(obj), cwd, "/build/", "machine_runtime.o");
            join_cstr3(inc, sizeof(inc), cwd, "/build", "");
            ADD(obj, inc, src);
            join_cstr3(obj, sizeof(obj), cwd, "/machine_runtime.o", "");
            join_cstr3(inc, sizeof(inc), cwd, "", "");
            ADD(obj, inc, src);
        }
        ADD(installed_obj, installed_inc, "");
        {
            char obj[PATH_MAX], inc[PATH_MAX], src[PATH_MAX];
            join_cstr3(obj, sizeof(obj), source_dir, "/build/", "machine_runtime.o");
            join_cstr3(inc, sizeof(inc), source_dir, "/build", "");
            join_cstr3(src, sizeof(src), source_dir, "/machine_runtime.c", "");
            ADD(obj, inc, src);
            join_cstr3(obj, sizeof(obj), source_dir, "/machine_runtime.o", "");
            join_cstr3(inc, sizeof(inc), source_dir, "", "");
            ADD(obj, inc, src);
            if (self_dir[0])
            {
                join_cstr3(obj, sizeof(obj), self_dir, "/build/", "machine_runtime.o");
                join_cstr3(inc, sizeof(inc), self_dir, "/include", "");
                join_cstr3(src, sizeof(src), self_dir, "/src/runtime.c", "");
                ADD(obj, inc, src);
            }
        }
    }
#undef ADD

    for (size_t i = 0; i < count; ++i)
    {
        if (choose_runtime_candidate(bundle, paths[i][0], paths[i][1], paths[i][2]))
            return 1;
    }
    return 0;
}

/*
 * Function overview: set_freestanding_bundle
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "set freestanding bundle".
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
static int set_freestanding_bundle(FreestandingBundle *bundle, const char *incdir, const char *runtime_source, const char *entry_source)
{
    if (!bundle || !incdir || !runtime_source || !entry_source)
        return 0;
    if (!copy_cstr(bundle->include_dir, sizeof(bundle->include_dir), incdir))
        return 0;
    if (!copy_cstr(bundle->runtime_source, sizeof(bundle->runtime_source), runtime_source))
        return 0;
    if (!copy_cstr(bundle->entry_source, sizeof(bundle->entry_source), entry_source))
        return 0;
    return 1;
}

/*
 * Function overview: choose_freestanding_support_bundle
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "choose freestanding support bundle".
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
static int choose_freestanding_support_bundle(FreestandingBundle *bundle, const char *source_path)
{
    char cwd[PATH_MAX] = {0};
    char source_dir[PATH_MAX] = {0};
    char self_path[PATH_MAX] = {0};
    char self_dir[PATH_MAX] = {0};
    struct Candidate
    {
        char inc[PATH_MAX];
        char runtime[PATH_MAX];
        char entry[PATH_MAX];
    } candidates[8];
    size_t count = 0;

    if (!getcwd(cwd, sizeof(cwd)))
        copy_cstr(cwd, sizeof(cwd), ".");
    if (!path_dirname(source_path, source_dir, sizeof(source_dir)))
        copy_cstr(source_dir, sizeof(source_dir), ".");
    ssize_t link_len = readlink("/proc/self/exe", self_path, sizeof(self_path) - 1);
    if (link_len > 0)
    {
        self_path[link_len] = '\0';
        path_dirname(self_path, self_dir, sizeof(self_dir));
    }

#define ADD_FS(base_inc, runtime_src, entry_src)                                                \
    do                                                                                          \
    {                                                                                           \
        copy_cstr(candidates[count].inc, sizeof(candidates[count].inc), (base_inc));            \
        copy_cstr(candidates[count].runtime, sizeof(candidates[count].runtime), (runtime_src)); \
        copy_cstr(candidates[count].entry, sizeof(candidates[count].entry), (entry_src));       \
        ++count;                                                                                \
    } while (0)

    ADD_FS("/usr/local/include/machine", "/usr/local/lib/machine/machine_runtime_freestanding.c", "/usr/local/lib/machine/machine_runtime_freestanding_entry.S");
    {
        char inc[PATH_MAX], runtime_src[PATH_MAX], entry_src[PATH_MAX];
        join_cstr3(inc, sizeof(inc), cwd, "/include", "");
        join_cstr3(runtime_src, sizeof(runtime_src), cwd, "/src/", "runtime_freestanding.c");
        join_cstr3(entry_src, sizeof(entry_src), cwd, "/src/", "runtime_freestanding_entry.S");
        ADD_FS(inc, runtime_src, entry_src);
        join_cstr3(inc, sizeof(inc), source_dir, "/include", "");
        join_cstr3(runtime_src, sizeof(runtime_src), source_dir, "/src/", "runtime_freestanding.c");
        join_cstr3(entry_src, sizeof(entry_src), source_dir, "/src/", "runtime_freestanding_entry.S");
        ADD_FS(inc, runtime_src, entry_src);
        if (self_dir[0])
        {
            join_cstr3(inc, sizeof(inc), self_dir, "/include", "");
            join_cstr3(runtime_src, sizeof(runtime_src), self_dir, "/src/", "runtime_freestanding.c");
            join_cstr3(entry_src, sizeof(entry_src), self_dir, "/src/", "runtime_freestanding_entry.S");
            ADD_FS(inc, runtime_src, entry_src);
        }
    }
#undef ADD_FS

    for (size_t i = 0; i < count; ++i)
    {
        char header_path[PATH_MAX];
        if (!join_cstr3(header_path, sizeof(header_path), candidates[i].inc, "/", "machine_runtime_freestanding.h"))
            continue;
        if (!file_exists(header_path) || !file_exists(candidates[i].runtime) || !file_exists(candidates[i].entry))
            continue;
        return set_freestanding_bundle(bundle, candidates[i].inc, candidates[i].runtime, candidates[i].entry);
    }
    return 0;
}

/*
 * Function overview: compile_c_to_binary
 *
 * High-level purpose:
 * - This routine belongs to codegen.c.
 * - It exists to generate c output, manage runtime selection, and drive native compilation/linking for hosted and special targets.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "compile c to binary".
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
bool compile_c_to_binary(const char *c_path, const char *binary_path, const char *source_path, const MachineCompileOptions *options, DiagnosticList *errors)
{
    char runtime_tmp_obj[PATH_MAX] = {0};
    const char *runtime_object_for_link = NULL;
    RuntimeBundle bundle = {0};
    FreestandingBundle fs_bundle = {0};
    MachineCompileOptions local_options = {0};
    int prefer_system_runtime = 0;
    char *quoted_include = NULL;
    char *quoted_c = NULL;
    char *quoted_runtime_obj = NULL;
    char *quoted_binary = NULL;
    char *quoted_runtime_src = NULL;
    char *quoted_entry_src = NULL;
    char *command = NULL;
    int ok = 0;

    local_options.target = MACHINE_TARGET_LINUX_HOSTED;
    local_options.backend = MACHINE_BACKEND_C;
    if (options)
        local_options = *options;
    prefer_system_runtime = local_options.prefer_system_runtime;

    if (local_options.backend != MACHINE_BACKEND_C)
    {
        diagnostics_add(NULL, errors, 1, 1,
                        "compile_c_to_binary only handles backend.c; route asm through compile_asm_to_binary");
        return false;
    }
    if (local_options.target == MACHINE_TARGET_BAREMETAL_X86_64)
    {
        diagnostics_add(NULL, errors, 1, 1,
                        "backend.c does not emit a baremetal artifact yet; use --backend x86_64-asm for baremetal-x86_64 in compiler iteration %s",
                        MACHINE_ITERATION_VERSION);
        return false;
    }

    if (local_options.target == MACHINE_TARGET_FREESTANDING_X86_64)
    {
        if (!choose_freestanding_support_bundle(&fs_bundle, source_path))
        {
            diagnostics_add(NULL, errors, 1, 1,
                            "no freestanding Machine runtime support files were found for '%s'",
                            source_path);
            return false;
        }
        quoted_include = shell_quote_single(fs_bundle.include_dir);
        quoted_c = shell_quote_single(c_path);
        quoted_runtime_src = shell_quote_single(fs_bundle.runtime_source);
        quoted_entry_src = shell_quote_single(fs_bundle.entry_source);
        quoted_binary = shell_quote_single(binary_path);
        command = format_alloc(
            "cc -std=gnu17 -Wall -Wextra -Wpedantic -Werror -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -no-pie -O2 -I%s %s %s %s -o %s -lgcc",
            quoted_include, quoted_c, quoted_runtime_src, quoted_entry_src, quoted_binary);
        {
            char *message = format_alloc("freestanding C build failed while building '%s'", binary_path);
            int ran = run_compiler_command(errors, binary_path, message ? message : "freestanding C build failed", command);
            free(message);
            if (!ran)
                goto cleanup;
        }
        ok = 1;
        goto cleanup;
    }

    if (!choose_runtime_bundle(&bundle, source_path, prefer_system_runtime))
    {
        diagnostics_add(NULL, errors, 1, 1,
                        "no compatible Machine runtime bundle was found; searched installed runtime plus local project/runtime copies for '%s'",
                        source_path);
        return false;
    }

    runtime_object_for_link = bundle.object_path;
    if (!file_exists(bundle.object_path))
    {
        if (!append_cstr_suffix(runtime_tmp_obj, sizeof(runtime_tmp_obj), binary_path, ".machine_runtime_tmp.o"))
        {
            diagnostics_add(NULL, errors, 1, 1, "temporary runtime object path is too long");
            return false;
        }
        quoted_include = shell_quote_single(bundle.include_dir);
        quoted_runtime_src = shell_quote_single(bundle.runtime_source);
        quoted_runtime_obj = shell_quote_single(runtime_tmp_obj);
        command = format_alloc(
            "cc -std=gnu17 -Wall -Wextra -Werror -O2 -I%s -c %s -o %s $(pkg-config --cflags sdl2 SDL2_image 2>/dev/null || true)",
            quoted_include, quoted_runtime_src, quoted_runtime_obj);
        {
            char *message = format_alloc("failed to build local Machine runtime object from '%s'", bundle.runtime_source);
            int ran = run_compiler_command(errors, binary_path, message ? message : "failed to build local Machine runtime object", command);
            free(message);
            if (!ran)
                goto cleanup;
        }
        free(command);
        command = NULL;
        free(quoted_include);
        quoted_include = NULL;
        free(quoted_runtime_src);
        quoted_runtime_src = NULL;
        free(quoted_runtime_obj);
        quoted_runtime_obj = NULL;
        runtime_object_for_link = runtime_tmp_obj;
    }

    quoted_include = shell_quote_single(bundle.include_dir);
    quoted_c = shell_quote_single(c_path);
    quoted_runtime_obj = shell_quote_single(runtime_object_for_link);
    quoted_binary = shell_quote_single(binary_path);
    command = format_alloc(
        "cc -std=gnu17 -Wall -Wextra -Werror -O2 -I%s %s %s -o %s -lm $(pkg-config --cflags --libs sdl2 SDL2_image 2>/dev/null || true)",
        quoted_include, quoted_c, quoted_runtime_obj, quoted_binary);
    {
        char *message = format_alloc("system C compiler failed while building '%s'", binary_path);
        int ran = run_compiler_command(errors, binary_path, message ? message : "system C compiler failed", command);
        free(message);
        if (!ran)
            goto cleanup;
    }

    ok = 1;

cleanup:
    if (runtime_tmp_obj[0])
        remove(runtime_tmp_obj);
    free(quoted_include);
    free(quoted_c);
    free(quoted_runtime_obj);
    free(quoted_binary);
    free(quoted_runtime_src);
    free(quoted_entry_src);
    free(command);
    return ok ? true : false;
}
