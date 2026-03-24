#ifndef MACHINE_PARSER_H
#define MACHINE_PARSER_H

#include "common.h"
#include "lexer.h"

typedef struct
{
    /* we define a set of types and data structures to represent the 
       abstract syntax tree (AST) of our programming language,
       including expressions, statements, function declarations, 
       struct declarations, and global variable declarations.
       these structures allow us to capture the semantics of our 
       programming language in a structured way,
       enabling us to perform semantic analysis and 
       code generation based on this representation. */
    MachineType kind;
    char struct_name[64];
    int array_depth;
} TypeRef;

typedef enum
{
    EXPR_INT,
    EXPR_FLOAT,
    EXPR_STRING,
    EXPR_BOOL,
    EXPR_IDENTIFIER,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_CALL,
    EXPR_INDEX,
    EXPR_ARRAY,
    EXPR_FIELD
} ExprKind;
/* we define an enumeration for the different kinds of expressions in our programming language,
   including integer literals, floating-point literals, 
   string literals, boolean literals, identifiers,
   unary operations, binary operations, function calls, 
   indexing expressions, array literals, and field access expressions.
   this enumeration allows us to distinguish between different types of expressions when
   constructing the abstract syntax tree (AST) and performing semantic analysis and 
   code generation based on the structure of the program. */

typedef struct Expr Expr;
struct Expr
{
    ExprKind kind;
    /* we define a structure to represent an expression in our programming language, 
       which includes the kind of expression,
       the inferred type of the expression, the source code location (line and column), 
       and a union to hold the specific data for each kind of expression.
       this structure allows us to capture the details of each expression in a way that can be
       easily analyzed and transformed during semantic analysis and code generation. */
    TypeRef inferred_type;
    int line;
    int column;
    char struct_name[64];
    union
    {
        long long int_value;
        double float_value;
        int bool_value;
        char text[256];
        struct
        {
            char op[3];
            Expr *operand;
        } unary;
        struct
        {
            char op[3];
            Expr *left;
            Expr *right;
        } binary;
        struct
        {
            Expr *callee;
            Expr *args[32];
            size_t arg_count;
        } call;
        struct
        {
            Expr *base;
            Expr *index;
        } index;
        struct
        {
            Expr *items[128];
            size_t item_count;
        } array;
        struct
        {
            Expr *base;
            char field[64];
        } field;
    } as;
};

typedef enum
{
    STMT_VAR,
    STMT_CONST,
    STMT_ASSIGN,
    STMT_PRINT,
    STMT_RETURN,
    STMT_IF,
    STMT_WHILE,
    STMT_EXPR,
    STMT_LABEL,
    STMT_GOTO,
    STMT_SWITCH
} StmtKind;

typedef struct Statement Statement;

/* we define an enumeration for the different kinds of statements in our programming language,
   including variable declarations, constant declarations, 
   assignment statements, print statements,
   return statements, if statements, while loops, 
   expression statements, labels, goto statements, and switch statements.
   this enumeration allows us to distinguish between 
   different types of statements when constructing the
   abstract syntax tree (AST) and performing semantic analysis and 
   code generation based on the structure of the program. */
typedef struct
{
    char name[64];
    TypeRef declared_type;
    Expr *initializer;
    int has_initializer;
    int is_const;
    int line;
} VarStmt;
typedef struct
{
    Expr *target;
    Expr *value;
    int line;
} AssignStmt;
typedef struct
{
    Expr *value;
    int line;
} PrintStmt;
typedef struct
{
    Expr *value;
    int line;
} ReturnStmt;
typedef struct
{
    Expr *condition;
    Statement *then_block;
    size_t then_count;
    Statement *else_block;
    size_t else_count;
    int line;
} IfStmt;
typedef struct
{
    Expr *condition;
    Statement *body;
    size_t body_count;
    int line;
} WhileStmt;
typedef struct
{
    Expr *expr;
    int line;
} ExprStmt;
typedef struct
{
    char name[64];
    int line;
} LabelStmt;
typedef struct
{
    char name[64];
    int line;
} GotoStmt;

typedef struct
{
    int is_default;
    Expr *match;
    Statement *body;
    size_t body_count;
    int line;
} SwitchCase;

typedef struct
{
    Expr *value;
    SwitchCase *cases;
    size_t case_count;
    int line;
} SwitchStmt;

/* we define a structure to represent a statement in our programming language,
   which includes the kind of statement and a union to hold the specific data for each kind of statement.
   this structure allows us to capture the details of each statement in a way that can be
   easily analyzed and transformed during semantic analysis and code generation. */
struct Statement
{
    StmtKind kind;
    union
    {
        VarStmt var_stmt;
        AssignStmt assign_stmt;
        PrintStmt print_stmt;
        ReturnStmt return_stmt;
        IfStmt if_stmt;
        WhileStmt while_stmt;
        ExprStmt expr_stmt;
        LabelStmt label_stmt;
        GotoStmt goto_stmt;
        SwitchStmt switch_stmt;
    } as;
};

typedef struct
{
    char name[64];
    TypeRef type;
} Param;
typedef struct
{
    char name[64];
    TypeRef type;
    int line;
} StructField;
typedef struct
{
    char name[64];
    StructField fields[MACHINE_MAX_FIELDS];
    size_t field_count;
    int line;
} StructDecl;
typedef struct
{
    char name[64];
    int line;
} ModuleDecl;
typedef struct
{
    char name[64];
    TypeRef declared_type;
    Expr *initializer;
    int has_initializer;
    int is_const;
    int line;
    bool used;
} GlobalVarDecl;

typedef struct
{
    char name[64];
    char source_name[64];
    char module_name[64];
    Param params[MACHINE_MAX_PARAMS];
    size_t param_count;
    TypeRef return_type;
    Statement *body;
    size_t body_count;
    int line;
    bool is_main;
    bool used;
} FunctionDecl;

typedef struct
{
    char name[64];
    TypeRef type;
    int line;
    bool used;
    bool is_function;
    bool is_const;
} Symbol;

/* we define a structure to represent the entire program in our programming language,
   which includes lists of struct declarations, module declarations, global variable declarations, function declarations,
   and pools for expressions and statements.
   this structure serves as the root of our abstract syntax tree (AST) and allows us to capture the
   complete structure of the program for semantic analysis and code generation. */
typedef struct
{
    StructDecl structs[MACHINE_MAX_STRUCTS];
    size_t struct_count;
    ModuleDecl modules[MACHINE_MAX_STRUCTS];
    size_t module_count;
    GlobalVarDecl globals[MACHINE_MAX_SYMBOLS];
    size_t global_count;
    FunctionDecl functions[MACHINE_MAX_FUNCTIONS];
    size_t function_count;
    Expr *expr_pool[MACHINE_MAX_EXPR_POOL];
    size_t expr_pool_count;
    Statement *stmt_blocks[4096];
    size_t stmt_block_count;
    SwitchCase *switch_case_blocks[1024];
    size_t switch_case_block_count;
} Program;

bool parse_program(const SourceFile *src,
                   const TokenList *tokens,
                   Program *program,
                   DiagnosticList *errors,
                   DiagnosticList *warnings);
void free_program(Program *program);

#endif
