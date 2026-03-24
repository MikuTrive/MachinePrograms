/* Expression parsing and type inference for Machine
 *
 * This file owns:
 *   - name lookup helpers
 *   - reserved-name checks
 *   - type parsing
 *   - expression parsing with postfix chains
 *   - call/type inference, including module calls like Name.func()
 */

#include "parser_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const Token *cur(Parser *p) { return &p->tokens->items[p->index]; }
/* we implement a set of helper functions for the parser, including functions to get 
   the current and previous tokens, match and expect specific token types, 
   and skip layout tokens such as newlines and indentation.
   these functions provide a convenient interface for navigating through the list of 
   tokens during the parsing process, allowing us to easily check for expected syntax 
   and handle errors when the syntax does not match our expectations. */
const Token *prev(Parser *p) { return &p->tokens->items[p->index - 1]; }
bool match(Parser *p, TokenType t)
{
    if (cur(p)->type == t)
    {
        ++p->index;
        return true;
    }
    return false;
}
/* we implement a function to check if the current token matches a specific token type, 
   and if it does, we advance the parser's index to consume the token.
   this function is commonly used throughout the parsing process to 
   check for expected syntax and move through the token stream accordingly. */
bool expect(Parser *p, TokenType t, const char *msg)
{
    if (match(p, t))
        return true;
    diagnostics_add(p->src, p->errors, cur(p)->line, cur(p)->column, "%s, got '%s'", msg, token_type_name(cur(p)->type));
    return false;
}

static void skip_layout_tokens(Parser *p)
/**
 * Skips layout tokens such as newlines and indentation.
 * @param p The parser instance.
 */
{
    while (cur(p)->type == TOKEN_NEWLINE || cur(p)->type == TOKEN_INDENT || cur(p)->type == TOKEN_DEDENT)
    {
        ++p->index;
    }
}

TypeRef make_type(MachineType kind, const char *struct_name)
{
    TypeRef t;
    memset(&t, 0, sizeof(t));
    t.kind = kind;
    t.array_depth = (kind == TYPE_ARRAY) ? 1 : 0;
    if (struct_name)
    {
        copy_cstr(t.struct_name, sizeof(t.struct_name), struct_name);
    }
    return t;
}
/* we implement a function to create a TypeRef structure, 
   which represents a type in our programming language.
   this function takes a MachineType enum value and an optional struct name, 
   and it initializes the TypeRef structure accordingly.
   this is a convenient way to create type references throughout the parsing process, 
   allowing us to easily represent different types such as primitive types, structs, arrays, and more. */
int type_equals(TypeRef a, TypeRef b)
{
    return a.kind == b.kind &&
           a.array_depth == b.array_depth &&
           (a.kind != TYPE_STRUCT || strcmp(a.struct_name, b.struct_name) == 0);
}
const char *type_display_name(TypeRef t, char *buf, size_t n)
/**
 * Returns the display name for a type.
 * @param t The type reference.
 * @param buf The buffer to store the display name.
 * @param n The size of the buffer.
 * @return A pointer to the display name.
 */
{
    if (t.kind == TYPE_STRUCT)
    {
        if (buf && n > 0)
            copy_cstr(buf, n, t.struct_name);
        return buf;
    }
    if (t.kind == TYPE_ARRAY && buf && n > 0)
    /* we implement a function to get the display name for a type reference, 
       which is useful for error messages and debugging.
       this function takes a TypeRef structure and returns a human-readable string 
       representation of the type, including handling for struct types and 
       array types with their dimensions. */
    {
        if (t.array_depth <= 1)
            copy_cstr(buf, n, "array");
        else
            snprintf(buf, n, "array[%dD]", t.array_depth);
        return buf;
    }
    return machine_type_name(t.kind);
}

int is_reserved_name(const char *name)
/**
 * Checks if a given name is reserved in our programming language.
 * @param name The name to check.
 * @return 1 if the name is reserved, 0 otherwise.
 */
{
    static const char *reserved[] = {
        /* we implement a function to check if a given name is reserved in our programming language, 
           which helps prevent naming conflicts with keywords and built-in functions.
           this function checks the provided name against a list of 
           reserved keywords and built-in function names, and it returns true if the 
           name is reserved, or false otherwise. */
        "main", "func", "struct", "var", "const", "if", "else", "elif", "while", "print", "ret", "true", "false",
        "goto", "label", "switch", "case", "default", "module",
        "len", "hp", "sqrt", "sin", "cos", "pow",
        "hp_add", "hp_sub", "hp_mul", "hp_div", "hp_sqrt", "hp_pow",
        "alloc_bytes", "free_mem", "store_i64", "load_i64", "store_f64", "load_f64",
        "store_str", "load_str", "list_new", "list_push_back", "list_get", "list_size", "list_free",
        "array_new", "array_push", "array_get", "array_set", "array_len", "array_free",
        "grid_new", "grid_get", "grid_set", "grid_rows", "grid_cols", "grid_fill", "grid_free",
        "term_enable_raw", "term_disable_raw", "term_key_available", "term_read_key",
        "term_enable_mouse", "term_disable_mouse", "term_poll_event", "term_last_key",
        "term_mouse_x", "term_mouse_y", "term_mouse_button", "term_clear", "term_flush",
        "term_move_cursor", "term_hide_cursor", "term_show_cursor", "term_draw_text",
        "sleep_ms", "tick_ms", "timer_reset", "timer_elapsed_ms",
        "win_create", "win_destroy", "win_is_open", "win_poll_event", "win_last_key", "win_mouse_x",
        "win_mouse_y", "win_mouse_button", "win_clear", "win_present", "win_set_title", "win_draw_rect",
        "win_fill_rect", "win_draw_line", "win_draw_pixel", "image_load", "image_draw", "image_draw_scaled",
        "image_width", "image_height", "image_free", "video_play", "video_stop", "video_is_running",
        "addr", "index", "str", "i64", "f64", "hp", "ptr", "list", "array", "bool", "void", "module"};
    for (size_t i = 0; i < sizeof(reserved) / sizeof(reserved[0]); ++i)
        if (strcmp(name, reserved[i]) == 0)
            return 1;
    return 0;
}

