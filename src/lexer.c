/*
 * Annotated reading copy of lexer.c
 *
 * What this file is for:
 * - Turn raw Machine source text into a token stream that later parser stages can consume reliably.
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

/*
 * Machine lexer.
 *
 * Responsibilities:
 *  1. Convert preprocessed source text into tokens.
 *  2. Enforce the project's indentation rule (each block step may indent by 2 or 4 spaces).
 *  3. Assume Machine '--' comments were already stripped by preprocess_machine_source().
 */

#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

/*
 * Function overview: push
 *
 * High-level purpose:
 * - This routine belongs to lexer.c.
 * - It exists to turn raw machine source text into a token stream that later parser stages can consume reliably.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "push".
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
static void push(TokenList *tokens, TokenType type, const char *lexeme, int line, int column)
{
    /* we implement a function to add a new token to the list of tokens during the lexing process.
     *       this function takes care of creating a new token with the
     *       specified type, lexeme, line number, and column number, and it adds the token to the token list
     *       while ensuring that we do not exceed the maximum number of tokens allowed.
     *       by centralizing token creation in this function,
     *       we can maintain a consistent approach to token management and
     *       handle any potential errors related to token limits in one place. */
    if (tokens->count >= MACHINE_MAX_TOKENS)
    {
        return;
    }
    Token *t = &tokens->items[tokens->count++];
    t->type = type;
    snprintf(t->lexeme, sizeof(t->lexeme), "%s", lexeme ? lexeme : "");
    t->line = line;
    t->column = column;
}

/* we implement a function to determine if a given word is a reserved keyword in our programming language,
 *   which cannot be used as an identifier for variables, functions, or other user-defined names.
 *   this function checks the input word against a list of reserved keywords and
 *   returns true if the word is reserved, and false otherwise.
 *   by using this function during the lexing and parsing processes, we can ensure that
 *   reserved keywords are not mistakenly used as identifiers,
 *   which helps maintain the integrity of the language's syntax and semantics. */
/*
 * Function overview: keyword_type
 *
 * High-level purpose:
 * - This routine belongs to lexer.c.
 * - It exists to turn raw machine source text into a token stream that later parser stages can consume reliably.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "keyword type".
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
static TokenType keyword_type(const char *word)
{
    if (strcmp(word, "main") == 0)
        return TOKEN_MAIN;
    if (strcmp(word, "func") == 0)
        return TOKEN_FUNC;
    if (strcmp(word, "struct") == 0)
        return TOKEN_STRUCT;
    if (strcmp(word, "module") == 0)
        return TOKEN_MODULE;
    if (strcmp(word, "var") == 0)
        return TOKEN_VAR;
    if (strcmp(word, "const") == 0)
        return TOKEN_CONST;
    if (strcmp(word, "if") == 0)
        return TOKEN_IF;
    if (strcmp(word, "else") == 0)
        return TOKEN_ELSE;
    if (strcmp(word, "elif") == 0)
        return TOKEN_ELIF;
    if (strcmp(word, "while") == 0)
        return TOKEN_WHILE;
    if (strcmp(word, "print") == 0)
        return TOKEN_PRINT;
    if (strcmp(word, "ret") == 0)
        return TOKEN_RET;
    if (strcmp(word, "true") == 0)
        return TOKEN_TRUE;
    if (strcmp(word, "false") == 0)
        return TOKEN_FALSE;
    if (strcmp(word, "goto") == 0)
        return TOKEN_GOTO;
    if (strcmp(word, "label") == 0)
        return TOKEN_LABEL;
    if (strcmp(word, "switch") == 0)
        return TOKEN_SWITCH;
    if (strcmp(word, "case") == 0)
        return TOKEN_CASE;
    if (strcmp(word, "default") == 0)
        return TOKEN_DEFAULT;
    if (strcmp(word, "unsafe") == 0)
        return TOKEN_UNSAFE;
    return TOKEN_IDENTIFIER;
}

/* we implement a function to get the string representation of a token type,
 *   which is useful for error messages and debugging.
 *   this function takes a TokenType enum value as input and returns a
 *   string that represents the name of the token type.
 *   by using this function, we can provide more informative error messages that include the
 *   specific token type involved in an error, which helps developers understand and
 *   fix issues in their code more effectively. */
