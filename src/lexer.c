/*
 * Machine lexer.
 *
 * Responsibilities:
 *  1. Convert preprocessed source text into tokens.
 *  2. Enforce the project's indentation rule (2 or 4 spaces only).
 *  3. Assume Machine '--' comments were already stripped by preprocess_machine_source().
 */

#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void push(TokenList *tokens, TokenType type, const char *lexeme, int line, int column) {
    /* we implement a function to add a new token to the list of tokens during the lexing process.
       this function takes care of creating a new token with the 
       specified type, lexeme, line number, and column number, and it adds the token to the token list 
       while ensuring that we do not exceed the maximum number of tokens allowed.
       by centralizing token creation in this function, 
       we can maintain a consistent approach to token management and 
       handle any potential errors related to token limits in one place. */  
    if (tokens->count >= MACHINE_MAX_TOKENS) {
        return;
    }
    Token *t = &tokens->items[tokens->count++];
    t->type = type;
    snprintf(t->lexeme, sizeof(t->lexeme), "%s", lexeme ? lexeme : "");
    t->line = line;
    t->column = column;
}

/* we implement a function to determine if a given word is a reserved keyword in our programming language, 
   which cannot be used as an identifier for variables, functions, or other user-defined names.
   this function checks the input word against a list of reserved keywords and 
   returns true if the word is reserved, and false otherwise.
   by using this function during the lexing and parsing processes, we can ensure that 
   reserved keywords are not mistakenly used as identifiers, 
   which helps maintain the integrity of the language's syntax and semantics. */
static TokenType keyword_type(const char *word) {
    if (strcmp(word, "main") == 0) return TOKEN_MAIN;
    if (strcmp(word, "func") == 0) return TOKEN_FUNC;
    if (strcmp(word, "struct") == 0) return TOKEN_STRUCT;
    if (strcmp(word, "module") == 0) return TOKEN_MODULE;
    if (strcmp(word, "var") == 0) return TOKEN_VAR;
    if (strcmp(word, "const") == 0) return TOKEN_CONST;
    if (strcmp(word, "if") == 0) return TOKEN_IF;
    if (strcmp(word, "else") == 0) return TOKEN_ELSE;
    if (strcmp(word, "elif") == 0) return TOKEN_ELIF;
    if (strcmp(word, "while") == 0) return TOKEN_WHILE;
    if (strcmp(word, "print") == 0) return TOKEN_PRINT;
    if (strcmp(word, "ret") == 0) return TOKEN_RET;
    if (strcmp(word, "true") == 0) return TOKEN_TRUE;
    if (strcmp(word, "false") == 0) return TOKEN_FALSE;
    if (strcmp(word, "goto") == 0) return TOKEN_GOTO;
    if (strcmp(word, "label") == 0) return TOKEN_LABEL;
    if (strcmp(word, "switch") == 0) return TOKEN_SWITCH;
    if (strcmp(word, "case") == 0) return TOKEN_CASE;
    if (strcmp(word, "default") == 0) return TOKEN_DEFAULT;
    return TOKEN_IDENTIFIER;
}

/* we implement a function to get the string representation of a token type, 
   which is useful for error messages and debugging.
   this function takes a TokenType enum value as input and returns a 
   string that represents the name of the token type.
   by using this function, we can provide more informative error messages that include the 
   specific token type involved in an error, which helps developers understand and 
   fix issues in their code more effectively. */
