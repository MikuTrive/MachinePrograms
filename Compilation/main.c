/*
 * Annotated reading edition of main.c
 * ----------------------------------
 * This file is intentionally left functionally unchanged.
 * The only additions in this copy are explanatory comments.
 *
 * Central role of this file:
 *   main.c is the orchestration layer of the Machine compiler executable.
 *   It wires together the front end, diagnostics, code generation, and
 *   final binary production.
 *
 * End-to-end flow implemented here:
 *   1) Parse top-level command-line arguments.
 *   2) Read the input .mne source file from disk.
 *   3) Preprocess Machine-specific source features such as line comments
 *      and compile directives.
 *   4) Tokenize the source.
 *   5) Parse tokens into the internal Program representation.
 *   6) Record target/backend/unsafe settings into the program state.
 *   7) Generate either C or assembly temporary output.
 *   8) Invoke the appropriate system toolchain stage to produce the final
 *      executable or target artifact.
 *
 * Architectural note:
 *   This file does not try to do deep parsing or code generation itself.
 *   Instead, it coordinates specialized modules through a clean pipeline.
 */

/*
 * Machine compiler entry point.
 *
 * Pipeline overview:
 *   read file -> preprocess -- comments / directives -> lex -> parse/check
 *   -> generate C -> invoke system C compiler -> native executable
 */

#include "cli.h"
#include "codegen.h"
#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "util.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * parse_target_name
 * -----------------
 * Purpose:
 *   Translate a textual target name from either the command line or a
 *   source directive into the internal MachineTarget enumeration.
 *
 * Inputs:
 *   - name: textual spelling such as "linux-hosted" or "baremetal-x86_64"
 *   - target: output pointer that receives the parsed enum value
 *
 * Return value:
 *   - 1 if the target name is recognized and stored
 *   - 0 if the input is invalid or unsupported
 *
 * Why this helper exists:
 *   Keeping string-to-enum translation in one place avoids duplicated
 *   strcmp chains elsewhere in the codebase. It also centralizes the set
 *   of accepted target spellings.
 */
static int parse_target_name(const char *name, MachineTarget *target)
{
    if (!name || !target)
        return 0;
    if (strcmp(name, "linux-hosted") == 0)
    {
        *target = MACHINE_TARGET_LINUX_HOSTED;
        return 1;
    }
    if (strcmp(name, "freestanding-x86_64") == 0)
    {
        *target = MACHINE_TARGET_FREESTANDING_X86_64;
        return 1;
    }
    if (strcmp(name, "baremetal-x86_64") == 0)
    {
        *target = MACHINE_TARGET_BAREMETAL_X86_64;
        return 1;
    }
    return 0;
}

/*
 * parse_backend_name
 * ------------------
 * Purpose:
 *   Translate a textual backend name into the internal MachineBackend enum.
 *
 * Supported examples in this version:
 *   - "c"
 *   - "x86_64-asm"
 *
 * Return policy:
 *   - 1 on success
 *   - 0 on failure
 *
 * Design benefit:
 *   This gives command-line parsing and source-directive parsing a shared
 *   conversion routine, which keeps accepted backend names consistent.
 */
static int parse_backend_name(const char *name, MachineBackend *backend)
{
    if (!name || !backend)
        return 0;
    if (strcmp(name, "c") == 0)
    {
        *backend = MACHINE_BACKEND_C;
        return 1;
    }
    if (strcmp(name, "x86_64-asm") == 0)
    {
        *backend = MACHINE_BACKEND_X86_64_ASM;
        return 1;
    }
    return 0;
}

/*
 * apply_source_directives
 * -----------------------
 * Purpose:
 *   Scan the already-loaded source text line by line and detect top-level
 *   compile directives embedded directly in the .mne file.
 *
 * Directives handled here:
 *   - bin.runtime
 *   - unsafe.enable
 *   - target.<name>
 *   - backend.<name>
 *
 * Important implementation detail:
 *   When a directive is recognized, the source characters for that line are
 *   replaced with spaces rather than physically removed from the buffer.
 *
 * Why replace with spaces instead of deleting text?
 *   - Source offsets, line numbers, and column locations remain stable.
 *   - Later phases such as lexing and diagnostics still see a buffer with
 *     the original line layout preserved.
 *   - This is especially helpful for precise error reporting.
 *
 * Default behavior:
 *   If an options structure is supplied, this function first resets it to
 *   the compiler's default assumptions:
 *     - no preferred system runtime
 *     - unsafe mode disabled
 *     - linux-hosted target
 *     - C backend
 *
 * Scope note:
 *   This routine only interprets known directives. It does not try to parse
 *   the full Machine language, and it intentionally operates on raw text.
 */
