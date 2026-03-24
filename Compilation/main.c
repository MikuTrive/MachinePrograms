/*
 * Machine compiler entry point.
 *
 * Pipeline overview:
 *   read file -> preprocess -- comments -> lex -> parse/check -> generate C
 *   -> invoke system C compiler -> native executable
 */

#include "cli.h"
#include "codegen.h"
#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int compile_source_file(const char *input_path, const char *output_path) {
    char *buffer = NULL;
    size_t length = 0;
    if (!read_text_file(input_path, &buffer, &length)) {
        fprintf(stderr, "machine: failed to read source file '%s'\n", input_path);
        return 1;
    }

    char *preprocessed = NULL;
    int comment_line = 0;
    int comment_col = 0;
    if (!preprocess_machine_source(buffer, &preprocessed, &comment_line, &comment_col)) {
        fprintf(stderr, "%s:%d:%d: error: comment preprocessing failed\n", input_path, comment_line, comment_col);
        free_text_file(buffer);
        return 1;
    }

    SourceFile src = { input_path, preprocessed, strlen(preprocessed) };
    DiagnosticList *lex_errors = calloc(1, sizeof(DiagnosticList));
    DiagnosticList *parse_errors = calloc(1, sizeof(DiagnosticList));
    DiagnosticList *warnings = calloc(1, sizeof(DiagnosticList));
    DiagnosticList *codegen_errors = calloc(1, sizeof(DiagnosticList));
    TokenList *tokens = calloc(1, sizeof(TokenList));
    Program *program = calloc(1, sizeof(Program));
    if (!lex_errors || !parse_errors || !warnings || !codegen_errors || !tokens || !program) {
        fprintf(stderr, "machine: out of memory while preparing compiler state\n");
        free(preprocessed);
        free_text_file(buffer);
        free(lex_errors); free(parse_errors); free(warnings); free(codegen_errors); free(tokens); free(program);
        return 1;
    }

    if (!lex_source(&src, tokens, lex_errors)) {
        diagnostics_print("error", input_path, lex_errors);
        free(preprocessed);
        free_text_file(buffer);
        free(lex_errors); free(parse_errors); free(warnings); free(codegen_errors); free(tokens); free(program);
        return 1;
    }
    if (!parse_program(&src, tokens, program, parse_errors, warnings)) {
        diagnostics_print("error", input_path, parse_errors);
        free_program(program);
        free(preprocessed);
        free_text_file(buffer);
        free(lex_errors); free(parse_errors); free(warnings); free(codegen_errors); free(tokens); free(program);
        return 1;
    }
    if (warnings->count > 0) {
        diagnostics_print("warning", input_path, warnings);
    }

    char generated_c[1024];
    if (!append_cstr_suffix(generated_c, sizeof(generated_c), output_path, ".machine_tmp.c")) {
        fprintf(stderr, "machine: output path is too long\n");
        free_program(program);
        free(preprocessed);
        free_text_file(buffer);
        free(lex_errors); free(parse_errors); free(warnings); free(codegen_errors); free(tokens); free(program);
        return 1;
    }
    if (!generate_c_file(program, generated_c, codegen_errors)) {
        diagnostics_print("error", input_path, codegen_errors);
        free_program(program);
        free(preprocessed);
        free_text_file(buffer);
        free(lex_errors); free(parse_errors); free(warnings); free(codegen_errors); free(tokens); free(program);
        return 1;
    }
    if (!compile_c_to_binary(generated_c, output_path, codegen_errors)) {
        diagnostics_print("error", input_path, codegen_errors);
        remove(generated_c);
        free_program(program);
        free(preprocessed);
        free_text_file(buffer);
        free(lex_errors); free(parse_errors); free(warnings); free(codegen_errors); free(tokens); free(program);
        return 1;
    }

    remove(generated_c);
    free_program(program);
    free(preprocessed);
    free_text_file(buffer);
    free(lex_errors); free(parse_errors); free(warnings); free(codegen_errors); free(tokens); free(program);
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        print_help();
        return 0;
    }
    if (strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }
    if (strcmp(argv[1], "--version") == 0) {
        print_version();
        return 0;
    }

    const char *input_path = argv[1];
    const char *output_path = NULL;
    if (!ends_with(input_path, ".mne")) {
        fprintf(stderr, "machine: input file must use the .mne extension\n");
        return 1;
    }

    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_path = argv[++i];
        } else {
            fprintf(stderr, "machine: unknown argument '%s'\n", argv[i]);
            return 1;
        }
    }

    char inferred_output[1024];
    if (!output_path) {
        path_change_extension(input_path, "", inferred_output, sizeof(inferred_output));
        output_path = inferred_output;
    }

    return compile_source_file(input_path, output_path);
}
