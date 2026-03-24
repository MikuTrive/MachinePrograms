/* Statement parsing and top-level program assembly for Machine
 *
 * This file owns:
 *   - variable / assignment / control-flow statements
 *   - block parsing
 *   - struct declarations
 *   - module declarations
 *   - function and main parsing
 */

#include "parser_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Statement *alloc_block(Parser *p, size_t count)
/**
 * Allocates a block of statements for the parser.
 * @param p The parser instance.
 * @param count The number of statements to allocate.
 * @return A pointer to the allocated statement block, or NULL on failure.
 */
{
    /* we implement a function to allocate a block of statements for the parser.
       this function takes care of memory management for statement blocks, 
       ensuring that they are properly allocated and tracked within the program structure.
       by centralizing block allocation in this function, 
       we can easily handle memory errors and maintain a consistent 
       approach to managing statement blocks throughout the parsing process. */
    Statement *block = (Statement *)calloc(count ? count : 1, sizeof(Statement));
    if (!block)
    {
        diagnostics_add(p->src, p->errors, cur(p)->line, cur(p)->column, "out of memory while building statement block");
        return NULL;
    }
    p->program->stmt_blocks[p->program->stmt_block_count++] = block;
    return block;
}

static SwitchCase *alloc_switch_cases(Parser *p, size_t count)
{
    SwitchCase *cases = (SwitchCase *)calloc(count ? count : 1, sizeof(SwitchCase));
    if (!cases)
    {
        diagnostics_add(p->src, p->errors, cur(p)->line, cur(p)->column, "out of memory while building switch cases");
        return NULL;
        /* we implement a function to allocate a block of switch cases for the parser.
           this function is responsible for managing memory allocation for switch cases, 
           ensuring that they are properly tracked within the program structure.
           by centralizing switch case allocation in this function, 
           we can handle memory errors effectively and maintain a consistent 
           approach to managing switch cases throughout the parsing process. */
    }
    p->program->switch_case_blocks[p->program->switch_case_block_count++] = cases;
    return cases;
}

static int type_is_assignable(TypeRef dst, TypeRef src)
/**
 * Checks if one type can be assigned to another in our programming language.
 * @param dst The destination type.
 * @param src The source type.
 * @return 1 if the assignment is valid, 0 otherwise.
 */
{
    /* we implement a function to check if one type can be assigned to 
       another in our programming language.
       this function takes into account the rules of type compatibility, 
       including implicit conversions and type hierarchies, to determine if an assignment is valid.
       by centralizing this logic in a single function, we can ensure consistent 
       type checking throughout the parser and provide clear error messages when type mismatches occur. */
    if (type_equals(dst, src))
        return 1;
    if (dst.kind == TYPE_HP && (src.kind == TYPE_F64 || src.kind == TYPE_I64))
        return 1;
    if (dst.kind == TYPE_F64 && src.kind == TYPE_I64)
        return 1;
    if ((dst.kind == TYPE_PTR || dst.kind == TYPE_ARRAY || dst.kind == TYPE_LIST) && src.kind == TYPE_I64)
        return 1;
    return 0;
}

static int parse_var_like(Parser *p, Statement *stmt, int is_const)
/**
 * Parses a variable-like declaration in the parser.
 * @param p The parser instance.
 * @param stmt The statement to populate.
 * @param is_const Whether the declaration is for a constant.
 * @return 1 on success, 0 on failure.
 */
{
    /* we implement a function to parse variable-like declarations in 
       our programming language, which can include both regular variables and constants.
       this function handles the syntax and semantics of variable declarations, 
       including optional type annotations and initializers, 
       while also performing necessary type checks and error reporting.
       by using a single function for both variables and constants, 
       we can reduce code duplication and maintain a consistent 
       approach to parsing these related constructs. */
    char buf1[128], buf2[128];
    const Token *name = cur(p);
    if (!expect(p, TOKEN_IDENTIFIER, is_const ? "expected constant name after const" : "expected variable name after var"))
        return 0;
    TypeRef declared = make_type(TYPE_UNKNOWN, NULL);
    int has_initializer = 0;
    Expr *init = NULL;
    if (match(p, TOKEN_COLON) && !parse_type_ref(p, &declared))
        return 0;
    if (match(p, TOKEN_EQUAL))
    {
        has_initializer = 1;
        init = parse_expression(p);
    }
    if (!expect(p, TOKEN_NEWLINE, is_const ? "expected end of line after constant declaration" : "expected end of line after variable declaration"))
        return 0;
    TypeRef init_type = has_initializer ? infer_expr(p, init) : declared;
    if (declared.kind == TYPE_UNKNOWN)
        declared = init_type;
    if (declared.kind == TYPE_UNKNOWN)
    {
        diagnostics_add(p->src, p->errors, name->line, name->column, "%s '%s' needs a type or initializer", is_const ? "constant" : "variable", name->lexeme);
        return 0;
    }
    /**
     * Checks if one type can be assigned to another in our programming language.
     * @param dst The destination type.
     * @param src The source type.
     * @return 1 if the assignment is valid, 0 otherwise.
     */
    if (is_const && !has_initializer)
    {
        diagnostics_add(p->src, p->errors, name->line, name->column, "const '%s' requires an initializer", name->lexeme);
        return 0;
    }
    if (has_initializer && !type_is_assignable(declared, init_type))
    {
        diagnostics_add(p->src, p->errors, name->line, name->column, "cannot initialize %s with %s", type_display_name(declared, buf1, sizeof(buf1)), type_display_name(init_type, buf2, sizeof(buf2)));
        return 0;
    }
    /**
     * Allocates a block of statements for the parser.
     * @param p The parser instance.
     * @param count The number of statements to allocate.
     * @return A pointer to the allocated statement block, or NULL on failure.
     */
    if (!add_local(p, name->lexeme, declared, name->line))
        return 0;
        /* we implement a function to parse variable-like declarations in our programming language, 
           which can include both regular variables and constants.
           this function handles the syntax and semantics of variable declarations, 
           including optional type annotations and initializers, 
           while also performing necessary type checks and error reporting.
           by using a single function for both variables and constants, 
           we can reduce code duplication and maintain a consistent approach to parsing these related constructs. */
    Symbol *sym = find_symbol(p, name->lexeme);
    if (sym)
        sym->is_const = is_const;
    stmt->kind = is_const ? STMT_CONST : STMT_VAR;
    if (!copy_cstr(stmt->as.var_stmt.name, sizeof(stmt->as.var_stmt.name), name->lexeme))
    {
        /**
         * Adds a diagnostic message to the parser's error list.
         * @param src The source file.
         * @param errors The error list.
         * @param line The line number.
         * @param column The column number.
         * @param fmt The format string for the error message.
         */
        diagnostics_add(p->src, p->errors, name->line, name->column, "name is too long");
        return 0;
    }
    stmt->as.var_stmt.declared_type = declared;
    stmt->as.var_stmt.initializer = init;
    stmt->as.var_stmt.has_initializer = has_initializer;
    stmt->as.var_stmt.is_const = is_const;
    stmt->as.var_stmt.line = name->line;
    return 1;
    /* we implement a function to parse variable-like declarations in our programming language, 
       which can include both regular variables and constants.
       this function handles the syntax and semantics of variable declarations, 
       including optional type annotations and initializers, 
       while also performing necessary type checks and error reporting.
       by using a single function for both variables and constants, 
       we can reduce code duplication and maintain a consistent approach to parsing these related constructs. */
}

