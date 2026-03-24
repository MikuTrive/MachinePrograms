#ifndef MACHINE_LEXER_H
#define MACHINE_LEXER_H

#include "common.h"

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

typedef struct
{
    TokenType type;
    char lexeme[256];
    int line;
    int column;
} Token;

typedef struct
{
    Token items[MACHINE_MAX_TOKENS];
    size_t count;
    int indent_width;
} TokenList;

bool lex_source(const SourceFile *source, TokenList *tokens, DiagnosticList *errors);
const char *token_type_name(TokenType type);

#endif
