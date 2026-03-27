// ============================================================================
// Annotated reading edition of parser.h
// ----------------------------------------------------------------------------
// Important note:
// - The original code has NOT been changed.
// - This file only adds English comments around the existing declarations.
// - To keep the source safe and compilable in a reading context, line comments
//   are used instead of nested block comments.
// - The goal of this header is to define the core abstract syntax tree (AST)
//   model used by the Machine compiler front end.
// ============================================================================

#ifndef MACHINE_PARSER_H
#define MACHINE_PARSER_H

// This header depends on the project's shared compiler definitions and on the
// token/lexer interface. In practice:
//
// - common.h provides shared enums, constants, helper types, diagnostics, and
//   language-wide definitions such as MachineType and capacity limits.
// - lexer.h provides the token stream data that the parser consumes.
//
// The parser sits between the lexer and later compiler phases:
// source text -> lexer -> token stream -> parser -> AST -> semantic/codegen
#include "common.h"
#include "lexer.h"

// ----------------------------------------------------------------------------
// TypeRef
// ----------------------------------------------------------------------------
// TypeRef is the compiler's compact representation of a type as it appears in
// source code or as it is inferred during semantic analysis.
//
// It carries:
//
// - kind:
//   The base language type (for example i64, f64, bool, str, ptr, struct, etc).
//
// - struct_name:
//   When the type is a struct type, this stores the user-defined struct name.
//
// - array_depth:
//   Tracks how many array layers are wrapped around the base type.
//   For example, a one-dimensional array has depth 1, a two-dimensional array
//   has depth 2, and so on.
//
// This structure is reused all over the AST because nearly every major node
// eventually needs type information.
typedef struct
{
    /* we define a set of types and data structures to represent the
     *       abstract syntax tree (AST) of our programming language,
     *       including expressions, statements, function declarations,
     *       struct declarations, and global variable declarations.
     *       these structures allow us to capture the semantics of our
     *       programming language in a structured way,
     *       enabling us to perform semantic analysis and
     *       code generation based on this representation. */
    MachineType kind;
    char struct_name[64];
    int array_depth;
} TypeRef;

// ----------------------------------------------------------------------------
// ExprKind
// ----------------------------------------------------------------------------
// ExprKind classifies every expression node that can appear in the AST.
//
// This enum is the parser's "tag" for the Expr union below. Once kind is set,
// later compiler stages know which union member is valid.
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
 *   including integer literals, floating-point literals,
 *   string literals, boolean literals, identifiers,
 *   unary operations, binary operations, function calls,
 *   indexing expressions, array literals, and field access expressions.
 *   this enumeration allows us to distinguish between different types of expressions when
 *   constructing the abstract syntax tree (AST) and performing semantic analysis and
 *   code generation based on the structure of the program. */

// Forward declaration so recursive pointer members can refer to Expr before the
// full struct layout is defined.
typedef struct Expr Expr;

// ----------------------------------------------------------------------------
// Expr
// ----------------------------------------------------------------------------
// Expr is the central expression node in the AST.
//
// Layout overview:
//
// - kind:
//   Tells us which expression shape this node represents.
//
// - inferred_type:
//   Stores semantic type information once the compiler has inferred or checked
//   the expression's type.
//
// - line / column:
//   Source position used for diagnostics and error reporting.
//
// - struct_name:
//   Extra storage used by some expression cases that need to preserve a struct
//   identity alongside general type information.
//
// - union as:
//   Stores the payload for the specific expression kind. Only the member that
//   corresponds to "kind" is considered valid.
//
// This pattern (tag + union payload) is standard in compilers because it keeps
// the AST compact while still allowing many node forms.
struct Expr
{
    ExprKind kind;
    /* we define a structure to represent an expression in our programming language,
     *       which includes the kind of expression,
     *       the inferred type of the expression, the source code location (line and column),
     *       and a union to hold the specific data for each kind of expression.
     *       this structure allows us to capture the details of each expression in a way that can be
     *       easily analyzed and transformed during semantic analysis and code generation. */
    TypeRef inferred_type;
    int line;
    int column;
    char struct_name[64];
    union
    {
        // Integer literal payload.
        long long int_value;

