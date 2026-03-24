#include "codegen.h"
#include "util.h"
#include "machine_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void indent(FILE *out, int level)
{
    for (int i = 0; i < level; ++i)
    {
        fputs("    ", out);
    }
}

/* we implement a set of helper functions for code generation, including functions to emit type definitions,
   global variable declarations, expressions, and statements in C code.
   these functions provide a convenient interface for generating C code from our intermediate representation of
   the program, allowing us to produce readable and efficient C code that corresponds to the semantics of our programming language. */
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

static void emit_zero_init(FILE *out, TypeRef t)
{
    /* we implement a function to emit the zero initialization value for a given type,
       which is used in code generation to initialize variables and fields to a default value.
       this function takes a TypeRef as input and writes the appropriate zero initialization value to
       the output file stream, handling different types such as integers, floating-point numbers, strings,
       pointers, lists, arrays, and structs accordingly. */
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
               this function iterates through the list of struct declarations in the program and
               generates a corresponding C struct definition for each one, ensuring that the field types are
               correctly translated to their C equivalents using the c_type helper function. */
        }
        fprintf(out, "} %s;\n\n", sd->name);
    }
}

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

static int call_name_from_expr(const Expr *callee, char *buf, size_t n)
{
    if (callee->kind == EXPR_IDENTIFIER)
        return copy_cstr(buf, n, callee->as.text);
    /* we implement a helper function to extract the name of a function being called from a call expression,
       which is used in code generation to determine how to emit calls to built-in functions and user-defined functions.
       this function takes an expression representing the callee of a call expression and
       attempts to extract the function name from it,
       handling cases where the callee is a simple identifier or a field access (e.g., struct method call).
       by using this function, we can identify calls to built-in functions and generate the
       appropriate C code for them, while also supporting user-defined functions and struct method calls in our programming language. */
    if (callee->kind == EXPR_FIELD && callee->as.field.base->kind == EXPR_IDENTIFIER)
        return join_cstr3(buf, n, callee->as.field.base->as.text, "__", callee->as.field.field);
    return 0;
}