static int parse_switch(Parser *p, Statement *stmt)
{
    char buf1[128], buf2[128];
    Expr *value = parse_expression(p);
    TypeRef vt = infer_expr(p, value);
    if (vt.kind != TYPE_I64 && vt.kind != TYPE_BOOL)
    {
        /**
         * Adds a diagnostic message to the parser's error list.
         * @param src The source file.
         * @param errors The error list.
         * @param line The line number.
         * @param column The column number.
         * @param fmt The format string for the error message.
         */
        diagnostics_add(p->src, p->errors, value->line, value->column, "switch currently supports only i64/bool expressions, got %s", type_display_name(vt, buf1, sizeof(buf1)));
        return 0;
    }
    if (!expect(p, TOKEN_COLON, "expected ':' after switch expression") || !expect(p, TOKEN_NEWLINE, "expected newline after switch header") || !expect(p, TOKEN_INDENT, "expected indented switch block"))
        return 0;
        /* we implement a function to parse a switch statement in our programming language.
           this function handles the syntax and semantics of switch statements, 
           including parsing the switch expression, validating its type, 
           and processing the case and default labels along with their associated blocks of statements.
           by centralizing switch statement parsing in this function, 
           we can ensure consistent handling of this control flow construct and 
           provide clear error messages when syntax or type issues arise. */

    SwitchCase temp[256];
    size_t count = 0;
    /* we use a temporary array to store switch cases as we parse them, 
    allowing us to handle an arbitrary number of cases without needing to know the count upfront. 
    Once we've parsed all cases, we can allocate the exact amount of memory needed and copy the cases over, 
    ensuring efficient memory usage while maintaining flexibility in the number of cases supported. */
    memset(temp, 0, sizeof(temp));
    while (cur(p)->type != TOKEN_DEDENT && cur(p)->type != TOKEN_EOF)
    {
        while (match(p, TOKEN_NEWLINE))
        {
        }
        if (cur(p)->type == TOKEN_DEDENT)
            break;
        if (match(p, TOKEN_CASE))
        /**
         * Parses a case label in a switch statement.
         * @param p The parser.
         * @param stmt The statement being parsed.
         * @return 1 if the case label is parsed successfully, 0 otherwise.
         */
        {
            temp[count].line = prev(p)->line;
            temp[count].match = parse_expression(p);
            TypeRef ct = infer_expr(p, temp[count].match);
            if (!type_equals(ct, vt))
            {
                diagnostics_add(p->src, p->errors, temp[count].line, prev(p)->column, "switch case type mismatch: expected %s, got %s", type_display_name(vt, buf1, sizeof(buf1)), type_display_name(ct, buf2, sizeof(buf2)));
                return 0;
                /* we implement a function to parse a case label in a switch statement.
                   this function is responsible for parsing the case expression, 
                   validating its type against the switch expression, 
                   and reporting any type mismatches with clear error messages.
                   by centralizing case label parsing in this function, 
                   we can maintain consistent handling of case labels and ensure that 
                   type errors are caught effectively during the parsing process. */
            }
        }
        else if (match(p, TOKEN_DEFAULT))
        {
            temp[count].is_default = 1;
            temp[count].line = prev(p)->line;
        }
        else
        {
            diagnostics_add(p->src, p->errors, cur(p)->line, cur(p)->column, "expected case or default inside switch");
            return 0;
        }
        if (!expect(p, TOKEN_COLON, "expected ':' after switch label") || !expect(p, TOKEN_NEWLINE, "expected newline after switch label") || !expect(p, TOKEN_INDENT, "expected indented case block"))
            return 0;
        if (!parse_block(p, &temp[count].body, &temp[count].body_count))
            return 0;
        ++count;
    }
    if (!expect(p, TOKEN_DEDENT, "expected end of switch block"))
        return 0;
    while (match(p, TOKEN_NEWLINE))
    /**
     * Parses a switch statement.
     * @param p The parser.
     * @param stmt The statement being parsed.
     * @return 1 if the switch statement is parsed successfully, 0 otherwise.
     */
    {
    }
    stmt->kind = STMT_SWITCH;
    stmt->as.switch_stmt.value = value;
    stmt->as.switch_stmt.case_count = count;
    stmt->as.switch_stmt.cases = alloc_switch_cases(p, count);
    memcpy(stmt->as.switch_stmt.cases, temp, count * sizeof(SwitchCase));
    /* we implement a function to parse a switch statement in our programming language.
       this function handles the syntax and semantics of switch statements, 
       including parsing the switch expression, validating its type, and processing the 
       case and default labels along with their associated blocks of statements.
       by centralizing switch statement parsing in this function, 
       we can ensure consistent handling of this control flow construct and 
       provide clear error messages when syntax or type issues arise. */
    stmt->as.switch_stmt.line = value->line;
    return 1;
}

