#ifndef MACHINE_PARSER_INTERNAL_H
#define MACHINE_PARSER_INTERNAL_H

#include "parser.h"
#include "util.h"

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
} Parser;

const Token *cur(Parser *p);
const Token *prev(Parser *p);
bool match(Parser *p, TokenType t);
bool expect(Parser *p, TokenType t, const char *msg);
Expr *new_expr(Parser *p, ExprKind kind, int line, int column);
Symbol *find_symbol(Parser *p, const char *name);
FunctionDecl *find_function(Program *program, const char *name);
StructDecl *find_struct(Program *program, const char *name);
ModuleDecl *find_module(Program *program, const char *name);
GlobalVarDecl *find_global(Program *program, const char *name);
StructField *find_struct_field(StructDecl *sd, const char *field);
int is_reserved_name(const char *name);
TypeRef make_type(MachineType kind, const char *struct_name);
int type_equals(TypeRef a, TypeRef b);
const char *type_display_name(TypeRef t, char *buf, size_t n);
int add_local(Parser *p, const char *name, TypeRef type, int line);
int parse_type_ref(Parser *p, TypeRef *out);
Expr *parse_expression(Parser *p);
TypeRef infer_expr(Parser *p, Expr *e);
int parse_block(Parser *p, Statement **out, size_t *out_count);

#endif