Expr *new_expr(Parser *p, ExprKind kind, int line, int column)
/**
 * Creates a new expression node for the parser.
 * @param p The parser instance.
 * @param kind The kind of the expression.
 * @param line The line number.
 * @param column The column number.
 * @return A pointer to the new expression node, or NULL on failure.
 */
{
    if (p->program->expr_pool_count >= MACHINE_MAX_EXPR_POOL)
    {
        diagnostics_add(p->src, p->errors, line, column, "expression pool limit reached");
        return NULL;
    }
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    /* we implement a function to create a new expression node for the parser, 
       which is used to build the abstract syntax tree (AST) during the parsing process.
       this function takes the parser instance, the kind of expression, 
       and the line and column numbers for error reporting, and it allocates a new Expr structure, initializes it, 
       and adds it to the program's expression pool.
       if the expression pool limit is reached or if memory allocation fails, we report an error to the user. */
    if (!e)
    {
        diagnostics_add(p->src, p->errors, line, column, "out of memory while building expression");
        return NULL;
    }
    e->kind = kind;
    e->inferred_type = make_type(TYPE_UNKNOWN, NULL);
    e->line = line;
    e->column = column;
    p->program->expr_pool[p->program->expr_pool_count++] = e;
    return e;
}

Symbol *find_symbol(Parser *p, const char *name)
/**
 * Finds a symbol in the parser's symbol table.
 * @param p The parser instance.
 * @param name The name of the symbol to find.
 * @return A pointer to the symbol if found, or NULL otherwise.
 */
{
    for (size_t i = 0; i < p->symbol_count; ++i)
        if (strcmp(p->symbols[i].name, name) == 0)
            return &p->symbols[i];
    /* we implement a function to find a symbol in the parser's symbol table, 
       which is used to resolve variable and function names during the parsing process.
       this function takes the parser instance and the name of the symbol to find, 
       and it searches through the symbol table for a matching name, 
       returning a pointer to the symbol if found, or NULL if the symbol is not found. */
    return NULL;
}
FunctionDecl *find_function(Program *program, const char *name)
/**
 * Finds a function declaration in the program.
 * @param program The program instance.
 * @param name The name of the function to find.
 * @return A pointer to the function declaration if found, or NULL otherwise.
 */
{
    for (size_t i = 0; i < program->function_count; ++i)
        if (strcmp(program->functions[i].name, name) == 0)
            return &program->functions[i];
    /* we implement a function to find a function declaration in the program, 
       which is used to resolve function names during the parsing process.
       this function takes the program instance and the name of the function to find, 
       and it searches through the function list for a matching name, 
       returning a pointer to the function declaration if found, 
       or NULL if the function is not found. */
    return NULL;
}
StructDecl *find_struct(Program *program, const char *name)
{
    for (size_t i = 0; i < program->struct_count; ++i)
        if (strcmp(program->structs[i].name, name) == 0)
            return &program->structs[i];
    return NULL;
}
ModuleDecl *find_module(Program *program, const char *name)
{
    for (size_t i = 0; i < program->module_count; ++i)
        if (strcmp(program->modules[i].name, name) == 0)
            return &program->modules[i];
    return NULL;
}
GlobalVarDecl *find_global(Program *program, const char *name)
{
    for (size_t i = 0; i < program->global_count; ++i)
        if (strcmp(program->globals[i].name, name) == 0)
            return &program->globals[i];
    return NULL;
}
StructField *find_struct_field(StructDecl *sd, const char *field)
/**
 * Finds a field in a struct declaration.
 * @param sd The struct declaration.
 * @param field The name of the field to find.
 * @return A pointer to the field if found, or NULL otherwise.
 */
{
    for (size_t i = 0; i < sd->field_count; ++i)
        if (strcmp(sd->fields[i].name, field) == 0)
            return &sd->fields[i];
    return NULL;
    /* we implement a function to find a field in a struct declaration, 
       which is used to resolve field names when accessing struct members during the parsing process.
       this function takes a pointer to the struct declaration and the name of the field to find, 
       and it searches through the fields of the struct for a matching name, 
       returning a pointer to the field if found, or NULL if the field is not found. */
}

int add_local(Parser *p, const char *name, TypeRef type, int line)
/**
 * Adds a local variable to the parser's symbol table.
 * @param p The parser instance.
 * @param name The name of the local variable.
 * @param type The type of the local variable.
 * @param line The line number where the variable is declared.
 * @return 1 on success, 0 on failure.
 */
{
    if (is_reserved_name(name))
    {
        diagnostics_add(p->src, p->errors, line, 1, "'%s' is reserved and cannot be used as a variable name", name);
        /* we implement a function to add a local variable to the parser's symbol table, 
           which is used to keep track of variable declarations within functions during the parsing process.
           this function takes the parser instance, the name of the local variable, 
           its type, and the line number where it is declared, 
           and it checks if the name is reserved or if it has already been defined in the current scope.
           if the name is valid and there is room in the symbol table, 
           it adds a new symbol for the local variable with the provided information. */
        return 0;
    }
    if (find_symbol(p, name) || find_global(p->program, name))
    {
        diagnostics_add(p->src, p->errors, line, 1, "redefinition of symbol '%s'", name);
        return 0;
    }
    if (p->symbol_count >= MACHINE_MAX_SYMBOLS)
    {
        diagnostics_add(p->src, p->errors, line, 1, "too many symbols in current function");
        return 0;
    }
    Symbol *sym = &p->symbols[p->symbol_count++];
    memset(sym, 0, sizeof(*sym));
    if (!copy_cstr(sym->name, sizeof(sym->name), name))
    {
        diagnostics_add(p->src, p->errors, line, 1, "name '%s' is too long", name);
        return 0;
    }
    sym->type = type;
    sym->line = line;
    sym->is_const = false;
    return 1;
}