static int parse_statement(Parser *p, Statement *stmt)
{
    char buf1[128], buf2[128];
    while (match(p, TOKEN_NEWLINE))
    {
    }
    if (cur(p)->type == TOKEN_DEDENT || cur(p)->type == TOKEN_EOF)
        return 0;
    memset(stmt, 0, sizeof(*stmt));

    if (match(p, TOKEN_LABEL))
    {
        const Token *name = cur(p);
        if (!expect(p, TOKEN_IDENTIFIER, "expected label name"))
            return 0;
        if (!expect(p, TOKEN_COLON, "expected ':' after label name") || !expect(p, TOKEN_NEWLINE, "expected end of line after label"))
            return 0;
        stmt->kind = STMT_LABEL;
        if (!copy_cstr(stmt->as.label_stmt.name, sizeof(stmt->as.label_stmt.name), name->lexeme))
        {
            diagnostics_add(p->src, p->errors, name->line, name->column, "label name is too long");
            return 0;
        }
        stmt->as.label_stmt.line = name->line;
        /* we implement a function to parse a label statement in our programming language.
           this function is responsible for parsing the label name, ensuring it follows the correct syntax, 
           and storing it in the statement structure for later use during code generation or interpretation.
           by centralizing label parsing in this function, 
           we can maintain consistent handling of labels and 
           provide clear error messages when syntax issues arise. */
        return 1;
    }
    if (match(p, TOKEN_GOTO))
    {
        const Token *name = cur(p);
        if (!expect(p, TOKEN_IDENTIFIER, "expected label target after goto") || !expect(p, TOKEN_NEWLINE, "expected end of line after goto"))
            return 0;
        stmt->kind = STMT_GOTO;
        if (!copy_cstr(stmt->as.goto_stmt.name, sizeof(stmt->as.goto_stmt.name), name->lexeme))
        {
            diagnostics_add(p->src, p->errors, name->line, name->column, "goto label name is too long");
            return 0;
        }
        stmt->as.goto_stmt.line = name->line;
        return 1;
        /* we implement a function to parse a goto statement in our programming language.
           this function is responsible for parsing the target label name, 
           ensuring it follows the correct syntax, and storing it in the statement 
           structure for later use during code generation or interpretation.
           by centralizing goto parsing in this function, we can maintain consistent handling of 
           goto statements and provide clear error messages when syntax issues arise. */
    }
    if (match(p, TOKEN_VAR))
        return parse_var_like(p, stmt, 0);
    if (match(p, TOKEN_CONST))
        return parse_var_like(p, stmt, 1);
    if (match(p, TOKEN_PRINT))
    {
        Expr *value = parse_expression(p);
        if (!expect(p, TOKEN_NEWLINE, "expected end of line after print"))
            return 0;
        infer_expr(p, value);
        stmt->kind = STMT_PRINT;
        stmt->as.print_stmt.value = value;
        stmt->as.print_stmt.line = value->line;
        /* we implement a function to parse a print statement in our programming language.
           this function is responsible for parsing the expression to be printed, 
           ensuring it follows the correct syntax, and storing it in the statement structure for 
           later use during code generation or interpretation.
           by centralizing print parsing in this function, we can maintain consistent handling of 
           print statements and provide clear error messages when syntax issues arise. */
        return 1;
    }
    if (match(p, TOKEN_RET))
    {
        Expr *value = NULL;
        if (cur(p)->type != TOKEN_NEWLINE)
            value = parse_expression(p);
        if (!expect(p, TOKEN_NEWLINE, "expected end of line after ret"))
            return 0;
        TypeRef t = value ? infer_expr(p, value) : make_type(TYPE_VOID, NULL);
        if (p->current_function && !type_is_assignable(p->current_function->return_type, t) && t.kind != TYPE_INVALID)
        {
            diagnostics_add(p->src, p->errors, prev(p)->line, prev(p)->column, "return type mismatch: function '%s' expects %s, got %s", p->current_function->name, type_display_name(p->current_function->return_type, buf1, sizeof(buf1)), type_display_name(t, buf2, sizeof(buf2)));
        }
        stmt->kind = STMT_RETURN;
        stmt->as.return_stmt.value = value;
        stmt->as.return_stmt.line = prev(p)->line;
        /* we implement a function to parse a return statement in our programming language.
           this function is responsible for parsing the return value (if any), 
           ensuring it follows the correct syntax, and storing it in the statement 
           structure for later use during code generation or interpretation.
           by centralizing return parsing in this function, we can maintain 
           consistent handling of return statements and provide clear error 
           messages when syntax issues arise. */
        return 1;
    }
    if (match(p, TOKEN_IF) || match(p, TOKEN_ELIF))
    /**
     * Parses an if or elif statement.
     * @param p The parser.
     * @param stmt The statement being parsed.
     * @return 1 if the statement is parsed successfully, 0 otherwise.
     */
    {
        Expr *cond = parse_expression(p);
        infer_expr(p, cond);
        if (!expect(p, TOKEN_COLON, "expected ':' after if/elif condition") || !expect(p, TOKEN_NEWLINE, "expected newline after if/elif header") || !expect(p, TOKEN_INDENT, "expected indented if/elif block"))
            return 0;
        if (!parse_block(p, &stmt->as.if_stmt.then_block, &stmt->as.if_stmt.then_count))
            return 0;
        if (match(p, TOKEN_ELIF))
        {
            Statement *nested = alloc_block(p, 1);
            if (!nested)
                return 0;
            p->index--;
            /* we implement a function to parse an if or elif statement in our programming language.
               this function is responsible for parsing the condition expression, 
               ensuring it follows the correct syntax, and then parsing the associated blocks 
               for the if/elif and any subsequent else or elif clauses.
               by centralizing if/elif parsing in this function, 
               we can maintain consistent handling of these control flow 
               constructs and provide clear error messages when syntax issues arise. */
            if (!parse_statement(p, &nested[0]))
                return 0;
            stmt->as.if_stmt.else_block = nested;
            stmt->as.if_stmt.else_count = 1;
        }
        else if (match(p, TOKEN_ELSE))
        {
            if (!expect(p, TOKEN_COLON, "expected ':' after else") || !expect(p, TOKEN_NEWLINE, "expected newline after else") || !expect(p, TOKEN_INDENT, "expected indented else block"))
                return 0;
            if (!parse_block(p, &stmt->as.if_stmt.else_block, &stmt->as.if_stmt.else_count))
                return 0;
                /* we implement a function to parse an if or elif statement in our programming language.
                   this function is responsible for parsing the condition expression, 
                   ensuring it follows the correct syntax, and then parsing the associated blocks for 
                   the if/elif and any subsequent else or elif clauses.
                   by centralizing if/elif parsing in this function, 
                   we can maintain consistent handling of these control flow constructs and 
                   provide clear error messages when syntax issues arise. */
        }
        stmt->kind = STMT_IF;
        stmt->as.if_stmt.condition = cond;
        stmt->as.if_stmt.line = cond->line;
        return 1;
    }
    /**
     * Parses a while statement.
     * @param p The parser.
     * @param stmt The statement being parsed.
     * @return 1 if the statement is parsed successfully, 0 otherwise.
     */
    if (match(p, TOKEN_WHILE))
    {
        Expr *cond = parse_expression(p);
        infer_expr(p, cond);
        if (!expect(p, TOKEN_COLON, "expected ':' after while condition") || !expect(p, TOKEN_NEWLINE, "expected newline after while header") || !expect(p, TOKEN_INDENT, "expected indented while block"))
            return 0;
        if (!parse_block(p, &stmt->as.while_stmt.body, &stmt->as.while_stmt.body_count))
            return 0;
            /* we implement a function to parse a while statement in our programming language.
               this function is responsible for parsing the loop condition, 
               ensuring it follows the correct syntax, and then parsing the associated block of 
               statements that make up the body of the loop.
               by centralizing while statement parsing in this function, 
               we can maintain consistent handling of this control flow construct and 
               provide clear error messages when syntax issues arise. */
        stmt->kind = STMT_WHILE;
        stmt->as.while_stmt.condition = cond;
        stmt->as.while_stmt.line = cond->line;
        return 1;
    }
    if (match(p, TOKEN_SWITCH))
        return parse_switch(p, stmt);

    Expr *lhs = parse_expression(p);
    if (match(p, TOKEN_EQUAL))
    {
        Expr *rhs = parse_expression(p);
        if (!expect(p, TOKEN_NEWLINE, "expected end of line after assignment"))
            return 0;
        if (lhs->kind == EXPR_IDENTIFIER)
        {
            /* we implement a function to parse an assignment statement in our programming language.
               this function is responsible for parsing the left-hand side (LHS) and right-hand side (RHS) expressions, 
               ensuring that the LHS is a valid assignable expression (like a variable), 
               and then performing type checks to ensure that the assignment is valid according to the language's type system.
               by centralizing assignment parsing in this function, 
               we can maintain consistent handling of assignments and provide clear 
               error messages when syntax or type issues arise. */
            Symbol *s = find_symbol(p, lhs->as.text);
            if ((s && s->is_const) || (find_global(p->program, lhs->as.text) && find_global(p->program, lhs->as.text)->is_const))
            {
                diagnostics_add(p->src, p->errors, lhs->line, lhs->column, "cannot assign to const '%s'", lhs->as.text);
            }
        }
        TypeRef a = infer_expr(p, lhs), b = infer_expr(p, rhs);
        if (!type_is_assignable(a, b) && a.kind != TYPE_INVALID && b.kind != TYPE_INVALID)
        {
            diagnostics_add(p->src, p->errors, lhs->line, lhs->column, "cannot assign %s to %s", type_display_name(b, buf1, sizeof(buf1)), type_display_name(a, buf2, sizeof(buf2)));
        }

        stmt->kind = STMT_ASSIGN;
        stmt->as.assign_stmt.target = lhs;
        stmt->as.assign_stmt.value = rhs;
        stmt->as.assign_stmt.line = lhs->line;
        return 1;
    }
    if (!expect(p, TOKEN_NEWLINE, "expected end of line after expression"))
        return 0;
    infer_expr(p, lhs);
    stmt->kind = STMT_EXPR;
    stmt->as.expr_stmt.expr = lhs;
    stmt->as.expr_stmt.line = lhs->line;
    /* we implement a function to parse an expression statement in our programming language.
       this function is responsible for parsing the expression, 
       ensuring it follows the correct syntax, and then storing it in the statement structure 
       for later use during code generation or interpretation.
       by centralizing expression statement parsing in this function, 
       we can maintain consistent handling of these statements and provide clear error 
       messages when syntax issues arise. */
    return 1;
}

