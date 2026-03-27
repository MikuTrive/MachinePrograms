/*
 * Annotated reading edition of lexer.h
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
#ifndef MACHINE_LEXER_H
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_LEXER_H

/*
 * Dependency include.
 *
 * This brings in declarations required by the current header.
 */
#include "common.h"

/*
 * Enumeration declaration.
 *
 * Enums usually define token kinds, AST node categories, type tags, or other fixed symbolic values.
 */
typedef enum
{
    TOKEN_EOF,
    TOKEN_NEWLINE,
    TOKEN_INDENT,
    TOKEN_DEDENT,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_MAIN,
    TOKEN_FUNC,
    TOKEN_STRUCT,
    TOKEN_MODULE,
    TOKEN_VAR,
    TOKEN_CONST,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_ELIF,
    TOKEN_WHILE,
    TOKEN_PRINT,
    TOKEN_RET,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_GOTO,
    TOKEN_LABEL,
    TOKEN_SWITCH,
    TOKEN_CASE,
    TOKEN_DEFAULT,
    TOKEN_UNSAFE,
    TOKEN_ARROW,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_EQUAL,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_AT,
    TOKEN_CARET,
    TOKEN_EQEQ,
    TOKEN_NEQ,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LE,
    TOKEN_GE
} TokenType;

/*
 * Structure declaration.
 *
 * Structures in this project carry parser state, token records, AST nodes, type information, or runtime-facing data.
 */
typedef struct
{
    TokenType type;
    char lexeme[256];
    int line;
    int column;
} Token;

/*
 * Structure declaration.
 *
 * Structures in this project carry parser state, token records, AST nodes, type information, or runtime-facing data.
 */
typedef struct
{
    Token items[MACHINE_MAX_TOKENS];
    size_t count;
    int indent_width;
} TokenList;

/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
bool lex_source(const SourceFile *source, TokenList *tokens, DiagnosticList *errors);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
const char *token_type_name(TokenType type);

/*
 * Preprocessor directive.
 *
 * Directives here usually define compile-time constants, feature switches, or version identifiers.
 */
#endif