int parse_type_ref(Parser *p, TypeRef *out)
/**
 * Parses a type reference in the parser.
 * @param p The parser instance.
 * @param out A pointer to the TypeRef to populate.
 * @return 1 on success, 0 on failure.
 */
{
    if (cur(p)->type != TOKEN_IDENTIFIER)
    {
        diagnostics_add(p->src, p->errors, cur(p)->line, cur(p)->column, "expected type name");
        return 0;
    }
    const char *s = cur(p)->lexeme;
    if (strcmp(s, "i64") == 0)
        *out = make_type(TYPE_I64, NULL);
    else if (strcmp(s, "f64") == 0)
        *out = make_type(TYPE_F64, NULL);
    else if (strcmp(s, "hp") == 0)
        *out = make_type(TYPE_HP, NULL);
    else if (strcmp(s, "str") == 0)
        *out = make_type(TYPE_STR, NULL);
    else if (strcmp(s, "ptr") == 0)
        *out = make_type(TYPE_PTR, NULL);
    else if (strcmp(s, "list") == 0)
        *out = make_type(TYPE_LIST, NULL);
    else if (strcmp(s, "array") == 0)
    /**
     * Creates a type reference for an array type.
     * @param base The base type of the array.
     * @param depth The depth of the array.
     * @return A TypeRef representing the array type.
     */
    {
        *out = make_type(TYPE_ARRAY, NULL);
        (*out).array_depth = 1;
    }
    else if (strcmp(s, "bool") == 0)
        *out = make_type(TYPE_BOOL, NULL);
    else if (strcmp(s, "void") == 0)
        *out = make_type(TYPE_VOID, NULL);
    else if (find_struct(p->program, s))
        *out = make_type(TYPE_STRUCT, s);
    else
    {
        /* if the type name does not match any of the known primitive types or struct types, 
        we report an error to the user indicating that the type is unknown. */
        diagnostics_add(p->src, p->errors, cur(p)->line, cur(p)->column, "unknown type '%s'", s);
        return 0;
    }
    ++p->index;
    return 1;
}

static Expr *parse_atom(Parser *p)
{
    const Token *t = cur(p);
    if (match(p, TOKEN_NUMBER))
    {
        Expr *e = new_expr(p, strchr(t->lexeme, '.') ? EXPR_FLOAT : EXPR_INT, t->line, t->column);
        if (!e)
            return NULL;
        if (e->kind == EXPR_FLOAT)
        {
            e->as.float_value = strtod(t->lexeme, NULL);
            e->inferred_type = make_type(TYPE_F64, NULL);
        }
        else
        {
            e->as.int_value = strtoll(t->lexeme, NULL, 10);
            e->inferred_type = make_type(TYPE_I64, NULL);
            /* we implement a function to parse an atomic expression, 
               which is the most basic form of expression in our programming language.
               this function checks the type of the current token and creates an appropriate expression 
               node for literals such as numbers, strings, booleans, arrays, and identifiers.
               for number literals, it distinguishes between integers and floating-point numbers based 
               on the presence of a decimal point, and it sets the inferred type accordingly.
               For string literals, it copies the string value and sets the type to string. 
               For boolean literals, it sets the value and type to boolean. For array literals, it parses the 
               items in the array and sets the type to array.
               For identifiers, it creates an identifier expression node with the name of the identifier. 
               If the token does not match any of these expected types, it reports an error 
               indicating that an expression was expected. */
        }
        return e;
    }
    if (match(p, TOKEN_STRING))
    /**
     * Parses a string literal.
     * @param p The parser instance.
     * @param t The current token.
     * @return A pointer to the parsed string expression, or NULL on failure.
     */
    {
        Expr *e = new_expr(p, EXPR_STRING, t->line, t->column);
        if (!e)
            return NULL;
        if (!copy_cstr(e->as.text, sizeof(e->as.text), t->lexeme))
        {
            diagnostics_add(p->src, p->errors, t->line, t->column, "literal is too long");
            return NULL;
        }
        e->inferred_type = make_type(TYPE_STR, NULL);
        return e;
    }
    if (match(p, TOKEN_TRUE) || match(p, TOKEN_FALSE))
    {
        Expr *e = new_expr(p, EXPR_BOOL, t->line, t->column);
        if (!e)
            return NULL;
        e->as.bool_value = (t->type == TOKEN_TRUE);
        e->inferred_type = make_type(TYPE_BOOL, NULL);
        /* we implement a function to parse an atomic expression, 
           which is the most basic form of expression in our programming language.
           this function checks the type of the current token and creates an appropriate 
           expression node for literals such as numbers, strings, booleans, arrays, and identifiers.
           for number literals, it distinguishes between integers and floating-point 
           numbers based on the presence of a decimal point, and it sets the inferred type accordingly.
           For string literals, it copies the string value and sets the type to string. 
           For boolean literals, it sets the value and type to boolean.
           For array literals, it parses the items in the array and sets the type to array. 
           For identifiers, it creates an identifier expression node with the name of the identifier. 
           If the token does not match any of these expected types,
           it reports an error indicating that an expression was expected. */
        return e;
    }
    if (match(p, TOKEN_LBRACKET))
    {
        Expr *e = new_expr(p, EXPR_ARRAY, t->line, t->column);
        if (!e)
            return NULL;
        skip_layout_tokens(p);
        if (!match(p, TOKEN_RBRACKET))
        /**
         * Parses an array literal.
         * @param p The parser instance.
         * @param t The current token.
         * @return A pointer to the parsed array expression, or NULL on failure.
         */
        {
            for (;;)
            {
                if (e->as.array.item_count >= sizeof(e->as.array.items) / sizeof(e->as.array.items[0]))
                {
                    diagnostics_add(p->src, p->errors, cur(p)->line, cur(p)->column, "array literal has too many items");
                    return NULL;
                }
                e->as.array.items[e->as.array.item_count++] = parse_expression(p);
                skip_layout_tokens(p);
                /* we implement a function to parse an array literal, 
                   which is a comma-separated list of expressions enclosed in square brackets.
                   this function creates a new array expression node and then 
                   enters a loop to parse each item in the array until it reaches the closing square bracket.
                   it checks for the maximum number of items allowed in the array and reports an error if that limit is exceeded.
                   after parsing each item, it skips any layout tokens and checks for a comma to separate items, 
                   or the closing square bracket to end the array literal. */
                if (match(p, TOKEN_RBRACKET))
                {
                    break;
                }
                if (!expect(p, TOKEN_COMMA, "expected ',' between array items"))
                {
                    return NULL;
                }
                skip_layout_tokens(p);
                if (match(p, TOKEN_RBRACKET))
                {
                    break;
                }
            }
        }
        return e;
    }
    if (match(p, TOKEN_IDENTIFIER))
    {
        /* we implement a function to parse an atomic expression, 
           which is the most basic form of expression in our programming language.
           this function checks the type of the current token and creates an appropriate expression 
           node for literals such as numbers, strings, booleans, arrays, and identifiers.
           for number literals, it distinguishes between integers and floating-point 
           numbers based on the presence of a decimal point, and it sets the inferred type accordingly.
           For string literals, it copies the string value and sets the type to string. 
           For boolean literals, it sets the value and type to boolean.
           For array literals, it parses the items in the array and sets the type to array. 
           For identifiers, it creates an identifier expression node with the name of the identifier.
           If the token does not match any of these expected types, 
           it reports an error indicating that an expression was expected. */
        Expr *e = new_expr(p, EXPR_IDENTIFIER, t->line, t->column);
        if (!e)
            return NULL;
        if (!copy_cstr(e->as.text, sizeof(e->as.text), t->lexeme))
        {
            diagnostics_add(p->src, p->errors, t->line, t->column, "identifier is too long");
            return NULL;
        }
        return e;
    }
    if (match(p, TOKEN_LPAREN))
    {
        Expr *e = parse_expression(p);
        expect(p, TOKEN_RPAREN, "expected ')' after expression");
        return e;
    }
    diagnostics_add(p->src, p->errors, t->line, t->column, "expected expression");
    return NULL;
}