int parse_block(Parser *p, Statement **out, size_t *out_count)
/**
 * Parses a block of statements.
 * @param p The parser.
 * @param out The output array of statements.
 * @param out_count The number of statements parsed.
 * @return 1 if the block is parsed successfully, 0 otherwise.
 */
{
    Statement *temp = (Statement *)calloc(MACHINE_MAX_STATEMENTS, sizeof(Statement));
    size_t count = 0;
    if (!temp)
    {
        diagnostics_add(p->src, p->errors, cur(p)->line, cur(p)->column, "out of memory while building block");
        return 0;
    }
    /* we implement a function to parse a block of statements in our programming language.
       this function is responsible for parsing a sequence of statements that are indented together, 
       typically following a control flow statement like if, while, or switch.
       it uses a temporary array to store the parsed statements and keeps track of the count, 
       allowing for flexible handling of blocks with varying numbers of statements.
       by centralizing block parsing in this function, we can maintain consistent handling 
       of statement blocks and provide clear error messages when syntax issues arise. */
    while (cur(p)->type != TOKEN_DEDENT && cur(p)->type != TOKEN_EOF)
    {
        while (match(p, TOKEN_NEWLINE))
        {
        }
        if (cur(p)->type == TOKEN_DEDENT || cur(p)->type == TOKEN_EOF)
            break;
        if (!parse_statement(p, &temp[count]))
            break;
        ++count;
    }
    if (!expect(p, TOKEN_DEDENT, "expected end of indented block"))
    {
        free(temp);
        return 0;
    }
    Statement *block = alloc_block(p, count);
    if (!block)
    {
        free(temp);
        return 0;
    }
    memcpy(block, temp, count * sizeof(Statement));
    free(temp);
    *out = block;
    *out_count = count;
    while (match(p, TOKEN_NEWLINE))
    {
    }
    return 1;
}