        // Floating-point literal payload.
        double float_value;

        // Boolean literal payload, commonly stored as 0/1.
        int bool_value;

        // Text storage for string literals and identifiers.
        char text[256];

        // Unary expression:
        //   op operand
        // Examples:
        //   -x
        //   !flag
        //   @x
        //   ^ptr
        struct
        {
            char op[3];
            Expr *operand;
        } unary;

        // Binary expression:
        //   left op right
        // Examples:
        //   a + b
        //   x == y
        //   ptr > 0
        struct
        {
            char op[3];
            Expr *left;
            Expr *right;
        } binary;

        // Function call:
        //   callee(arg1, arg2, ...)
        struct
        {
            Expr *callee;
            Expr *args[32];
            size_t arg_count;
        } call;

        // Indexing:
        //   base[index]
        // Used for arrays, lists, and any indexable abstraction supported by
        // the language.
        struct
        {
            Expr *base;
            Expr *index;
        } index;

        // Array literal:
        //   [a, b, c]
        struct
        {
            Expr *items[128];
            size_t item_count;
        } array;

        // Field access:
        //   base.field
        struct
        {
            Expr *base;
            char field[64];
        } field;
    } as;
};

// ----------------------------------------------------------------------------
// StmtKind
// ----------------------------------------------------------------------------
// StmtKind classifies every statement node in the AST.
//
// Like ExprKind, this is the discriminator for the Statement union.
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
    STMT_SWITCH,
    STMT_UNSAFE
} StmtKind;

// Forward declaration for recursive statement references.
typedef struct Statement Statement;

/* we define an enumeration for the different kinds of statements in our programming language,
 *   including variable declarations, constant declarations,
 *   assignment statements, print statements,
 *   return statements, if statements, while loops,
 *   expression statements, labels, goto statements, and switch statements.
 *   this enumeration allows us to distinguish between
 *   different types of statements when constructing the
 *   abstract syntax tree (AST) and performing semantic analysis and
 *   code generation based on the structure of the program. */

// ----------------------------------------------------------------------------
// Statement payload structures
// ----------------------------------------------------------------------------
// The compiler splits statement payloads into smaller named structs so the
// main Statement union is easier to read and maintain.

// Variable or constant declaration statement.
//
// Fields:
// - name: declared identifier.
// - declared_type: explicit or resolved type.
// - initializer: optional expression on the right-hand side.
// - has_initializer: whether initializer is present.
// - is_const: whether this declaration is const-like.
// - line: source line for diagnostics.
typedef struct
{
    char name[64];
    TypeRef declared_type;
    Expr *initializer;
    int has_initializer;
    int is_const;
    int line;
} VarStmt;

// Assignment statement:
//   target = value
typedef struct
{
    Expr *target;
    Expr *value;
    int line;
} AssignStmt;

// Print statement:
//   print value
typedef struct
{
    Expr *value;
    int line;
} PrintStmt;

// Return statement:
//   ret value
// Depending on language rules, value may be NULL/omitted for void-like returns.
typedef struct
{
    Expr *value;
    int line;
} ReturnStmt;

// If statement with optional else block.
// then_block / else_block point to statement arrays allocated elsewhere.
typedef struct
{
    Expr *condition;
    Statement *then_block;
    size_t then_count;
    Statement *else_block;
    size_t else_count;
    int line;
} IfStmt;

// While loop statement.
typedef struct
{
    Expr *condition;
    Statement *body;
    size_t body_count;
    int line;
} WhileStmt;

// Expression used as a statement, for example a function call whose result is
// ignored.
typedef struct
{
    Expr *expr;
    int line;
} ExprStmt;

// Label statement used for goto-style control flow.
typedef struct
{
    char name[64];
    int line;
} LabelStmt;

// Goto statement that jumps to a named label.
typedef struct
{
    char name[64];
    int line;
} GotoStmt;