const char *token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_NEWLINE: return "NEWLINE";
        case TOKEN_INDENT: return "INDENT";
        case TOKEN_DEDENT: return "DEDENT";
        case TOKEN_IDENTIFIER: return "identifier";
        case TOKEN_NUMBER: return "number";
        case TOKEN_STRING: return "string";
        case TOKEN_MAIN: return "main";
        case TOKEN_FUNC: return "func";
        case TOKEN_STRUCT: return "struct";
        case TOKEN_MODULE: return "module";
        case TOKEN_VAR: return "var";
        case TOKEN_CONST: return "const";
        case TOKEN_IF: return "if";
        case TOKEN_ELSE: return "else";
        case TOKEN_ELIF: return "elif";
        case TOKEN_WHILE: return "while";
        case TOKEN_PRINT: return "print";
        case TOKEN_RET: return "ret";
        case TOKEN_TRUE: return "true";
        case TOKEN_FALSE: return "false";
        case TOKEN_GOTO: return "goto";
        case TOKEN_LABEL: return "label";
        case TOKEN_SWITCH: return "switch";
        case TOKEN_CASE: return "case";
        case TOKEN_DEFAULT: return "default";
        case TOKEN_ARROW: return "->";
        case TOKEN_LPAREN: return "(";
        case TOKEN_RPAREN: return ")";
        case TOKEN_LBRACKET: return "[";
        case TOKEN_RBRACKET: return "]";
        case TOKEN_COLON: return ":";
        case TOKEN_COMMA: return ",";
        case TOKEN_DOT: return ".";
        case TOKEN_EQUAL: return "=";
        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_STAR: return "*";
        case TOKEN_SLASH: return "/";
        case TOKEN_AT: return "@";
        case TOKEN_CARET: return "^";
        case TOKEN_EQEQ: return "==";
        case TOKEN_NEQ: return "!=";
        case TOKEN_LT: return "<";
        case TOKEN_GT: return ">";
        case TOKEN_LE: return "<=";
        case TOKEN_GE: return ">=";
        default: return "unknown";
    }
}

/* we implement a function to scan a string literal from the source code, 
   which is responsible for handling both regular string literals (enclosed in double quotes) and 
   multiline string literals (enclosed in triple double quotes).
   this function takes care of processing escape sequences within the string, 
   ensuring that the string is properly terminated, and adding the resulting string token to the list of tokens.
   by centralizing string scanning in this function, 
   we can maintain consistent handling of string literals and 
   provide clear error messages when issues arise, such as unterminated strings or unsupported escape sequences. */