static void warn_unused_symbols(Parser *p)
/**
 * Warns about unused symbols in the program.
 * @param p The parser.
 */
{
    for (size_t i = 0; i < p->symbol_count; ++i)
        if (!p->symbols[i].used)
            diagnostics_add(p->src, p->warnings, p->symbols[i].line, 1, "unused variable '%s'", p->symbols[i].name);
}

static int parse_struct_decl(Parser *p)
{
    const Token *kw = cur(p);
    if (!expect(p, TOKEN_STRUCT, "expected 'struct'"))
        return 0;
    const Token *name = cur(p);
    if (!expect(p, TOKEN_IDENTIFIER, "expected struct name"))
        return 0;
    if (is_reserved_name(name->lexeme))
    /**
     * Checks if a name is reserved.
     * @param name The name to check.
     * @return 1 if the name is reserved, 0 otherwise.
     */
    {
        diagnostics_add(p->src, p->errors, name->line, name->column, "'%s' is reserved and cannot be used as a struct name", name->lexeme);
        return 0;
    }
    if (find_struct(p->program, name->lexeme))
    {
        diagnostics_add(p->src, p->errors, name->line, name->column, "redefinition of struct '%s'", name->lexeme);
        return 0;
        /* we implement a function to parse a struct declaration in our programming language.
           this function is responsible for parsing the struct keyword, 
           the struct name, and the body of the struct which includes its fields.
           it also performs checks for reserved names and redefinitions to 
           ensure that the struct declaration is valid within the context of the program.
           by centralizing struct declaration parsing in this function, 
           we can maintain consistent handling of structs and provide clear 
           error messages when syntax or semantic issues arise. */
    }
    if (!expect(p, TOKEN_COLON, "expected ':' after struct name") || !expect(p, TOKEN_NEWLINE, "expected newline after struct header") || !expect(p, TOKEN_INDENT, "expected indented struct block"))
        return 0;
    StructDecl *sd = &p->program->structs[p->program->struct_count++];
    /* we allocate a new struct declaration in the program's struct array, 
       ensuring that we have space for it and that we track the count of structs declared.
       this allows us to store the details of the struct, 
       including its name and fields, for later use during type checking and code generation. */
    memset(sd, 0, sizeof(*sd));
    sd->line = kw->line;
    if (!copy_cstr(sd->name, sizeof(sd->name), name->lexeme))
    {
        diagnostics_add(p->src, p->errors, name->line, name->column, "struct name is too long");
        return 0;
    }
    /* we implement a function to parse a struct declaration in our programming language.
       this function is responsible for parsing the struct keyword, 
       the struct name, and the body of the struct which includes its fields.
       it also performs checks for reserved names and 
       redefinitions to ensure that the struct declaration is valid within the context of the program.
       by centralizing struct declaration parsing in this function, 
       we can maintain consistent handling of structs and 
       provide clear error messages when syntax or semantic issues arise. */
    while (cur(p)->type != TOKEN_DEDENT && cur(p)->type != TOKEN_EOF)
    /**
     * Parses the body of a struct declaration.
     * @param p The parser.
     * @return 1 if the struct body is parsed successfully, 0 otherwise.
     */
    {
        while (match(p, TOKEN_NEWLINE))
        {
        }
        if (cur(p)->type == TOKEN_DEDENT)
            break;
        const Token *fname = cur(p);
        if (!expect(p, TOKEN_IDENTIFIER, "expected field name in struct"))
            return 0;
        if (!expect(p, TOKEN_COLON, "expected ':' after field name"))
            return 0;
        if (sd->field_count >= MACHINE_MAX_FIELDS)
        {
            diagnostics_add(p->src, p->errors, fname->line, fname->column, "too many fields in struct '%s'", sd->name);
            return 0;
        }
        StructField *fd = &sd->fields[sd->field_count++];
        if (!copy_cstr(fd->name, sizeof(fd->name), fname->lexeme))
        /* we parse the body of a struct declaration, which consists of field declarations.
           for each field, we parse its name and type, 
           ensuring that the syntax is correct and that we do not exceed the 
           maximum number of fields allowed in a struct.
           we also perform checks for reserved names and provide clear error messages when 
           issues arise during parsing. */
        {
            diagnostics_add(p->src, p->errors, fname->line, fname->column, "field name is too long");
            return 0;
        }
        fd->line = fname->line;
        if (!parse_type_ref(p, &fd->type))
            return 0;
        if (!expect(p, TOKEN_NEWLINE, "expected end of line after struct field"))
            return 0;
    }
    if (!expect(p, TOKEN_DEDENT, "expected end of struct block"))
        return 0;
    while (match(p, TOKEN_NEWLINE))
    {
    }
    return 1;
}