static Expr *parse_primary(Parser *p)
{
    /* we implement a function to parse a primary expression, which is the base 
       case for parsing more complex expressions in our programming language.
       this function starts by parsing an atomic expression using the parse_atom function, 
       and then it enters a loop to handle postfix operations such as function calls, array indexing, 
       field access, and pointer dereferencing.
       for each of these operations, it creates a new expression node that represents the 
       operation and updates the current expression to be the result of that operation, 
       allowing for chaining of multiple postfix operations in a single expression. */
    Expr *e = parse_atom(p);
    if (!e)
        return NULL;
    for (;;)
    {
        if (match(p, TOKEN_LPAREN))
        {
            Expr *call = new_expr(p, EXPR_CALL, e->line, e->column);
            if (!call)
                return NULL;
            call->as.call.callee = e;
            if (!match(p, TOKEN_RPAREN))
            {
                do
                {
                    call->as.call.args[call->as.call.arg_count++] = parse_expression(p);
                } while (match(p, TOKEN_COMMA));
                if (!expect(p, TOKEN_RPAREN, "expected ')' after function arguments"))
                    return NULL;
            }
            e = call;
            continue;
        }
        if (match(p, TOKEN_LBRACKET))
        /**
         * Parses an array indexing expression.
         * @param p The parser instance.
         * @param e The base expression being indexed.
         * @return A pointer to the parsed index expression, or NULL on failure.
         */
        {
            Expr *idx = parse_expression(p);
            if (!expect(p, TOKEN_RBRACKET, "expected ']' after index"))
                return NULL;
            Expr *outer = new_expr(p, EXPR_INDEX, e->line, e->column);
            outer->as.index.base = e;
            outer->as.index.index = idx;
            e = outer;
            continue;
        }
        if (match(p, TOKEN_DOT))
        {
            const Token *fname = cur(p);
            if (!expect(p, TOKEN_IDENTIFIER, "expected field name after '.'"))
                return NULL;
            Expr *outer = new_expr(p, EXPR_FIELD, fname->line, fname->column);
            outer->as.field.base = e;
            if (!copy_cstr(outer->as.field.field, sizeof(outer->as.field.field), fname->lexeme))
            {
                diagnostics_add(p->src, p->errors, fname->line, fname->column, "field name is too long");
                return NULL;
            }
            e = outer;
            continue;
        }
        if (match(p, TOKEN_CARET))
        {
            Expr *outer = new_expr(p, EXPR_UNARY, e->line, e->column);
            strcpy(outer->as.unary.op, "^");
            outer->as.unary.operand = e;
            e = outer;
            continue;
        }
        break;
    }
    return e;
}

/* we implement a function to parse a unary expression, 
   which is an expression that consists of a single operand with a unary operator applied to it.
   this function checks for the presence of unary operators such as negation or pointer dereferencing, 
   and if it finds one, it creates a new expression node that represents the unary 
   operation and recursively calls itself to parse the operand of the unary operator.
   if no unary operator is found, it falls back to parsing a primary expression, 
   which is the base case for more complex expressions. */
