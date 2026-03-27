/*
 * Annotated reading edition of parser_internal.h
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
#ifndef MACHINE_PARSER_INTERNAL_H
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_PARSER_INTERNAL_H

/*
 * Dependency include.
 *
 * This brings in declarations required by the current header.
 */
#include "parser.h"
/*
 * Dependency include.
 *
 * This brings in declarations required by the current header.
 */
#include "util.h"

/*
 * Structure declaration.
 *
 * Structures in this project carry parser state, token records, AST nodes, type information, or runtime-facing data.
 */
typedef struct
{
    const SourceFile *src;
    const TokenList *tokens;
    size_t index;
    Program *program;
    DiagnosticList *errors;
    DiagnosticList *warnings;
    FunctionDecl *current_function;
    char current_module[64];
    Symbol symbols[MACHINE_MAX_SYMBOLS];
    size_t symbol_count;
    int allow_unsafe;
    int unsafe_depth;
} Parser;

/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
const Token *cur(Parser *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
const Token *prev(Parser *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
bool match(Parser *p, TokenType t);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
bool expect(Parser *p, TokenType t, const char *msg);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
Expr *new_expr(Parser *p, ExprKind kind, int line, int column);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
Symbol *find_symbol(Parser *p, const char *name);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
FunctionDecl *find_function(Program *program, const char *name);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
StructDecl *find_struct(Program *program, const char *name);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
ModuleDecl *find_module(Program *program, const char *name);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
GlobalVarDecl *find_global(Program *program, const char *name);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
StructField *find_struct_field(StructDecl *sd, const char *field);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
int is_reserved_name(const char *name);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
TypeRef make_type(MachineType kind, const char *struct_name);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
int type_equals(TypeRef a, TypeRef b);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
const char *type_display_name(TypeRef t, char *buf, size_t n);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
int add_local(Parser *p, const char *name, TypeRef type, int line);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
int parse_type_ref(Parser *p, TypeRef *out);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
Expr *parse_expression(Parser *p);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
TypeRef infer_expr(Parser *p, Expr *e);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
int parse_block(Parser *p, Statement **out, size_t *out_count);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
int is_unsafe_builtin_name(const char *name);

/*
 * Preprocessor directive.
 *
 * Directives here usually define compile-time constants, feature switches, or version identifiers.
 */
#endif