static int add_global(Parser *p, const char *name, TypeRef type, Expr *init, int has_initializer, int is_const, int line)
/**
 * Adds a global variable declaration to the program.
 * @param p The parser.
 * @param name The name of the global variable.
 * @param type The type of the global variable.
 * @param init The initializer expression for the global variable.
 * @param has_initializer Whether the global variable has an initializer.
 * @param is_const Whether the global variable is constant.
 * @param line The line number of the global variable declaration.
 * @return 1 if the global variable is added successfully, 0 otherwise.
 */
{
    if (is_reserved_name(name))
    {
        diagnostics_add(p->src, p->errors, line, 1, "'%s' is reserved and cannot be used as a global variable name", name);
        return 0;
    }
    /* we implement a function to add a global variable declaration to the program.
       this function checks for reserved names and redefinitions to ensure that 
       the global variable name is valid within the context of the program.
       it also checks that we do not exceed the 
       maximum number of global variables allowed in the program.
       if all checks pass, it allocates a new global variable declaration in the 
       program's global array and initializes its properties based on the provided parameters. */
    if (find_global(p->program, name) || find_function(p->program, name) || find_struct(p->program, name) || find_module(p->program, name))
    {
        diagnostics_add(p->src, p->errors, line, 1, "redefinition of top-level name '%s'", name);
        return 0;
    }
    if (p->program->global_count >= MACHINE_MAX_SYMBOLS)
    {
        diagnostics_add(p->src, p->errors, line, 1, "too many global variables");
        return 0;
    }
    GlobalVarDecl *g = &p->program->globals[p->program->global_count++];
    memset(g, 0, sizeof(*g));
    if (!copy_cstr(g->name, sizeof(g->name), name))
    {
        diagnostics_add(p->src, p->errors, line, 1, "global name is too long");
        return 0;
    }

    g->declared_type = type;
    g->initializer = init;
    g->has_initializer = has_initializer;
    g->line = line;
    g->is_const = is_const;
    return 1;
}

static int parse_global_var(Parser *p, int is_const)
/**
 * Parses a global variable declaration.
 * @param p The parser.
 * @param is_const Whether the global variable is constant.
 * @return 1 if the global variable is parsed successfully, 0 otherwise.
 */
{
    char buf1[128], buf2[128];
    const Token *kw = cur(p);
    (void)kw;
    if (!expect(p, is_const ? TOKEN_CONST : TOKEN_VAR, is_const ? "expected 'const'" : "expected 'var'"))
    /* we implement a function to parse a global variable declaration in our programming language.
       this function handles the syntax and semantics of global variable declarations, 
       including optional type annotations and initializers, while also performing necessary type checks and error reporting.
       by using a single function for both variables and constants, 
       we can reduce code duplication and maintain a consistent approach to parsing these related constructs. */
        return 0;
    const Token *name = cur(p);
    if (!expect(p, TOKEN_IDENTIFIER, is_const ? "expected global constant name after const" : "expected global variable name after var"))
        return 0;
    TypeRef declared = make_type(TYPE_UNKNOWN, NULL);
    /* we start with an unknown type for the declared type, 
       which allows us to handle cases where the type is not explicitly specified but can be 
       inferred from the initializer.
       this approach provides flexibility in the syntax of global variable declarations while 
       still ensuring that we can perform necessary type checks and 
       report errors when the type cannot be determined. */
    int has_initializer = 0;
    Expr *init = NULL;
    if (match(p, TOKEN_COLON) && !parse_type_ref(p, &declared))
        return 0;
    if (match(p, TOKEN_EQUAL))
    {
        has_initializer = 1;
        init = parse_expression(p);
    }
    if (!expect(p, TOKEN_NEWLINE, "expected end of line after global variable declaration"))
        return 0;
    TypeRef init_type = has_initializer ? infer_expr(p, init) : declared;
    if (declared.kind == TYPE_UNKNOWN)
        declared = init_type;
        /* if the declared type is still unknown after parsing the initializer, 
           it means we have no information about the type of the global variable, 
           which is an error since global variables must have a known type.
           we report this error to the user, indicating that the 
           global variable needs a type or an initializer to determine its type. */
    if (declared.kind == TYPE_UNKNOWN)
    {
        diagnostics_add(p->src, p->errors, name->line, name->column, "global variable '%s' needs a type or initializer", name->lexeme);
        return 0;
    }
    if (has_initializer && !type_is_assignable(declared, init_type))
    {
        diagnostics_add(p->src, p->errors, name->line, name->column, "cannot initialize global %s with %s", type_display_name(declared, buf1, sizeof(buf1)), type_display_name(init_type, buf2, sizeof(buf2)));
        return 0;
    }
    if (is_const && !has_initializer)
    {
        diagnostics_add(p->src, p->errors, name->line, name->column, "global const '%s' requires an initializer", name->lexeme);
        return 0;
    }
    return add_global(p, name->lexeme, declared, init, has_initializer, is_const, name->line);
}