static Expr *parse_unary(Parser *p)
{
    const Token *t = cur(p);
    if (match(p, TOKEN_MINUS) || match(p, TOKEN_AT))
    {
        Expr *e = new_expr(p, EXPR_UNARY, t->line, t->column);
        strcpy(e->as.unary.op, prev(p)->lexeme);
        e->as.unary.operand = parse_unary(p);
        return e;
    }
    return parse_primary(p);
}
static Expr *parse_mul(Parser *p)
{
    Expr *left = parse_unary(p);
    while (cur(p)->type == TOKEN_STAR || cur(p)->type == TOKEN_SLASH)
    {
        const Token *op = cur(p);
        ++p->index;
        Expr *right = parse_unary(p);
        Expr *e = new_expr(p, EXPR_BINARY, op->line, op->column);
        strcpy(e->as.binary.op, op->lexeme);
        e->as.binary.left = left;
        e->as.binary.right = right;
        left = e;
    }
    return left;
}
static Expr *parse_add(Parser *p)
{
    Expr *left = parse_mul(p);
    while (cur(p)->type == TOKEN_PLUS || cur(p)->type == TOKEN_MINUS)
    {
        const Token *op = cur(p);
        ++p->index;
        Expr *right = parse_mul(p);
        Expr *e = new_expr(p, EXPR_BINARY, op->line, op->column);
        strcpy(e->as.binary.op, op->lexeme);
        e->as.binary.left = left;
        e->as.binary.right = right;
        left = e;
    }
    return left;
    /* we implement functions to parse binary expressions with different levels of operator precedence, 
       such as multiplication and division for the highest precedence, 
       followed by addition and subtraction, and then comparison operators.
       each of these functions follows a similar pattern: it starts by parsing the 
       left-hand side of the expression using the next level of precedence, and then it 
       enters a loop to check for the presence of the relevant operators.
       if it finds an operator, it creates a new expression node for the binary operation, 
       parses the right-hand side of the expression, 
       and updates the left-hand side to be the result of the binary operation, 
       allowing for chaining of multiple binary operations with the same precedence level. */
}
static Expr *parse_compare(Parser *p)
{
    Expr *left = parse_add(p);
    while (cur(p)->type == TOKEN_EQEQ || cur(p)->type == TOKEN_NEQ || cur(p)->type == TOKEN_LT || cur(p)->type == TOKEN_GT || cur(p)->type == TOKEN_LE || cur(p)->type == TOKEN_GE)
    {
        /* we implement a function to parse comparison expressions, 
           which are binary expressions that compare two values using operators such as equality, 
           inequality, less than, greater than, and so on.
           this function starts by parsing the left-hand side of the expression using the 
           next level of precedence (addition and subtraction), and then it enters a loop to check 
           for the presence of comparison operators. */
        const Token *op = cur(p);
        ++p->index;
        Expr *right = parse_add(p);
        Expr *e = new_expr(p, EXPR_BINARY, op->line, op->column);
        strcpy(e->as.binary.op, op->lexeme);
        e->as.binary.left = left;
        e->as.binary.right = right;
        left = e;
    }
    return left;
}
Expr *parse_expression(Parser *p) { return parse_compare(p); }