static void apply_source_directives(char *source_text, MachineCompileOptions *options)
{
    char *line = source_text;
    if (options)
    {
        options->prefer_system_runtime = 0;
        options->allow_unsafe = 0;
        options->target = MACHINE_TARGET_LINUX_HOSTED;
        options->backend = MACHINE_BACKEND_C;
    }

    while (*line)
    {
        char *line_end = line;
        while (*line_end && *line_end != '\n')
            ++line_end;

        char *trim_left = line;
        while (trim_left < line_end && (*trim_left == ' ' || *trim_left == '\t' || *trim_left == '\r'))
            ++trim_left;

        char *trim_right = line_end;
        while (trim_right > trim_left && (trim_right[-1] == ' ' || trim_right[-1] == '\t' || trim_right[-1] == '\r'))
            --trim_right;

        size_t len = (size_t)(trim_right - trim_left);
        if (len == strlen("bin.runtime") && strncmp(trim_left, "bin.runtime", len) == 0)
        {
            if (options)
                options->prefer_system_runtime = 1;
            for (char *p = line; p < line_end; ++p)
                *p = ' ';
        }
        else if (len == strlen("unsafe.enable") && strncmp(trim_left, "unsafe.enable", len) == 0)
        {
            if (options)
                options->allow_unsafe = 1;
            for (char *p = line; p < line_end; ++p)
                *p = ' ';
        }
        else if (len > strlen("target.") && strncmp(trim_left, "target.", strlen("target.")) == 0)
        {
            char name[128];
            size_t n = len - strlen("target.");
            if (n >= sizeof(name))
                n = sizeof(name) - 1;
            memcpy(name, trim_left + strlen("target."), n);
            name[n] = '\0';
            if (options)
                parse_target_name(name, &options->target);
            for (char *p = line; p < line_end; ++p)
                *p = ' ';
        }
        else if (len > strlen("backend.") && strncmp(trim_left, "backend.", strlen("backend.")) == 0)
        {
            char name[128];
            size_t n = len - strlen("backend.");
            if (n >= sizeof(name))
                n = sizeof(name) - 1;
            memcpy(name, trim_left + strlen("backend."), n);
            name[n] = '\0';
            if (options)
                parse_backend_name(name, &options->backend);
            for (char *p = line; p < line_end; ++p)
                *p = ' ';
        }

        if (*line_end == '\0')
            break;
        line = line_end + 1;
    }
}

/*
 * compile_source_file
 * -------------------
 * Purpose:
 *   Execute the full compiler pipeline for one input .mne file and produce
 *   the requested output artifact.
 *
 * High-level stages inside this function:
 *   1) Read the source file from disk.
 *   2) Preprocess comment/directive-related syntax.
 *   3) Merge command-line options with source-file directives.
 *   4) Lex the source into tokens.
 *   5) Parse tokens into a Program AST / semantic structure.
 *   6) Propagate target/backend/unsafe options into the Program object.
 *   7) Generate a temporary .c or .s file depending on the backend.
 *   8) Invoke the corresponding native toolchain step.
 *   9) Clean up temporary files and all allocated compiler state.
 *
 * Error handling style:
 *   This function follows an explicit early-return pattern. After each
 *   pipeline stage it checks for failure, emits diagnostics, frees any
 *   already-allocated state, and returns non-zero.
 *
 * Why this function is large:
 *   It is the pipeline coordinator. The heavy semantic work still lives in
 *   other modules, but the sequencing and cleanup responsibilities live here.
 */