const char *token_type_name(TokenType type)
{
    switch (type)
    {
    case TOKEN_EOF:
        return "EOF";
    case TOKEN_NEWLINE:
        return "NEWLINE";
    case TOKEN_INDENT:
        return "INDENT";
    case TOKEN_DEDENT:
        return "DEDENT";
    case TOKEN_IDENTIFIER:
        return "identifier";
    case TOKEN_NUMBER:
        return "number";
    case TOKEN_STRING:
        return "string";
    case TOKEN_MAIN:
        return "main";
    case TOKEN_FUNC:
        return "func";
    case TOKEN_STRUCT:
        return "struct";
    case TOKEN_MODULE:
        return "module";
    case TOKEN_VAR:
        return "var";
    case TOKEN_CONST:
        return "const";
    case TOKEN_IF:
        return "if";
    case TOKEN_ELSE:
        return "else";
    case TOKEN_ELIF:
        return "elif";
    case TOKEN_WHILE:
        return "while";
    case TOKEN_PRINT:
        return "print";
    case TOKEN_RET:
        return "ret";
    case TOKEN_TRUE:
        return "true";
    case TOKEN_FALSE:
        return "false";
    case TOKEN_GOTO:
        return "goto";
    case TOKEN_LABEL:
        return "label";
    case TOKEN_SWITCH:
        return "switch";
    case TOKEN_CASE:
        return "case";
    case TOKEN_DEFAULT:
        return "default";
    case TOKEN_UNSAFE:
        return "unsafe";
    case TOKEN_ARROW:
        return "->";
    case TOKEN_LPAREN:
        return "(";
    case TOKEN_RPAREN:
        return ")";
    case TOKEN_LBRACKET:
        return "[";
    case TOKEN_RBRACKET:
        return "]";
    case TOKEN_COLON:
        return ":";
    case TOKEN_COMMA:
        return ",";
    case TOKEN_DOT:
        return ".";
    case TOKEN_EQUAL:
        return "=";
    case TOKEN_PLUS:
        return "+";
    case TOKEN_MINUS:
        return "-";
    case TOKEN_STAR:
        return "*";
    case TOKEN_SLASH:
        return "/";
    case TOKEN_AT:
        return "@";
    case TOKEN_CARET:
        return "^";
    case TOKEN_EQEQ:
        return "==";
    case TOKEN_NEQ:
        return "!=";
    case TOKEN_LT:
        return "<";
    case TOKEN_GT:
        return ">";
    case TOKEN_LE:
        return "<=";
    case TOKEN_GE:
        return ">=";
    default:
        return "unknown";
    }
}

/* we implement a function to scan a string literal from the source code,
 *   which is responsible for handling both regular string literals (enclosed in double quotes) and
 *
 * Function overview: literals
 *
 * High-level purpose:
 * - This routine belongs to lexer.c.
 * - It exists to turn raw machine source text into a token stream that later parser stages can consume reliably.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "literals".
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
/* multiline string literals (enclosed in triple double quotes).
 this function takes care of processing escape sequences within the string,
ensuring that the string is properly terminated, and adding the resulting string token to the list of tokens.
by centralizing string scanning in this function,
we can maintain consistent handling of string literals and
provide clear error messages when issues arise, such as unterminated strings or unsupported escape sequences. */
static bool scan_string(const SourceFile *src, const char **cursor, int *line, int *column, TokenList *tokens, DiagnosticList *errors)
{
    const char *start = *cursor;
    int start_line = *line;
    int start_col = *column;
    char out[256] = {0};
    size_t out_i = 0;

    if (start[0] == '"' && start[1] == '"' && start[2] == '"')
    {
        *cursor += 3;
        *column += 3;
        while (**cursor)
        {
            if ((*cursor)[0] == '"' && (*cursor)[1] == '"' && (*cursor)[2] == '"')
            {
                *cursor += 3;
                *column += 3;
                push(tokens, TOKEN_STRING, out, start_line, start_col);
                return true;
            }
            char ch = **cursor;
            if (out_i + 2 >= sizeof(out))
            {
                /* if the output buffer for the string literal is full,
                 *                   we report an error indicating that the string literal is too long and
                 *                   return false to indicate that scanning the string failed. */
                diagnostics_add(src, errors, start_line, start_col, "string literal is too long");
                return false;
            }
            if (ch == '\n')
            {
                out[out_i++] = '\\';
                out[out_i++] = 'n';
                ++(*cursor);
                ++(*line);
                *column = 1;
                continue;
            }
            if (ch == '"' || ch == '\\')
            {
                out[out_i++] = '\\';
            }
            out[out_i++] = ch;
            ++(*cursor);
            ++(*column);
        }
        diagnostics_add(src, errors, start_line, start_col, "unterminated multiline string literal");
        return false;
    }

    ++(*cursor);
    ++(*column);
    while (**cursor && **cursor != '"')
    {
        char ch = **cursor;
        if (ch == '\n')
        {
            diagnostics_add(src, errors, start_line, start_col, "unterminated string literal");
            return false;
        }
        if (out_i + 2 >= sizeof(out))
        {
            diagnostics_add(src, errors, start_line, start_col, "string literal is too long");
            return false;
        }
        if (ch == '\\')
        {
            ++(*cursor);
            ++(*column);
            char next = **cursor;
            if (next == 'n')
            {
                out[out_i++] = '\\';
                out[out_i++] = 'n';
            }
            else if (next == 't')
            {
                out[out_i++] = '\\';
                out[out_i++] = 't';
            }
            else if (next == '"' || next == '\\')
            {
                out[out_i++] = '\\';
                out[out_i++] = next;
            }
            else
            {
                diagnostics_add(src, errors, *line, *column, "unsupported escape sequence '\\%c'", next);
                /* if the escape sequence is not recognized, we report an error indicating
                 *                   that the escape sequence is unsupported and return false to indicate that scanning the string failed. */
                return false;
            }
            ++(*cursor);
            ++(*column);
            continue;
        }
        if (ch == '"')
        {
            out[out_i++] = '\\';
        }
        out[out_i++] = ch;
        ++(*cursor);
        ++(*column);
    }
    if (**cursor != '"')
    {
        diagnostics_add(src, errors, start_line, start_col, "unterminated string literal");
        return false;
    }
    ++(*cursor);
    ++(*column);
    push(tokens, TOKEN_STRING, out, start_line, start_col);
    return true;
}
/* we implement the main lexing function, which takes the source file as input and produces a
 *   list of tokens while also collecting any diagnostics (errors) that occur during the lexing process.
 *   this function processes the source code character by character,
 *   handling indentation levels, tokenizing identifiers, numbers, strings, and operators,
 *   and managing the state of the lexer to ensure that the resulting tokens accurately represent the structure of the source code.
 *   by centralizing the lexing logic in this function,
 *   we can maintain a clear and organized approach to converting raw
 *   source text into a structured format that can be used by the parser in subsequent stages of compilation. */