static int parse_function(Parser *p)
/**
 * Parses a function declaration.
 * @param p The parser.
 * @return 1 if the function is parsed successfully, 0 otherwise.
 */
{
    FunctionDecl *f = &p->program->functions[p->program->function_count++];
    memset(f, 0, sizeof(*f));
    if (match(p, TOKEN_MAIN))
    {
        if (p->current_module[0])
        {
            diagnostics_add(p->src, p->errors, cur(p)->line, cur(p)->column, "main: cannot appear inside a module");
            /* we implement a function to parse the main 
               function declaration in our programming language.
               this function checks that the main function is not declared inside a module, 
               as this is not allowed according to the language's rules.
               if the main function is declared correctly, it initializes its properties, 
               including setting its name to "main" and its return type to i64, 
               which is the expected signature for the main function in this language. */
            return 0;
        }
        f->is_main = true;
        strcpy(f->name, "main");
        strcpy(f->source_name, "main");
        f->return_type = make_type(TYPE_I64, NULL);
        if (!expect(p, TOKEN_COLON, "expected ':' after main") || !expect(p, TOKEN_NEWLINE, "expected newline after main header") || !expect(p, TOKEN_INDENT, "expected indented main block"))
            return 0;
    }
    else
    {
        const Token *start = cur(p);
        if (!expect(p, TOKEN_FUNC, "expected 'func'"))
            return 0;
        const Token *name = cur(p);
        if (!expect(p, TOKEN_IDENTIFIER, "expected function name"))
            return 0;
        if (is_reserved_name(name->lexeme))
        {
            diagnostics_add(p->src, p->errors, name->line, name->column, "'%s' is reserved and cannot be used as a function name", name->lexeme);
            return 0;
        }
        if (!copy_cstr(f->source_name, sizeof(f->source_name), name->lexeme))
        {
            diagnostics_add(p->src, p->errors, name->line, name->column, "function name is too long");
            return 0;
        }
        if (!copy_cstr(f->module_name, sizeof(f->module_name), p->current_module))
        {
            diagnostics_add(p->src, p->errors, name->line, name->column, "module name is too long");
            return 0;
        }
        if (p->current_module[0])
        {
            if (!join_cstr3(f->name, sizeof(f->name), p->current_module, "__", name->lexeme))
            {
                diagnostics_add(p->src, p->errors, name->line, name->column, "qualified function name is too long");
                return 0;
            }
        }
        else if (!copy_cstr(f->name, sizeof(f->name), name->lexeme))
        {
            diagnostics_add(p->src, p->errors, name->line, name->column, "function name is too long");
            return 0;
        }
        f->line = start->line;
        /* we implement a function to parse a function declaration in our programming language.
           this function is responsible for parsing the function keyword, 
           the function name, the parameter list, and the return type, as well as the body of the function.
           it also performs checks for reserved names and redefinitions to ensure that the 
           function declaration is valid within the context of the program.
           by centralizing function declaration parsing in this function, 
           we can maintain consistent handling of functions and 
           provide clear error messages when syntax or semantic issues arise. */
        if (!expect(p, TOKEN_LPAREN, "expected '(' after function name"))
            return 0;
        if (!match(p, TOKEN_RPAREN))
        {
            do
            {
                const Token *param_name = cur(p);
                if (!expect(p, TOKEN_IDENTIFIER, "expected parameter name") || !expect(p, TOKEN_COLON, "expected ':' after parameter name"))
                    return 0;
                Param *param = &f->params[f->param_count++];
                if (!copy_cstr(param->name, sizeof(param->name), param_name->lexeme))
                {
                    diagnostics_add(p->src, p->errors, param_name->line, param_name->column, "parameter name is too long");
                    return 0;
                }
                if (!parse_type_ref(p, &param->type))
                    return 0;
            } while (match(p, TOKEN_COMMA));
            if (!expect(p, TOKEN_RPAREN, "expected ')' after parameter list"))
                return 0;
        }
        if (!expect(p, TOKEN_ARROW, "expected '->' before return type") || !parse_type_ref(p, &f->return_type))
            return 0;
        if (!expect(p, TOKEN_COLON, "expected ':' after function signature") || !expect(p, TOKEN_NEWLINE, "expected newline after function header") || !expect(p, TOKEN_INDENT, "expected indented function block"))
            return 0;
    }
    p->current_function = f;
    p->symbol_count = 0;
    for (size_t i = 0; i < f->param_count; ++i)
        add_local(p, f->params[i].name, f->params[i].type, f->line);
    if (!parse_block(p, &f->body, &f->body_count))
    /* after parsing the function body, we check for any unused local variables and parameters, 
       and we add warnings for them to help the programmer identify potential issues in their code.
       this is an important step in the parsing process, 
       as it helps maintain code quality and encourages good programming practices by 
       alerting the programmer to variables that are declared but never used. */
        return 0;
    warn_unused_symbols(p);
    p->current_function = NULL;
    return 1;
}