static int is_machine_builtin_name(const char *name)
{
    static const char *builtins[] = {
        /* we implement a function to check if a given function name corresponds to a built-in function provided by the machine runtime,
           which is used in code generation to determine how to emit calls to these built-in functions.
           this function takes a function name as input and checks it against a list of known built-in function names,
           returning true if the name matches one of the built-ins and false otherwise.
           by using this function, we can ensure that calls to built-in functions are correctly identified and
           emitted with the appropriate C code that corresponds to their functionality in the machine runtime. */
        "hp_add", "hp_sub", "hp_mul", "hp_div", "hp_sqrt", "hp_pow",
        "alloc_bytes", "free_mem", "store_i64", "load_i64", "store_f64", "load_f64", "store_str", "load_str",
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

static void emit_call(FILE *out, const Expr *e)
{
    char name[128];
    if (!call_name_from_expr(e->as.call.callee, name, sizeof(name)))
    {
        fputs("/* unsupported call target */0", out);
        return;
        /* we implement a function to emit a function call expression in C code,
           which handles calls to built-in functions, user-defined functions, and struct constructors.
           this function first attempts to extract the function name from the call expression using the
           call_name_from_expr helper function, and if it fails to get a name,
           it emits a placeholder comment indicating an unsupported call target.
           if the function name is successfully extracted, the function checks if it matches any
           known built-in functions or struct constructors and emits the appropriate C code for those cases.
           for user-defined functions, it emits a standard function call using the extracted name and the emitted arguments.
           by handling different types of function calls in this way, we can generate correct and
           efficient C code that corresponds to the semantics of our programming language. */
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
           which handles different kinds of expressions such as literals, identifiers,
           unary operations, binary operations, function calls, indexing, array literals, and field accesses.
           this function uses a switch statement to determine the kind of expression
           and emits the corresponding C code for each case, recursively calling itself to emit sub-expressions as needed.
           by handling each kind of expression appropriately, we can generate correct and
           efficient C code that represents the semantics of the original expression in our programming language. */
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
   which handles printing values of different types using the appropriate format specifiers.
   this function takes an expression representing the value to be printed and emits a
   printf statement that formats the output according to the inferred type of the expression,
   ensuring that integers, floating-point numbers, booleans, and other types are printed correctly.
   by using this function, we can generate C code that produces the expected
   output when executing print statements in our programming language. */
static void emit_print(FILE *out, const Expr *e, int level)
{
    indent(out, level);
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

static void emit_block(FILE *out, const Statement *block, size_t count, int level)
{
    for (size_t i = 0; i < count; ++i)
        emit_stmt(out, &block[i], level);
}

/* we implement a function to emit a statement in C code, which handles different
   kinds of statements such as variable declarations, assignments, print statements, return statements,
   if statements, while loops, expression statements, labels, goto statements, and switch statements.
   this function uses a switch statement to determine the kind of
   statement and emits the corresponding C code for each case, recursively calling itself to
   emit sub-statements as needed (e.g., for if and while blocks).
   by handling each kind of statement appropriately, we can generate correct and
   efficient C code that represents the semantics of the original statement in our programming language. */
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
           element assignments to use the appropriate machine runtime function for setting array values.
           if the target of the assignment is an index expression on an array, we emit a
           call to machine_array_set with the base array, index, and value. Otherwise,
           we emit a standard assignment statement. */
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
       and switch statements, emitting the appropriate C code for each case.
       for if statements, we emit the condition and then recursively emit the
       then and else blocks with proper indentation and braces. For while loops,
       we emit the condition and the loop body similarly. For switch statements,
       we emit the switch value and then iterate through the cases, emitting each case label and
       its corresponding block of statements, including a break statement at the end of each case. */
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
            the corresponding blocks of statements for each case, ensuring that each case
            ends with a break statement to prevent fall-through. */
        }
        indent(out, level);
        fputs("}\n", out);
        break;
    }
}

/* we implement a function to generate a C file from the given program,
   which includes emitting the necessary type definitions, global variable declarations,
   and function definitions in C code.
   this function takes the program representation, the output file path for the generated C code,
   and a list to collect any diagnostics (errors) that occur during code generation.
   it opens the output file for writing, emits the required C code for structs, globals,
   and functions, and handles any errors that may arise during file operations or code generation,
   adding appropriate diagnostic messages to the provided list. */
bool generate_c_file(const Program *program, const char *output_path, DiagnosticList *errors)
{
    FILE *out = fopen(output_path, "w");
    char tbuf[128];
    if (!out)
    {
        diagnostics_add(NULL, errors, 1, 1, "failed to open generated C file '%s'", output_path);
        return false;
    }
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
        fprintf(out, "%s %s(", f->is_main ? "int" : c_type(f->return_type, tbuf, sizeof(tbuf)), f->is_main ? "main" : f->name);
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
        if (f->return_type.kind == TYPE_I64 && f->is_main)
        {
            indent(out, 1);
            fputs("return 0;\n", out);
        }
        fputs("}\n\n", out);
    }
    /* we iterate through the list of function declarations in the program and emit the
       corresponding C function definitions for each one, including the function
       signature (return type and parameters) and the function body.
       for the main function, we ensure that it has a return type of int and takes no parameters,
       and we also emit a call to initialize global variables if there are any.
       by generating C code for each function in this way, we can produce a
       complete C source file that represents the functionality of the original program in our programming language. */
    fclose(out);
    return true;
}

bool compile_c_to_binary(const char *c_path, const char *binary_path, DiagnosticList *errors)
{
    char command[4096];
    const char *runtime_local = "build/machine_runtime.o";
    const char *runtime_installed = "/usr/local/lib/machine/machine_runtime.o";
    const char *runtime_path = file_exists(runtime_local) ? runtime_local : runtime_installed;
    snprintf(command, sizeof(command),
             "cc -std=gnu17 -w -O2 -Iinclude -I/usr/local/include/machine '%s' '%s' -o '%s' -lm $(pkg-config --cflags --libs sdl2 SDL2_image 2>/dev/null || true)",
             c_path, runtime_path, binary_path);
    if (system(command) != 0)
    {
        diagnostics_add(NULL, errors, 1, 1, "system C compiler failed while building '%s'", binary_path);
        return false;
    }
    return true;
}