/*
 * Function overview: lex_source
 *
 * High-level purpose:
 * - This routine belongs to lexer.c.
 * - It exists to turn raw machine source text into a token stream that later parser stages can consume reliably.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "lex source".
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
bool lex_source(const SourceFile *src, TokenList *tokens, DiagnosticList *errors)
{
    tokens->count = 0;
    tokens->indent_width = 0;

    int indent_stack[128] = {0};
    int indent_top = 0;
    const char *cursor = src->source;
    int line = 1;

    while (*cursor)
    {
        int spaces = 0;
        int column = 1;
        while (*cursor == ' ')
        {
            ++spaces;
            ++cursor;
            ++column;
        }

        bool blank = (*cursor == '\n' || *cursor == '\0' || *cursor == '#');
        if (!blank)
        {
            if ((spaces % 2) != 0)
            {
                diagnostics_add(src, errors, line, 1, "indentation must use an even number of spaces; each block step may be 2 or 4 spaces");
                return false;
            }
            if (spaces > indent_stack[indent_top])
            {
                int delta = spaces - indent_stack[indent_top];
                if (delta != 2 && delta != 4)
                {
                    diagnostics_add(src, errors, line, 1, "indentation can only increase by 2 or 4 spaces per block");
                    return false;
                }
                ++indent_top;
                indent_stack[indent_top] = spaces;
                push(tokens, TOKEN_INDENT, "", line, 1);
            }
            else if (spaces < indent_stack[indent_top])
            {
                while (indent_top > 0 && spaces < indent_stack[indent_top])
                {
                    --indent_top;
                    push(tokens, TOKEN_DEDENT, "", line, 1);
                }
                if (spaces != indent_stack[indent_top])
                {
                    diagnostics_add(src, errors, line, 1, "inconsistent indentation level");
                    return false;
                }
            }
        }

        /* we process each line of the source code, counting leading spaces to determine indentation levels,
         *           and then we tokenize the content of the line while handling comments and blank lines appropriately.
         *           each new block may increase indentation by either 2 or 4 spaces, which lets projects mix both styles
         *           across the same source file while keeping block boundaries unambiguous.
         *           after handling indentation, we proceed to tokenize the line's content,
         *           recognizing identifiers, numbers, strings, operators, and other syntax
         *           elements while also managing line and column information for accurate error reporting. */
        while (*cursor && *cursor != '\n')
        {
            if (*cursor == '#')
            {
                break;
            }
            if (*cursor == ' ' || *cursor == '\t' || *cursor == '\r')
            {
                ++cursor;
                ++column;
                continue;
            }
            if (isalpha((unsigned char)*cursor) || *cursor == '_')
            {
                char word[256] = {0};
                int start_col = column;
                int i = 0;
                while (isalnum((unsigned char)*cursor) || *cursor == '_')
                {
                    if (i < (int)sizeof(word) - 1)
                    {
                        word[i++] = *cursor;
                    }
                    ++cursor;
                    ++column;
                }
                push(tokens, keyword_type(word), word, line, start_col);
                continue;
            }
            if (isdigit((unsigned char)*cursor))
            {
                char num[256] = {0};
                int start_col = column;
                int i = 0;
                int dots = 0;
                while (isdigit((unsigned char)*cursor) || *cursor == '.')
                {
                    if (*cursor == '.')
                    {
                        ++dots;
                    }
                    if (i < (int)sizeof(num) - 1)
                    {
                        num[i++] = *cursor;
                    }
                    ++cursor;
                    ++column;
                }
                if (dots > 1)
                {
                    diagnostics_add(src, errors, line, start_col, "invalid numeric literal '%s'", num);
                    return false;
                }
                push(tokens, TOKEN_NUMBER, num, line, start_col);
                continue;
            }
            if (*cursor == '"')
            {
                if (!scan_string(src, &cursor, &line, &column, tokens, errors))
                {
                    return false;
                }
                continue;
            }
            {
                int start_col = column;
                char lexeme[3] = {0};
                lexeme[0] = *cursor;
                lexeme[1] = '\0';
                if (*cursor == '-' && cursor[1] == '>')
                {
                    push(tokens, TOKEN_ARROW, "->", line, start_col);
                    cursor += 2;
                    column += 2;
                    continue;
                }
                if (*cursor == '=' && cursor[1] == '=')
                {
                    push(tokens, TOKEN_EQEQ, "==", line, start_col);
                    cursor += 2;
                    column += 2;
                    continue;
                }
                if (*cursor == '!' && cursor[1] == '=')
                {
                    push(tokens, TOKEN_NEQ, "!=", line, start_col);
                    cursor += 2;
                    column += 2;
                    continue;
                }
                if (*cursor == '<' && cursor[1] == '=')
                {
                    push(tokens, TOKEN_LE, "<=", line, start_col);
                    cursor += 2;
                    column += 2;
                    continue;
                }
                if (*cursor == '>' && cursor[1] == '=')
                {
                    push(tokens, TOKEN_GE, ">=", line, start_col);
                    cursor += 2;
                    column += 2;
                    continue;
                }
                switch (*cursor)
                {
                case '(':
                    push(tokens, TOKEN_LPAREN, lexeme, line, start_col);
                    break;
                case ')':
                    push(tokens, TOKEN_RPAREN, lexeme, line, start_col);
                    break;
                case '[':
                    push(tokens, TOKEN_LBRACKET, lexeme, line, start_col);
                    break;
                case ']':
                    push(tokens, TOKEN_RBRACKET, lexeme, line, start_col);
                    break;
                case ':':
                    push(tokens, TOKEN_COLON, lexeme, line, start_col);
                    break;
                case ',':
                    push(tokens, TOKEN_COMMA, lexeme, line, start_col);
                    break;
                case '.':
                    push(tokens, TOKEN_DOT, lexeme, line, start_col);
                    break;
                case '=':
                    push(tokens, TOKEN_EQUAL, lexeme, line, start_col);
                    break;
                case '+':
                    push(tokens, TOKEN_PLUS, lexeme, line, start_col);
                    break;
                case '-':
                    push(tokens, TOKEN_MINUS, lexeme, line, start_col);
                    break;
                case '*':
                    push(tokens, TOKEN_STAR, lexeme, line, start_col);
                    break;
                case '/':
                    push(tokens, TOKEN_SLASH, lexeme, line, start_col);
                    break;
                case '@':
                    push(tokens, TOKEN_AT, lexeme, line, start_col);
                    break;
                case '^':
                    push(tokens, TOKEN_CARET, lexeme, line, start_col);
                    break;
                case '<':
                    push(tokens, TOKEN_LT, lexeme, line, start_col);
                    break;
                case '>':
                    push(tokens, TOKEN_GT, lexeme, line, start_col);
                    break;
                default:
                    diagnostics_add(src, errors, line, start_col, "unexpected character '%c'", *cursor);
                    return false;
                }
                ++cursor;
                ++column;
            }
        }
        if (*cursor == '\n')
        {
            push(tokens, TOKEN_NEWLINE, "", line, column);
            ++cursor;
            ++line;
        }
    }

    while (indent_top > 0)
    {
        --indent_top;
        push(tokens, TOKEN_DEDENT, "", line, 1);
    }
    push(tokens, TOKEN_EOF, "", line, 1);
    return errors->count == 0;
}