// One case inside a switch statement.
//
// - is_default marks the default branch.
// - match is the case expression when not default.
// - body/body_count describe the block belonging to this case.
typedef struct
{
    int is_default;
    Expr *match;
    Statement *body;
    size_t body_count;
    int line;
} SwitchCase;

// Full switch statement.
typedef struct
{
    Expr *value;
    SwitchCase *cases;
    size_t case_count;
    int line;
} SwitchStmt;

// Unsafe statement block.
//
// This lets the parser represent:
//
// unsafe:
//   ...
//
// as a dedicated AST node whose body is another statement block.
typedef struct
{
    Statement *body;
    size_t body_count;
    int line;
} UnsafeStmt;

/* we define a structure to represent a statement in our programming language,
 *   which includes the kind of statement and a union to hold the specific data for each kind of statement.
 *   this structure allows us to capture the details of each statement in a way that can be
 *   easily analyzed and transformed during semantic analysis and code generation. */

// ----------------------------------------------------------------------------
// Statement
// ----------------------------------------------------------------------------
// Statement is the statement-level equivalent of Expr:
//
// - kind tells us which statement form this node holds.
// - union as stores the concrete payload for that statement form.
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
        UnsafeStmt unsafe_stmt;
    } as;
};

// ----------------------------------------------------------------------------
// Supporting declaration structures
// ----------------------------------------------------------------------------

// Function parameter descriptor.
typedef struct
{
    char name[64];
    TypeRef type;
} Param;

// One field inside a struct declaration.
typedef struct
{
    char name[64];
    TypeRef type;
    int line;
} StructField;

// Complete struct declaration with a fixed-capacity field table.
typedef struct
{
    char name[64];
    StructField fields[MACHINE_MAX_FIELDS];
    size_t field_count;
    int line;
} StructDecl;

// Module declaration metadata.
typedef struct
{
    char name[64];
    int line;
} ModuleDecl;

// Global variable declaration.
//
// The "used" flag is commonly helpful during semantic passes and diagnostics,
// such as dead-code or unused-global checks.
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

// Function declaration.
//
// This is one of the most important top-level AST nodes because it ties
// together function identity, parameter types, return type, and the body block.
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

// Symbol table entry.
//
// This compact structure lets the compiler reason about identifiers in scope
// without always needing a full AST node.
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
 *   which includes lists of struct declarations, module declarations, global variable declarations, function declarations,
 *   and pools for expressions and statements.
 *   this structure serves as the root of our abstract syntax tree (AST) and allows us to capture the
 *   complete structure of the program for semantic analysis and code generation. */

// ----------------------------------------------------------------------------
// Program
// ----------------------------------------------------------------------------
// Program is the root object for the parser output.
//
// It collects all top-level declarations plus the memory pools that own AST
// nodes allocated during parsing.
//
// Important fields:
//
// - structs / modules / globals / functions:
//   Top-level declarations discovered in the source file.
//
// - expr_pool:
//   Storage for all heap-allocated expression nodes so they can be freed
//   centrally later.
//
// - stmt_blocks / switch_case_blocks:
//   Tracking arrays for separately allocated statement and switch-case blocks.
//
// - allow_unsafe:
//   Whether unsafe features are permitted for this parse.
//
// - target_id / backend_id:
//   Build-selection metadata propagated from preprocessing / CLI options so
//   later compiler stages know which codegen target and backend were selected.
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
    int allow_unsafe;
    int target_id;
    int backend_id;
} Program;

// ----------------------------------------------------------------------------
// Public parser API
// ----------------------------------------------------------------------------
//
// parse_program(...)
// ------------------
// Consumes a source file descriptor and a token list, then fills Program with
// the parsed AST and reports diagnostics through the provided error/warning
// lists. The allow_unsafe flag controls whether unsafe constructs may appear.
//
// free_program(...)
// -----------------
// Releases memory and block allocations tracked by Program.
bool parse_program(const SourceFile *src,
                   const TokenList *tokens,
                   Program *program,
                   DiagnosticList *errors,
                   DiagnosticList *warnings,
                   int allow_unsafe);
void free_program(Program *program);

#endif