static TypeRef infer_call(Parser *p, Expr *e)
/**
 * Infers the type of a function call expression.
 * @param p The parser instance.
 * @param e The function call expression.
 * @return The inferred type of the function call.
 */
{
    char fullname[128];
    for (size_t i = 0; i < e->as.call.arg_count; ++i)
        infer_expr(p, e->as.call.args[i]);

    if (e->as.call.callee->kind == EXPR_IDENTIFIER)
    /**
     * Infers the type of a function call expression.
     * @param p The parser instance.
     * @param e The function call expression.
     * @return The inferred type of the function call.
     */
    {
        const char *name = e->as.call.callee->as.text;
        if (find_struct(p->program, name))
        {
            if (e->as.call.arg_count != 0)
                diagnostics_add(p->src, p->errors, e->line, e->column, "struct constructor '%s' currently takes no arguments", name);
            return e->inferred_type = make_type(TYPE_STRUCT, name);
        }
        if (strcmp(name, "len") == 0)
            return e->inferred_type = make_type(TYPE_I64, NULL);
        if (strcmp(name, "hp") == 0)
            return e->inferred_type = make_type(TYPE_HP, NULL);
        if (strcmp(name, "sqrt") == 0 || strcmp(name, "sin") == 0 || strcmp(name, "cos") == 0 || strcmp(name, "pow") == 0)
            return e->inferred_type = make_type(TYPE_F64, NULL);
        if (strncmp(name, "hp_", 3) == 0)
            return e->inferred_type = make_type(TYPE_HP, NULL);
        if (strcmp(name, "alloc_bytes") == 0 || strcmp(name, "addr") == 0)
            return e->inferred_type = make_type(TYPE_PTR, NULL);
        if (strcmp(name, "free_mem") == 0 || strcmp(name, "store_i64") == 0 || strcmp(name, "store_f64") == 0 || strcmp(name, "store_str") == 0 || strcmp(name, "list_push_back") == 0 || strcmp(name, "list_free") == 0 || strcmp(name, "array_push") == 0 || strcmp(name, "array_set") == 0 || strcmp(name, "array_free") == 0)
            return e->inferred_type = make_type(TYPE_VOID, NULL);
        if (strcmp(name, "load_i64") == 0)
            return e->inferred_type = make_type(TYPE_I64, NULL);
        if (strcmp(name, "load_f64") == 0)
            return e->inferred_type = make_type(TYPE_F64, NULL);
        if (strcmp(name, "load_str") == 0)
            return e->inferred_type = make_type(TYPE_STR, NULL);
        if (strcmp(name, "list_new") == 0)
            return e->inferred_type = make_type(TYPE_LIST, NULL);
        if (strcmp(name, "list_get") == 0 || strcmp(name, "list_size") == 0)
            return e->inferred_type = make_type(TYPE_I64, NULL);
        if (strcmp(name, "array_new") == 0)
            return e->inferred_type = make_type(TYPE_ARRAY, NULL);
        if (strcmp(name, "array_get") == 0 || strcmp(name, "array_len") == 0)
            return e->inferred_type = make_type(TYPE_I64, NULL);
        if (strcmp(name, "grid_new") == 0)
            return e->inferred_type = make_type(TYPE_PTR, NULL);
        if (strcmp(name, "grid_get") == 0 || strcmp(name, "grid_rows") == 0 || strcmp(name, "grid_cols") == 0)
            return e->inferred_type = make_type(TYPE_I64, NULL);
        if (strcmp(name, "term_key_available") == 0)
            return e->inferred_type = make_type(TYPE_BOOL, NULL);
        if (strcmp(name, "term_read_key") == 0 || strcmp(name, "term_poll_event") == 0 || strcmp(name, "term_last_key") == 0 || strcmp(name, "term_mouse_x") == 0 || strcmp(name, "term_mouse_y") == 0 || strcmp(name, "term_mouse_button") == 0 || strcmp(name, "tick_ms") == 0 || strcmp(name, "timer_elapsed_ms") == 0 || strcmp(name, "win_poll_event") == 0 || strcmp(name, "win_last_key") == 0 || strcmp(name, "win_mouse_x") == 0 || strcmp(name, "win_mouse_y") == 0 || strcmp(name, "win_mouse_button") == 0 || strcmp(name, "image_width") == 0 || strcmp(name, "image_height") == 0 || strcmp(name, "video_play") == 0)
            return e->inferred_type = make_type(TYPE_I64, NULL);
        if (strcmp(name, "term_key_available") == 0 || strcmp(name, "win_create") == 0 || strcmp(name, "win_is_open") == 0 || strcmp(name, "video_is_running") == 0)
            return e->inferred_type = make_type(TYPE_BOOL, NULL);
        if (strcmp(name, "image_load") == 0)
            return e->inferred_type = make_type(TYPE_PTR, NULL);
        if (strcmp(name, "term_enable_raw") == 0 || strcmp(name, "term_disable_raw") == 0 || strcmp(name, "term_enable_mouse") == 0 || strcmp(name, "term_disable_mouse") == 0 || strcmp(name, "term_clear") == 0 || strcmp(name, "term_flush") == 0 || strcmp(name, "term_move_cursor") == 0 || strcmp(name, "term_hide_cursor") == 0 || strcmp(name, "term_show_cursor") == 0 || strcmp(name, "term_draw_text") == 0 || strcmp(name, "sleep_ms") == 0 || strcmp(name, "timer_reset") == 0 || strcmp(name, "grid_set") == 0 || strcmp(name, "grid_fill") == 0 || strcmp(name, "grid_free") == 0 || strcmp(name, "win_destroy") == 0 || strcmp(name, "win_clear") == 0 || strcmp(name, "win_present") == 0 || strcmp(name, "win_set_title") == 0 || strcmp(name, "win_draw_rect") == 0 || strcmp(name, "win_fill_rect") == 0 || strcmp(name, "win_draw_line") == 0 || strcmp(name, "win_draw_pixel") == 0 || strcmp(name, "win_draw_text") == 0 || strcmp(name, "image_draw") == 0 || strcmp(name, "image_draw_scaled") == 0 || strcmp(name, "image_free") == 0 || strcmp(name, "video_stop") == 0)
            return e->inferred_type = make_type(TYPE_VOID, NULL);
        if (strcmp(name, "index") == 0 || strcmp(name, "str") == 0)
            return e->inferred_type = make_type(TYPE_STR, NULL);
        FunctionDecl *f = find_function(p->program, name);

        /* if the function name matches one of the built-in functions or
           struct constructors, we return the appropriate type for that function call.
           if the function name does not match any known built-in functions,
           we look it up in the program's function list to see if it is a user-defined function,
           and if found, we mark it as used and return its return type as the inferred type of the function call.
           if the function is not found in either the built-in list or the user-defined list,
           we report an error indicating that the function is unknown and return an invalid type. */
        if (f)
        {
            f->used = true;
            return e->inferred_type = f->return_type;
        }
        diagnostics_add(p->src, p->errors, e->line, e->column, "unknown function '%s'", name);
        /* if the function name does not match any known built-in functions or
           user-defined functions, we report an error indicating that the
           function is unknown and return an invalid type. */
        return e->inferred_type = make_type(TYPE_INVALID, NULL);
    }

    if (e->as.call.callee->kind == EXPR_FIELD && e->as.call.callee->as.field.base->kind == EXPR_IDENTIFIER)
    /**
     * Infers the type of a module-qualified function call expression.
     * @param p The parser instance.
     * @param e The module-qualified function call expression.
     * @return The inferred type of the function call.
     */
    {
        const char *module_name = e->as.call.callee->as.field.base->as.text;
        const char *func_name = e->as.call.callee->as.field.field;
        if (!find_module(p->program, module_name))
        {
            diagnostics_add(p->src, p->errors, e->line, e->column, "unknown module '%s'", module_name);
            return e->inferred_type = make_type(TYPE_INVALID, NULL);
        }
        if (!join_cstr3(fullname, sizeof(fullname), module_name, "__", func_name))
        {
            diagnostics_add(p->src, p->errors, e->line, e->column, "module-qualified function name is too long");
            return make_type(TYPE_UNKNOWN, NULL);
        }
        FunctionDecl *f = find_function(p->program, fullname);
        if (!f)
        {
            diagnostics_add(p->src, p->errors, e->line, e->column, "module '%s' has no function '%s'", module_name, func_name);
            return e->inferred_type = make_type(TYPE_INVALID, NULL);
        }
        f->used = true;
        /* if the function call is of the form module.function,
           we check if the module exists and then look for the function
           within that module by constructing a qualified name.
           if the module or function is not found, we report an error. If found,
           we mark the function as used and return its return type as the inferred type of the function call. */
        return e->inferred_type = f->return_type;
    }

    diagnostics_add(p->src, p->errors, e->line, e->column, "call target is not callable in this compiler stage");
    return e->inferred_type = make_type(TYPE_INVALID, NULL);
}