static int compile_source_file(const char *input_path, const char *output_path, const MachineCompileOptions *cli_options)
{
    /*
     * Raw source buffer loaded from disk before preprocessing.
     * This keeps the original file contents available until a separate
     * preprocessed buffer is produced.
     */
    char *buffer = NULL;
    size_t length = 0;
    MachineCompileOptions options = {0};
    if (!read_text_file(input_path, &buffer, &length))
    {
        fprintf(stderr, "machine: failed to read source file '%s'\n", input_path);
        return 1;
    }

    /*
     * Preprocessed source text. This buffer is the one passed to later
     * compiler stages after comment/directive normalization.
     */
    char *preprocessed = NULL;
    int comment_line = 0;
    int comment_col = 0;
    if (!preprocess_machine_source(buffer, &preprocessed, &comment_line, &comment_col))
    {
        fprintf(stderr, "%s:%d:%d: error: comment preprocessing failed\n", input_path, comment_line, comment_col);
        free_text_file(buffer);
        return 1;
    }
    if (cli_options)
        options = *cli_options;
    apply_source_directives(preprocessed, &options);
    if (cli_options)
    {
        if (cli_options->prefer_system_runtime)
            options.prefer_system_runtime = cli_options->prefer_system_runtime;
        if (cli_options->allow_unsafe)
            options.allow_unsafe = cli_options->allow_unsafe;
        if (cli_options->target != MACHINE_TARGET_LINUX_HOSTED)
            options.target = cli_options->target;
        if (cli_options->backend != MACHINE_BACKEND_C)
            options.backend = cli_options->backend;
    }

    /*
     * SourceFile is the compact object handed into lexing/parsing. It ties
     * together the path, the source buffer, and the text length so later
     * phases do not need to manage these separately.
     */
    SourceFile src = {input_path, preprocessed, strlen(preprocessed)};
    DiagnosticList *lex_errors = calloc(1, sizeof(DiagnosticList));
    DiagnosticList *parse_errors = calloc(1, sizeof(DiagnosticList));
    DiagnosticList *warnings = calloc(1, sizeof(DiagnosticList));
    DiagnosticList *codegen_errors = calloc(1, sizeof(DiagnosticList));
    TokenList *tokens = calloc(1, sizeof(TokenList));
    Program *program = calloc(1, sizeof(Program));
    if (!lex_errors || !parse_errors || !warnings || !codegen_errors || !tokens || !program)
    {
        fprintf(stderr, "machine: out of memory while preparing compiler state\n");
        free(preprocessed);
        free_text_file(buffer);
        free(lex_errors);
        free(parse_errors);
        free(warnings);
        free(codegen_errors);
        free(tokens);
        free(program);
        return 1;
    }

    if (!lex_source(&src, tokens, lex_errors))
    {
        diagnostics_print("error", input_path, lex_errors);
        free(preprocessed);
        free_text_file(buffer);
        free(lex_errors);
        free(parse_errors);
        free(warnings);
        free(codegen_errors);
        free(tokens);
        free(program);
        return 1;
    }
    if (!parse_program(&src, tokens, program, parse_errors, warnings, options.allow_unsafe))
    {
        diagnostics_print("error", input_path, parse_errors);
        free_program(program);
        free(preprocessed);
        free_text_file(buffer);
        free(lex_errors);
        free(parse_errors);
        free(warnings);
        free(codegen_errors);
        free(tokens);
        free(program);
        return 1;
    }
    if (warnings->count > 0)
        diagnostics_print("warning", input_path, warnings);

    program->allow_unsafe = options.allow_unsafe;
    program->target_id = options.target;
    program->backend_id = options.backend;

    /*
     * The compiler writes an intermediate file before invoking the system
     * compiler or assembler toolchain. The suffix depends on the chosen
     * backend:
     *   - .machine_tmp.c for the C backend
     *   - .machine_tmp.s for the assembly backend
     */
    char generated_tmp[1024];
    const char *tmp_suffix = (options.backend == MACHINE_BACKEND_X86_64_ASM) ? ".machine_tmp.s" : ".machine_tmp.c";
    if (!append_cstr_suffix(generated_tmp, sizeof(generated_tmp), output_path, tmp_suffix))
    {
        fprintf(stderr, "machine: output path is too long\n");
        free_program(program);
        free(preprocessed);
        free_text_file(buffer);
        free(lex_errors);
        free(parse_errors);
        free(warnings);
        free(codegen_errors);
        free(tokens);
        free(program);
        return 1;
    }
    /*
     * Backend split:
     *   If the assembly backend is selected, generate assembly and then link
     *   it into the final binary. Otherwise, generate C and compile that.
     */
    if (options.backend == MACHINE_BACKEND_X86_64_ASM)
    {
        if (!generate_asm_file(program, generated_tmp, codegen_errors))
        {
            diagnostics_print("error", input_path, codegen_errors);
            free_program(program);
            free(preprocessed);
            free_text_file(buffer);
            free(lex_errors);
            free(parse_errors);
            free(warnings);
            free(codegen_errors);
            free(tokens);
            free(program);
            return 1;
        }
        if (!compile_asm_to_binary(generated_tmp, output_path, input_path, &options, codegen_errors))
        {
            diagnostics_print("error", input_path, codegen_errors);
            remove(generated_tmp);
            free_program(program);
            free(preprocessed);
            free_text_file(buffer);
            free(lex_errors);
            free(parse_errors);
            free(warnings);
            free(codegen_errors);
            free(tokens);
            free(program);
            return 1;
        }
    }
    else
    {
        if (!generate_c_file(program, generated_tmp, codegen_errors))
        {
            diagnostics_print("error", input_path, codegen_errors);
            free_program(program);
            free(preprocessed);
            free_text_file(buffer);
            free(lex_errors);
            free(parse_errors);
            free(warnings);
            free(codegen_errors);
            free(tokens);
            free(program);
            return 1;
        }
        if (!compile_c_to_binary(generated_tmp, output_path, input_path, &options, codegen_errors))
        {
            diagnostics_print("error", input_path, codegen_errors);
            remove(generated_tmp);
            free_program(program);
            free(preprocessed);
            free_text_file(buffer);
            free(lex_errors);
            free(parse_errors);
            free(warnings);
            free(codegen_errors);
            free(tokens);
            free(program);
            return 1;
        }
    }

    remove(generated_tmp);
    free_program(program);
    free(preprocessed);
    free_text_file(buffer);
    free(lex_errors);
    free(parse_errors);
    free(warnings);
    free(codegen_errors);
    free(tokens);
    free(program);
    return 0;
}