static int parse_module_decl(Parser *p)
{
    if (!expect(p, TOKEN_MODULE, "expected 'module'"))
        return 0;
    const Token *name = cur(p);
    if (!expect(p, TOKEN_IDENTIFIER, "expected module name"))
        return 0;
    if (is_reserved_name(name->lexeme))
    {
        diagnostics_add(p->src, p->errors, name->line, name->column, "'%s' is reserved and cannot be used as a module name", name->lexeme);
        return 0;
    }
    if (find_module(p->program, name->lexeme))
    {
        diagnostics_add(p->src, p->errors, name->line, name->column, "redefinition of module '%s'", name->lexeme);
        return 0;
    }
    ModuleDecl *md = &p->program->modules[p->program->module_count++];
    /* we implement a function to parse a module declaration in our programming language.
       this function is responsible for parsing the module keyword, 
       the module name, and the body of the module which includes its contents such as functions and variables.
       it also performs checks for reserved names and redefinitions to 
       ensure that the module declaration is valid within the context of the program.
       by centralizing module declaration parsing in this function, 
       we can maintain consistent handling of modules and 
       provide clear error messages when syntax or semantic issues arise. */
    memset(md, 0, sizeof(*md));
    if (!copy_cstr(md->name, sizeof(md->name), name->lexeme))
    {
        diagnostics_add(p->src, p->errors, name->line, name->column, "module name is too long");
        return 0;
    }
    md->line = name->line;
    if (!expect(p, TOKEN_COLON, "expected ':' after module name") || !expect(p, TOKEN_NEWLINE, "expected newline after module header") || !expect(p, TOKEN_INDENT, "expected indented module block"))
        return 0;
    if (!copy_cstr(p->current_module, sizeof(p->current_module), md->name))
    {
        diagnostics_add(p->src, p->errors, name->line, name->column, "module name is too long");
        return 0;
    }
    while (cur(p)->type != TOKEN_DEDENT && cur(p)->type != TOKEN_EOF)
    {
        while (match(p, TOKEN_NEWLINE))
        {
        }
        if (cur(p)->type == TOKEN_DEDENT)
            break;
        if (cur(p)->type != TOKEN_FUNC)
        {
            diagnostics_add(p->src, p->errors, cur(p)->line, cur(p)->column, "module body currently supports only func declarations");
            return 0;
        }
        if (!parse_function(p))
            return 0;
    }
    p->current_module[0] = '\0';
    /* after parsing the module body, we check for any unused symbols within the 
       module and add warnings for them to help the programmer identify potential issues in their code.
       this is an important step in the parsing process, 
       as it helps maintain code quality and encourages good programming practices by 
       alerting the programmer to variables and functions that are 
       declared but never used within the module. */
    if (!expect(p, TOKEN_DEDENT, "expected end of module block"))
        return 0;
    while (match(p, TOKEN_NEWLINE))
    {
    }
    return 1;
}

/* Parses the entire program from the list of tokens and builds the program structure.
 * @param src The source file information for error reporting.
 * @param tokens The list of tokens to parse.
 * @param program The output program structure to populate with parsed data.
 * @param errors The list to which parsing errors will be added.
 * @param warnings The list to which parsing warnings will be added.
 * @return 1 if the program is parsed successfully, 0 otherwise.
 */
bool parse_program(const SourceFile *src, const TokenList *tokens, Program *program, DiagnosticList *errors, DiagnosticList *warnings)
{
    memset(program, 0, sizeof(*program));
    Parser p = {0};
    p.src = src;
    p.tokens = tokens;
    p.program = program;
    p.errors = errors;
    p.warnings = warnings;
    while (match(&p, TOKEN_NEWLINE))
    {
    }
    while (cur(&p)->type != TOKEN_EOF)
    {
        while (match(&p, TOKEN_NEWLINE))
        {
        }
        if (cur(&p)->type == TOKEN_EOF)
            break;
        if (cur(&p)->type == TOKEN_STRUCT)
        {
            if (!parse_struct_decl(&p))
                return 0;
            continue;
        }
        if (cur(&p)->type == TOKEN_MODULE)
        {
            if (!parse_module_decl(&p))
                return 0;
            continue;
        }
        if (cur(&p)->type == TOKEN_VAR)
        {
            if (!parse_global_var(&p, 0))
                return 0;
            continue;
        }
        if (cur(&p)->type == TOKEN_CONST)
        {
            if (!parse_global_var(&p, 1))
                return 0;
            continue;
        }
        if (cur(&p)->type != TOKEN_MAIN && cur(&p)->type != TOKEN_FUNC)
        {
            diagnostics_add(src, errors, cur(&p)->line, cur(&p)->column, "top-level items must be 'var', 'const', 'struct', 'module', 'main:' or 'func ...:'");
            return 0;
        }
        if (!parse_function(&p))
            return 0;
        while (match(&p, TOKEN_NEWLINE))
        {
        }
    }
    if (!find_function(program, "main"))
    {
        diagnostics_add(src, errors, 1, 1, "program must define main:");
        /* after parsing the entire program, we check for the presence of 
           the main function, which is required as the entry point of the program.
           if the main function is not defined, we report an error to the user, 
           indicating that the program must have a main function to be valid. */
        return 0;
    }
    for (size_t i = 0; i < program->function_count; ++i)
    {
        if (!program->functions[i].is_main && !program->functions[i].used)
        {
            const char *warn_name = program->functions[i].source_name[0] ? program->functions[i].source_name : program->functions[i].name;
            diagnostics_add(src, warnings, program->functions[i].line ? program->functions[i].line : 1, 1, "unused function '%s'", warn_name);
        }
    }
    /* after parsing the entire program, we check for any unused functions and global variables, 
       and we add warnings for them to help the programmer identify potential issues in their code.
       this is an important step in the parsing process, 
       as it helps maintain code quality and encourages good programming practices by 
       alerting the programmer to functions and global variables that are declared but never used. */
    for (size_t i = 0; i < program->global_count; ++i)
    {
        if (!program->globals[i].used)
            diagnostics_add(src, warnings, program->globals[i].line, 1, "unused global variable '%s'", program->globals[i].name);
    }
    return errors->count == 0;
}

void free_program(Program *program)
{
    for (size_t i = 0; i < program->expr_pool_count; ++i)
        free(program->expr_pool[i]);
    for (size_t i = 0; i < program->stmt_block_count; ++i)
        free(program->stmt_blocks[i]);
    for (size_t i = 0; i < program->switch_case_block_count; ++i)
        free(program->switch_case_blocks[i]);
    memset(program, 0, sizeof(*program));
}