static bool scan_string(const SourceFile *src, const char **cursor, int *line, int *column, TokenList *tokens, DiagnosticList *errors) {
    const char *start = *cursor;
    int start_line = *line;
    int start_col = *column;
    char out[256] = {0};
    size_t out_i = 0;

    if (start[0] == '"' && start[1] == '"' && start[2] == '"') {
        *cursor += 3;
        *column += 3;
        while (**cursor) {
            if ((*cursor)[0] == '"' && (*cursor)[1] == '"' && (*cursor)[2] == '"') {
                *cursor += 3;
                *column += 3;
                push(tokens, TOKEN_STRING, out, start_line, start_col);
                return true;
            }
            char ch = **cursor;
            if (out_i + 2 >= sizeof(out)) {
                /* if the output buffer for the string literal is full, 
                   we report an error indicating that the string literal is too long and 
                   return false to indicate that scanning the string failed. */
                diagnostics_add(src, errors, start_line, start_col, "string literal is too long");
                return false;
            }
            if (ch == '\n') {
                out[out_i++] = '\\';
                out[out_i++] = 'n';
                ++(*cursor);
                ++(*line);
                *column = 1;
                continue;
            }
            if (ch == '"' || ch == '\\') {
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
    while (**cursor && **cursor != '"') {
        char ch = **cursor;
        if (ch == '\n') {
            diagnostics_add(src, errors, start_line, start_col, "unterminated string literal");
            return false;
        }
        if (out_i + 2 >= sizeof(out)) {
            diagnostics_add(src, errors, start_line, start_col, "string literal is too long");
            return false;
        }
        if (ch == '\\') {
            ++(*cursor);
            ++(*column);
            char next = **cursor;
            if (next == 'n') {
                out[out_i++] = '\\';
                out[out_i++] = 'n';
            } else if (next == 't') {
                out[out_i++] = '\\';
                out[out_i++] = 't';
            } else if (next == '"' || next == '\\') {
                out[out_i++] = '\\';
                out[out_i++] = next;
            } else {
                diagnostics_add(src, errors, *line, *column, "unsupported escape sequence '\\%c'", next);
                /* if the escape sequence is not recognized, we report an error indicating 
                   that the escape sequence is unsupported and return false to indicate that scanning the string failed. */
                return false;
            }
            ++(*cursor);
            ++(*column);
            continue;
        }
        if (ch == '"') {
            out[out_i++] = '\\';
        }
        out[out_i++] = ch;
        ++(*cursor);
        ++(*column);
    }
    if (**cursor != '"') {
        diagnostics_add(src, errors, start_line, start_col, "unterminated string literal");
        return false;
    }
    ++(*cursor);
    ++(*column);
    push(tokens, TOKEN_STRING, out, start_line, start_col);
    return true;
}
/* we implement the main lexing function, which takes the source file as input and produces a 
   list of tokens while also collecting any diagnostics (errors) that occur during the lexing process.
   this function processes the source code character by character, 
   handling indentation levels, tokenizing identifiers, numbers, strings, and operators, 
   and managing the state of the lexer to ensure that the resulting tokens accurately represent the structure of the source code.
   by centralizing the lexing logic in this function, 
   we can maintain a clear and organized approach to converting raw 
   source text into a structured format that can be used by the parser in subsequent stages of compilation. */

bool lex_source(const SourceFile *src, TokenList *tokens, DiagnosticList *errors) {
    tokens->count = 0;
    tokens->indent_width = 0;

    int indent_stack[128] = {0};
    int indent_top = 0;
    const char *cursor = src->source;
    int line = 1;

    while (*cursor) {
        int spaces = 0;
        int column = 1;
        while (*cursor == ' ') {
            ++spaces;
            ++cursor;
            ++column;
        }

        bool blank = (*cursor == '\n' || *cursor == '\0' || *cursor == '#');
        if (!blank) {
            if (tokens->indent_width == 0 && spaces > 0) {
                if (spaces == 2 || spaces == 4) {
                    tokens->indent_width = spaces;
                } else {
                    diagnostics_add(src, errors, line, 1, "indentation must start with exactly 2 or 4 spaces");
                    return false;
                }
            }
            if (tokens->indent_width == 0) {
                tokens->indent_width = 2;
            }
            if (spaces % tokens->indent_width != 0) {
                diagnostics_add(src, errors, line, 1, "indentation must use exactly %d spaces per level", tokens->indent_width);
                return false;
            }
            int level = spaces / tokens->indent_width;
            if (level > indent_stack[indent_top]) {
                while (level > indent_stack[indent_top]) {
                    ++indent_top;
                    indent_stack[indent_top] = indent_stack[indent_top - 1] + 1;
                    push(tokens, TOKEN_INDENT, "", line, 1);
                }
            } else if (level < indent_stack[indent_top]) {
                while (indent_top > 0 && level < indent_stack[indent_top]) {
                    --indent_top;
                    push(tokens, TOKEN_DEDENT, "", line, 1);
                }
                if (level != indent_stack[indent_top]) {
                    diagnostics_add(src, errors, line, 1, "inconsistent indentation level");
                    return false;
                }
            }
        }

        /* we process each line of the source code, counting leading spaces to determine indentation levels, 
           and then we tokenize the content of the line while handling comments and blank lines appropriately.
           for non-blank lines, we check the indentation against the expected indentation width and manage the 
           indent stack to generate INDENT and DEDENT tokens as needed.
           after handling indentation, we proceed to tokenize the line's content, 
           recognizing identifiers, numbers, strings, operators, and other syntax 
           elements while also managing line and column information for accurate error reporting. */
        while (*cursor && *cursor != '\n') {
            if (*cursor == '#') {
                break;
            }
            if (*cursor == ' ' || *cursor == '\t' || *cursor == '\r') {
                ++cursor;
                ++column;
                continue;
            }
            if (isalpha((unsigned char)*cursor) || *cursor == '_') {
                char word[256] = {0};
                int start_col = column;
                int i = 0;
                while (isalnum((unsigned char)*cursor) || *cursor == '_') {
                    if (i < (int)sizeof(word) - 1) {
                        word[i++] = *cursor;
                    }
                    ++cursor;
                    ++column;
                }
                push(tokens, keyword_type(word), word, line, start_col);
                continue;
            }
            if (isdigit((unsigned char)*cursor)) {
                char num[256] = {0};
                int start_col = column;
                int i = 0;
                int dots = 0;
                while (isdigit((unsigned char)*cursor) || *cursor == '.') {
                    if (*cursor == '.') {
                        ++dots;
                    }
                    if (i < (int)sizeof(num) - 1) {
                        num[i++] = *cursor;
                    }
                    ++cursor;
                    ++column;
                }
                if (dots > 1) {
                    diagnostics_add(src, errors, line, start_col, "malformed numeric literal '%s'", num);
                    return false;
                }
                push(tokens, TOKEN_NUMBER, num, line, start_col);
                continue;
            }
            if (*cursor == '"') {
                if (!scan_string(src, &cursor, &line, &column, tokens, errors)) {
                    return false;
                }
                /* if scanning the string literal fails, we return false to indicate that lexing failed, 
                   and any errors will have been added to the diagnostics list for reporting. */
                continue;
            }

            int start_col = column;
            if (cursor[0] == '-' && cursor[1] == '>') { push(tokens, TOKEN_ARROW, "->", line, start_col); cursor += 2; column += 2; continue; }
            if (cursor[0] == '=' && cursor[1] == '=') { push(tokens, TOKEN_EQEQ, "==", line, start_col); cursor += 2; column += 2; continue; }
            if (cursor[0] == '!' && cursor[1] == '=') { push(tokens, TOKEN_NEQ, "!=", line, start_col); cursor += 2; column += 2; continue; }
            if (cursor[0] == '<' && cursor[1] == '=') { push(tokens, TOKEN_LE, "<=", line, start_col); cursor += 2; column += 2; continue; }
            if (cursor[0] == '>' && cursor[1] == '=') { push(tokens, TOKEN_GE, ">=", line, start_col); cursor += 2; column += 2; continue; }

            TokenType type;
            char txt[2] = { *cursor, '\0' };
            switch (*cursor) {
                case '(': type = TOKEN_LPAREN; break;
                case ')': type = TOKEN_RPAREN; break;
                case '[': type = TOKEN_LBRACKET; break;
                case ']': type = TOKEN_RBRACKET; break;
                case ':': type = TOKEN_COLON; break;
                case ',': type = TOKEN_COMMA; break;
                case '.': type = TOKEN_DOT; break;
                case '=': type = TOKEN_EQUAL; break;
                case '+': type = TOKEN_PLUS; break;
                case '-': type = TOKEN_MINUS; break;
                case '*': type = TOKEN_STAR; break;
                case '/': type = TOKEN_SLASH; break;
                case '@': type = TOKEN_AT; break;
                case '^': type = TOKEN_CARET; break;
                case '<': type = TOKEN_LT; break;
                case '>': type = TOKEN_GT; break;
                default:
                    diagnostics_add(src, errors, line, column, "unexpected character '%c'", *cursor);
                    return false;
            }
            /* we handle single-character tokens and two-character operators, 
               adding them to the token list with the appropriate type and lexeme, 
               while also managing the cursor and column position for accurate tokenization. */
            push(tokens, type, txt, line, start_col);
            ++cursor;
            ++column;
        }

        push(tokens, TOKEN_NEWLINE, "", line, column);
        while (*cursor && *cursor != '\n') ++cursor;
        if (*cursor == '\n') ++cursor;
        ++line;
    }

    while (indent_top > 0) {
        --indent_top;
        push(tokens, TOKEN_DEDENT, "", line, 1);
    }
    push(tokens, TOKEN_EOF, "", line, 1);
    return errors->count == 0;
}