/*
 * main
 * ----
 * Purpose:
 *   Entry point of the machine compiler executable.
 *
 * Responsibilities:
 *   - Handle trivial top-level modes such as --help and --version.
 *   - Validate the input file extension.
 *   - Parse command-line overrides for output path, target, backend, unsafe
 *     mode, and runtime preference.
 *   - Infer an output path when the user does not specify -o.
 *   - Delegate the actual compilation work to compile_source_file().
 *
 * Policy detail:
 *   Defaults are intentionally established here before command-line parsing:
 *   - target defaults to linux-hosted
 *   - backend defaults to C
 *
 * Source directives vs CLI options:
 *   Later in the pipeline, source directives are applied first and then
 *   selected CLI options override them when explicitly present. That gives
 *   source files self-description while still letting the command line force
 *   a different compilation mode.
 */
int main(int argc, char **argv)
{
    if (argc == 1)
    {
        print_help();
        return 0;
    }
    if (strcmp(argv[1], "--help") == 0)
    {
        print_help();
        return 0;
    }
    if (strcmp(argv[1], "--version") == 0)
    {
        print_version();
        return 0;
    }

    /*
     * The first non-program argument is treated as the input source path.
     * Simple top-level modes (--help / --version) have already returned.
     */
    const char *input_path = argv[1];
    const char *output_path = NULL;
    MachineCompileOptions options = {0};
    options.target = MACHINE_TARGET_LINUX_HOSTED;
    options.backend = MACHINE_BACKEND_C;
    if (!ends_with(input_path, ".mne"))
    {
        fprintf(stderr, "machine: input file must use the .mne extension\n");
        return 1;
    }

    /*
     * Parse remaining command-line arguments after the input file.
     * This loop recognizes:
     *   - -o <output>
     *   - --target <name>
     *   - --backend <name>
     *   - --unsafe
     *   - --prefer-system-runtime
     */
    for (int i = 2; i < argc; ++i)
    {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
            output_path = argv[++i];
        else if (strcmp(argv[i], "--target") == 0 && i + 1 < argc)
        {
            if (!parse_target_name(argv[++i], &options.target))
            {
                fprintf(stderr, "machine: unknown target '%s'\n", argv[i]);
                return 1;
            }
        }
        else if (strcmp(argv[i], "--backend") == 0 && i + 1 < argc)
        {
            if (!parse_backend_name(argv[++i], &options.backend))
            {
                fprintf(stderr, "machine: unknown backend '%s'\n", argv[i]);
                return 1;
            }
        }
        else if (strcmp(argv[i], "--unsafe") == 0)
            options.allow_unsafe = 1;
        else if (strcmp(argv[i], "--prefer-system-runtime") == 0)
            options.prefer_system_runtime = 1;
        else
        {
            fprintf(stderr, "machine: unknown argument '%s'\n", argv[i]);
            return 1;
        }
    }

    /*
     * If the user did not provide -o, derive an output path by removing the
     * .mne extension from the input file path.
     */
    char inferred_output[1024];
    if (!output_path)
    {
        path_change_extension(input_path, "", inferred_output, sizeof(inferred_output));
        output_path = inferred_output;
    }

    return compile_source_file(input_path, output_path, &options);
}