TypeRef infer_expr(Parser *p, Expr *e)
{
    char buf1[128], buf2[128];
    if (!e)
        return make_type(TYPE_INVALID, NULL);
    if (e->inferred_type.kind != TYPE_UNKNOWN)
        return e->inferred_type;
    switch (e->kind)
    {
    case EXPR_INT:
        return e->inferred_type = make_type(TYPE_I64, NULL);
    case EXPR_FLOAT:
        return e->inferred_type = make_type(TYPE_F64, NULL);
    case EXPR_STRING:
        return e->inferred_type = make_type(TYPE_STR, NULL);
    case EXPR_BOOL:
        return e->inferred_type = make_type(TYPE_BOOL, NULL);
    case EXPR_IDENTIFIER:
    {
        Symbol *sym = find_symbol(p, e->as.text);
        if (sym)
        {
            sym->used = true;
            copy_cstr(e->struct_name, sizeof(e->struct_name), sym->type.struct_name);
            return e->inferred_type = sym->type;
            /* if the expression is an identifier, we look it up in the current symbol table to
               see if it is a local variable, and if found, we mark it as used and return
               its type as the inferred type of the expression.
               if the identifier is not found in the local symbol table,
               we then look it up in the global variable list to see if it is a global variable,
               and if found, we mark it as used and return its declared type as the inferred type of the expression.
               if the identifier is not found in either the local symbol table or the global variable list,
               we report an error indicating that the symbol is undeclared and return an invalid type. */
        }
        GlobalVarDecl *g = find_global(p->program, e->as.text);
        if (g)
        {
            g->used = true;
            copy_cstr(e->struct_name, sizeof(e->struct_name), g->declared_type.struct_name);
            return e->inferred_type = g->declared_type;
        }
        diagnostics_add(p->src, p->errors, e->line, e->column, "use of undeclared symbol '%s'", e->as.text);
        return e->inferred_type = make_type(TYPE_INVALID, NULL);
    }
    case EXPR_UNARY:
    {
        TypeRef inner = infer_expr(p, e->as.unary.operand);
        if (strcmp(e->as.unary.op, "-") == 0)
        {
            if (inner.kind == TYPE_I64 || inner.kind == TYPE_F64 || inner.kind == TYPE_HP)
                return e->inferred_type = inner;
            diagnostics_add(p->src, p->errors, e->line, e->column, "unary '-' expects numeric type, got %s", type_display_name(inner, buf1, sizeof(buf1)));
            return e->inferred_type = make_type(TYPE_INVALID, NULL);
        }
        if (strcmp(e->as.unary.op, "@") == 0)
            return e->inferred_type = make_type(TYPE_PTR, NULL);
        if (strcmp(e->as.unary.op, "^") == 0)
            return e->inferred_type = make_type(TYPE_I64, NULL);
        return e->inferred_type = make_type(TYPE_INVALID, NULL);
        /* if the expression is a unary operation, we first infer the type of
           the operand, and then we check the operator to determine the resulting type of the expression.
           for example, if the operator is negation ('-'), we check if
           the operand is a numeric type (i64, f64, or hp) and return that type as the inferred type of
           the expression; if the operator is pointer dereference ('@'),
           we return a pointer type; if the operator is some other unary operator (e.g., '^'),
           we return an appropriate type based on what that operator is defined to do.
           if the operator does not match any known unary operators or if the
           operand type is not compatible with the operator, we report an error and return an invalid type. */
    }
    case EXPR_INDEX:
    {
        TypeRef base = infer_expr(p, e->as.index.base);
        TypeRef idx = infer_expr(p, e->as.index.index);
        if (idx.kind != TYPE_I64)
        {
            diagnostics_add(p->src, p->errors, e->line, e->column, "index must be i64, got %s", type_display_name(idx, buf1, sizeof(buf1)));
            /* if the expression is an index operation (e.g., array indexing),
               we first infer the types of the base expression and the index expression.
               we check if the index expression is of type i64, since indexing typically
               requires an integer index. If the index type is not i64, we report an error and return an invalid type.
               if the index type is valid, we then check the type of the base expression to
               determine what kind of indexing is being performed. If the base type is a string,
               we return string as the inferred type; if the base type is a list,
               we return i64 as the inferred type (assuming lists are indexed to
               produce i64 values); if the base type is an array,
               we check the array depth to determine the resulting type of the
               indexing operation (e.g., if it's a one-dimensional array, we return i64; if it's a multi-dimensional array,
               we return an array type with one less dimension).
               If the base type is not one of the supported indexable types,
               we report an error and return an invalid type. */
            return e->inferred_type = make_type(TYPE_INVALID, NULL);
        }
        if (base.kind == TYPE_STR)
            return e->inferred_type = make_type(TYPE_STR, NULL);
        if (base.kind == TYPE_LIST)
            return e->inferred_type = make_type(TYPE_I64, NULL);
        if (base.kind == TYPE_ARRAY)
        {
            if (base.array_depth > 1)
            {
                TypeRef t = make_type(TYPE_ARRAY, NULL);
                t.array_depth = base.array_depth - 1;
                return e->inferred_type = t;
            }
            return e->inferred_type = make_type(TYPE_I64, NULL);
        }
        diagnostics_add(p->src, p->errors, e->line, e->column, "indexing is supported only for str, list and array, got %s", type_display_name(base, buf1, sizeof(buf1)));
        return e->inferred_type = make_type(TYPE_INVALID, NULL);
    }
    case EXPR_FIELD:

        /**
         * Infer the type of a field access expression.
         * @param p The parser.
         * @param e The field access expression.
         * @return The inferred type of the field access expression.
         */
        {
            TypeRef base = infer_expr(p, e->as.field.base);
            if (base.kind != TYPE_STRUCT)
            {
                diagnostics_add(p->src, p->errors, e->line, e->column, "field access requires a struct value, got %s", type_display_name(base, buf1, sizeof(buf1)));
                return e->inferred_type = make_type(TYPE_INVALID, NULL);
            }
            /* if the expression is a field access (e.g., struct field access),
               we first infer the type of the base expression, and then we check if the base type is a struct.
               if the base type is not a struct, we report an error and return an invalid type,
               since field access is only valid on struct types. If the base type is a struct,
               we then look up the struct definition to find the field being accessed, and if the field is found,
               we return the type of that field as the inferred type of the field access expression.
               If the field is not found in the struct definition, we report an error indicating that the
               struct does not have the specified field and return an invalid type. */
            StructDecl *sd = find_struct(p->program, base.struct_name);
            StructField *fd = sd ? find_struct_field(sd, e->as.field.field) : NULL;
            if (!fd)
            {
                diagnostics_add(p->src, p->errors, e->line, e->column, "struct '%s' has no field '%s'", base.struct_name, e->as.field.field);
                return e->inferred_type = make_type(TYPE_INVALID, NULL);
            }
            copy_cstr(e->struct_name, sizeof(e->struct_name), fd->type.struct_name);
            return e->inferred_type = fd->type;
        }
    case EXPR_CALL:
        return infer_call(p, e);

    case EXPR_ARRAY:
    {
        int depth = -1;
        for (size_t i = 0; i < e->as.array.item_count; ++i)
        {
            TypeRef it = infer_expr(p, e->as.array.items[i]);
            if (it.kind == TYPE_I64)
            {
                if (depth == -1)
                    depth = 1;
                else if (depth != 1)
                {
                    diagnostics_add(p->src, p->errors, e->line, e->column, "array literal mixes scalar and nested array items");
                    return e->inferred_type = make_type(TYPE_INVALID, NULL);
                }
            }
            else if (it.kind == TYPE_ARRAY)
            {
                int item_depth = it.array_depth + 1;
                if (depth == -1)
                    depth = item_depth;
                else if (depth != item_depth)
                {
                    diagnostics_add(p->src, p->errors, e->line, e->column, "nested array literal depths must match");
                    return e->inferred_type = make_type(TYPE_INVALID, NULL);
                }
            }
            else
            {
                diagnostics_add(p->src, p->errors, e->line, e->column, "array literals currently support i64 items or nested i64 arrays only");
                return e->inferred_type = make_type(TYPE_INVALID, NULL);
            }
        }
        if (depth == -1)
            depth = 1;
        TypeRef t = make_type(TYPE_ARRAY, NULL);
        t.array_depth = depth;
        return e->inferred_type = t;
    }
    case EXPR_BINARY:
    {
        TypeRef a = infer_expr(p, e->as.binary.left), b = infer_expr(p, e->as.binary.right);
        const char *op = e->as.binary.op;
        if (strcmp(op, "+") == 0)
        {
            if (a.kind == TYPE_STR && b.kind == TYPE_STR)
                return e->inferred_type = make_type(TYPE_STR, NULL);
            if (a.kind == TYPE_HP || b.kind == TYPE_HP)
                return e->inferred_type = make_type(TYPE_HP, NULL);
            if (a.kind == TYPE_F64 || b.kind == TYPE_F64)
                return e->inferred_type = make_type(TYPE_F64, NULL);
            if (a.kind == TYPE_I64 && b.kind == TYPE_I64)
                return e->inferred_type = make_type(TYPE_I64, NULL);
        }
        if (strcmp(op, "-") == 0 || strcmp(op, "*") == 0 || strcmp(op, "/") == 0)
        {
            if (a.kind == TYPE_HP || b.kind == TYPE_HP)
                return e->inferred_type = make_type(TYPE_HP, NULL);
            if (a.kind == TYPE_F64 || b.kind == TYPE_F64)
                return e->inferred_type = make_type(TYPE_F64, NULL);
            if (a.kind == TYPE_I64 && b.kind == TYPE_I64)
                return e->inferred_type = make_type(TYPE_I64, NULL);
        }
        if (!strcmp(op, "==") || !strcmp(op, "!=") || !strcmp(op, "<") || !strcmp(op, ">") || !strcmp(op, "<=") || !strcmp(op, ">="))
        /**
         * Infer the type of a binary expression.
         * @param p The parser.
         * @param e The binary expression.
         * @return The inferred type of the binary expression.
         */
        {
            if (a.kind == TYPE_STR && b.kind == TYPE_STR)
                return e->inferred_type = make_type(TYPE_BOOL, NULL);
            if ((a.kind == TYPE_I64 || a.kind == TYPE_F64 || a.kind == TYPE_HP || a.kind == TYPE_BOOL) && (b.kind == TYPE_I64 || b.kind == TYPE_F64 || b.kind == TYPE_HP || b.kind == TYPE_BOOL))
                return e->inferred_type = make_type(TYPE_BOOL, NULL);
            if ((!strcmp(op, "==") || !strcmp(op, "!=")) && ((a.kind == TYPE_PTR && b.kind == TYPE_PTR) || (a.kind == TYPE_PTR && b.kind == TYPE_I64) || (a.kind == TYPE_I64 && b.kind == TYPE_PTR)))
            {
                return e->inferred_type = make_type(TYPE_BOOL, NULL);
            }
        }
        diagnostics_add(p->src, p->errors, e->line, e->column, "operator '%s' is invalid for %s and %s", op, type_display_name(a, buf1, sizeof(buf1)), type_display_name(b, buf2, sizeof(buf2)));
        return e->inferred_type = make_type(TYPE_INVALID, NULL);
    }
    }
    return make_type(TYPE_INVALID, NULL);
}
