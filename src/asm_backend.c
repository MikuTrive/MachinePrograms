/*
 * Annotated reading copy of asm_backend.c
 *
 * What this file is for:
 * - Generate x86_64 assembly output from the Machine AST and coordinate assembly-oriented code emission decisions.
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

// -----------------------------------------------------------------------------
// Additional safe annotation layer
//
// This copy adds only single-line comments so that it remains safe even when the
// original file already contains many block comments. No original code line has
// been rewritten, reordered, or removed. The purpose of these extra notes is to
// make the file easier to study without introducing nested block-comment risks.
// -----------------------------------------------------------------------------

#include "codegen.h"
#include "util.h"
#include "machine_runtime.h"
#include "version.h"

#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Data structure: AsmStringRef
// Keeps a mapping between an AST string literal expression and the numeric label
// assigned to that literal in emitted assembly output.
typedef struct
{
    const Expr *expr;
    int label_id;
} AsmStringRef;

// Data structure: AsmLocal
// Describes one local variable or parameter as tracked by the assembly backend.
// The backend uses this to remember the variable name, type, stack slot offset,
// and whether the symbol originated from a function parameter.
typedef struct
{
    char name[64];
    TypeRef type;
    int offset;
    int is_param;
    int param_index;
} AsmLocal;

// Data structure: AsmGenerator
// Global state for one assembly generation pass over an entire Machine program.
// This object carries output streams, diagnostic sinks, target selection, string
// table information, and global failure status across all emitted functions.
typedef struct
{
    FILE *out;
    DiagnosticList *errors;
    const Program *program;
    int target_id;
    int failed;
    int next_label_id;
    int next_string_label_id;
    AsmStringRef strings[4096];
    size_t string_count;
} AsmGenerator;

// Data structure: AsmFunctionCtx
// Per-function emission state. This bundles the current function declaration,
// its discovered locals, stack layout, label sequencing, and return label state.
typedef struct
{
    AsmGenerator *gen;
    const FunctionDecl *func;
    AsmLocal locals[512];
    size_t local_count;
    int stack_size;
    int next_label_id;
    char return_label[64];
    int failed;
} AsmFunctionCtx;

// Data structure: RuntimeBundle
// Represents the hosted-runtime bundle chosen for a build: include directory,
// runtime object path, optional runtime source path, and whether source exists.
typedef struct
{
    char object_path[PATH_MAX];
    char include_dir[PATH_MAX];
    char runtime_source[PATH_MAX];
    int has_source;
} RuntimeBundle;

// Data structure: FreestandingBundle
// Represents the source/header bundle needed to build a freestanding target.
typedef struct
{
    char include_dir[PATH_MAX];
    char runtime_source[PATH_MAX];
    char entry_source[PATH_MAX];
} FreestandingBundle;

// Data structure: BaremetalBundle
// Represents the source/header/linker bundle needed to build a bare-metal target.
typedef struct
{
    char include_dir[PATH_MAX];
    char runtime_source[PATH_MAX];
    char entry_source[PATH_MAX];
    char linker_script[PATH_MAX];
} BaremetalBundle;

// Register convention table
// Maps Machine call arguments 0..5 onto the System V x86_64 integer argument
// registers used by the backend when preparing direct calls.
static const char *asm_arg_regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

static char *format_alloc(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int needed = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (needed < 0)
        return NULL;
    char *buf = (char *)malloc((size_t)needed + 1);
    if (!buf)
        return NULL;
    va_start(ap, fmt);
    vsnprintf(buf, (size_t)needed + 1, fmt, ap);
    va_end(ap);
    return buf;
}

static char *shell_quote_single(const char *value)
{
    size_t extra = 2;
    for (const char *p = value; *p; ++p)
        extra += (*p == '\'') ? 4 : 1;
    char *out = (char *)malloc(extra + 1);
    char *w = out;
    if (!out)
        return NULL;
    *w++ = '\'';
    for (const char *p = value; *p; ++p)
    {
        if (*p == '\'')
        {
            memcpy(w, "'\\''", 4);
            w += 4;
        }
        else
            *w++ = *p;
    }
    *w++ = '\'';
    *w = '\0';
    return out;
}

/*
 * Function overview: file_contains_text
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "file contains text".
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
static int file_contains_text(const char *path, const char *needle)
{
    char *buffer = NULL;
    size_t length = 0;
    int found = 0;
    if (!read_text_file(path, &buffer, &length))
        return 0;
    (void)length;
    if (strstr(buffer, needle) != NULL)
        found = 1;
    free_text_file(buffer);
    return found;
}

/*
 * Function overview: runtime_header_is_compatible
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "runtime header is compatible".
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
static int runtime_header_is_compatible(const char *include_dir)
{
    char header_path[PATH_MAX];
    char version_line[128];
    if (!join_cstr3(header_path, sizeof(header_path), include_dir, "/", "machine_runtime.h"))
        return 0;
    if (!file_exists(header_path))
        return 0;
    snprintf(version_line, sizeof(version_line), "#define MACHINE_RUNTIME_API_VERSION %d", MACHINE_RUNTIME_API_VERSION);
    if (file_contains_text(header_path, version_line))
        return 1;
    return file_contains_text(header_path, "machine_ptr_hex(") &&
           file_contains_text(header_path, "machine_ptr_bin(") &&
           file_contains_text(header_path, "machine_ptr_offset(");
}

/*
 * Function overview: run_compiler_command
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "run compiler command".
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
static int run_compiler_command(DiagnosticList *errors, const char *binary_path, const char *message, const char *command)
{
    if (!command)
    {
        diagnostics_add(NULL, errors, 1, 1, "failed to build compiler command while building '%s'", binary_path);
        return 0;
    }
    if (system(command) != 0)
    {
        diagnostics_add(NULL, errors, 1, 1, "%s", message ? message : "compiler command failed");
        return 0;
    }
    return 1;
}

/*
 * Function overview: set_runtime_bundle
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "set runtime bundle".
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
static int set_runtime_bundle(RuntimeBundle *bundle, const char *obj, const char *incdir, const char *source)
{
    if (!bundle || !obj || !incdir)
        return 0;
    if (!copy_cstr(bundle->object_path, sizeof(bundle->object_path), obj))
        return 0;
    if (!copy_cstr(bundle->include_dir, sizeof(bundle->include_dir), incdir))
        return 0;
    bundle->has_source = 0;
    bundle->runtime_source[0] = '\0';
    if (source && *source)
    {
        if (!copy_cstr(bundle->runtime_source, sizeof(bundle->runtime_source), source))
            return 0;
        bundle->has_source = 1;
    }
    return 1;
}

/*
 * Function overview: choose_runtime_candidate
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "choose runtime candidate".
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
static int choose_runtime_candidate(RuntimeBundle *bundle, const char *obj_path, const char *include_dir, const char *runtime_source)
{
    if (!file_exists(obj_path))
    {
        if (!runtime_source || !*runtime_source || !file_exists(runtime_source))
            return 0;
    }
    if (!runtime_header_is_compatible(include_dir))
        return 0;
    return set_runtime_bundle(bundle, obj_path, include_dir, runtime_source);
}

/*
 * Function overview: choose_runtime_bundle
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "choose runtime bundle".
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
static int choose_runtime_bundle(RuntimeBundle *bundle, const char *source_path, int prefer_system_runtime)
{
    char cwd[PATH_MAX] = {0};
    char source_dir[PATH_MAX] = {0};
    char self_path[PATH_MAX] = {0};
    char self_dir[PATH_MAX] = {0};
    const char *paths[16][3];
    size_t count = 0;
    if (!getcwd(cwd, sizeof(cwd)))
        copy_cstr(cwd, sizeof(cwd), ".");
    if (!path_dirname(source_path, source_dir, sizeof(source_dir)))
        copy_cstr(source_dir, sizeof(source_dir), ".");
    ssize_t link_len = readlink("/proc/self/exe", self_path, sizeof(self_path) - 1);
    if (link_len > 0)
    {
        self_path[link_len] = '\0';
        path_dirname(self_path, self_dir, sizeof(self_dir));
    }
#define ADD(obj, inc, src)       \
    do                           \
    {                            \
        paths[count][0] = (obj); \
        paths[count][1] = (inc); \
        paths[count][2] = (src); \
        ++count;                 \
    } while (0)
    if (prefer_system_runtime)
        ADD("/usr/local/lib/machine/machine_runtime.o", "/usr/local/include/machine", "/usr/local/lib/machine/runtime.c");
    {
        char obj[PATH_MAX], inc[PATH_MAX], src[PATH_MAX];
        join_cstr3(obj, sizeof(obj), cwd, "/build/", "machine_runtime.o");
        join_cstr3(inc, sizeof(inc), cwd, "/include", "");
        join_cstr3(src, sizeof(src), cwd, "/src/", "runtime.c");
        ADD(strdup(obj), strdup(inc), strdup(src));
        join_cstr3(obj, sizeof(obj), source_dir, "/build/", "machine_runtime.o");
        join_cstr3(inc, sizeof(inc), source_dir, "/include", "");
        join_cstr3(src, sizeof(src), source_dir, "/src/", "runtime.c");
        ADD(strdup(obj), strdup(inc), strdup(src));
        if (self_dir[0])
        {
            join_cstr3(obj, sizeof(obj), self_dir, "/build/", "machine_runtime.o");
            join_cstr3(inc, sizeof(inc), self_dir, "/include", "");
            join_cstr3(src, sizeof(src), self_dir, "/src/", "runtime.c");
            ADD(strdup(obj), strdup(inc), strdup(src));
        }
    }
    if (!prefer_system_runtime)
        ADD("/usr/local/lib/machine/machine_runtime.o", "/usr/local/include/machine", "/usr/local/lib/machine/runtime.c");
#undef ADD
    for (size_t i = 0; i < count; ++i)
    {
        if (choose_runtime_candidate(bundle, paths[i][0], paths[i][1], paths[i][2]))
            return 1;
    }
    return 0;
}

/*
 * Function overview: set_freestanding_bundle
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "set freestanding bundle".
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
static int set_freestanding_bundle(FreestandingBundle *bundle, const char *incdir, const char *runtime_source, const char *entry_source)
{
    return copy_cstr(bundle->include_dir, sizeof(bundle->include_dir), incdir) &&
           copy_cstr(bundle->runtime_source, sizeof(bundle->runtime_source), runtime_source) &&
           copy_cstr(bundle->entry_source, sizeof(bundle->entry_source), entry_source);
}

/*
 * Function overview: choose_freestanding_support_bundle
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "choose freestanding support bundle".
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
static int choose_freestanding_support_bundle(FreestandingBundle *bundle, const char *source_path)
{
    char cwd[PATH_MAX] = {0};
    char source_dir[PATH_MAX] = {0};
    char self_path[PATH_MAX] = {0};
    char self_dir[PATH_MAX] = {0};
    struct Candidate
    {
        char inc[PATH_MAX];
        char runtime[PATH_MAX];
        char entry[PATH_MAX];
    } candidates[8];
    size_t count = 0;
    if (!getcwd(cwd, sizeof(cwd)))
        copy_cstr(cwd, sizeof(cwd), ".");
    if (!path_dirname(source_path, source_dir, sizeof(source_dir)))
        copy_cstr(source_dir, sizeof(source_dir), ".");
    ssize_t link_len = readlink("/proc/self/exe", self_path, sizeof(self_path) - 1);
    if (link_len > 0)
    {
        self_path[link_len] = '\0';
        path_dirname(self_path, self_dir, sizeof(self_dir));
    }
#define ADD_FS(base_inc, runtime_src, entry_src)                                                \
    do                                                                                          \
    {                                                                                           \
        copy_cstr(candidates[count].inc, sizeof(candidates[count].inc), (base_inc));            \
        copy_cstr(candidates[count].runtime, sizeof(candidates[count].runtime), (runtime_src)); \
        copy_cstr(candidates[count].entry, sizeof(candidates[count].entry), (entry_src));       \
        ++count;                                                                                \
    } while (0)
    ADD_FS("/usr/local/include/machine", "/usr/local/lib/machine/machine_runtime_freestanding.c", "/usr/local/lib/machine/machine_runtime_freestanding_entry.S");
    {
        char inc[PATH_MAX], runtime_src[PATH_MAX], entry_src[PATH_MAX];
        join_cstr3(inc, sizeof(inc), cwd, "/include", "");
        join_cstr3(runtime_src, sizeof(runtime_src), cwd, "/src/", "runtime_freestanding.c");
        join_cstr3(entry_src, sizeof(entry_src), cwd, "/src/", "runtime_freestanding_entry.S");
        ADD_FS(inc, runtime_src, entry_src);
        join_cstr3(inc, sizeof(inc), source_dir, "/include", "");
        join_cstr3(runtime_src, sizeof(runtime_src), source_dir, "/src/", "runtime_freestanding.c");
        join_cstr3(entry_src, sizeof(entry_src), source_dir, "/src/", "runtime_freestanding_entry.S");
        ADD_FS(inc, runtime_src, entry_src);
        if (self_dir[0])
        {
            join_cstr3(inc, sizeof(inc), self_dir, "/include", "");
            join_cstr3(runtime_src, sizeof(runtime_src), self_dir, "/src/", "runtime_freestanding.c");
            join_cstr3(entry_src, sizeof(entry_src), self_dir, "/src/", "runtime_freestanding_entry.S");
            ADD_FS(inc, runtime_src, entry_src);
        }
    }
#undef ADD_FS
    for (size_t i = 0; i < count; ++i)
    {
        char header_path[PATH_MAX];
        if (!join_cstr3(header_path, sizeof(header_path), candidates[i].inc, "/", "machine_runtime_freestanding.h"))
            continue;
        if (!file_exists(header_path) || !file_exists(candidates[i].runtime) || !file_exists(candidates[i].entry))
            continue;
        return set_freestanding_bundle(bundle, candidates[i].inc, candidates[i].runtime, candidates[i].entry);
    }
    return 0;
}

/*
 * Function overview: set_baremetal_bundle
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "set baremetal bundle".
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
static int set_baremetal_bundle(BaremetalBundle *bundle, const char *incdir, const char *runtime_source, const char *entry_source, const char *linker_script)
{
    return copy_cstr(bundle->include_dir, sizeof(bundle->include_dir), incdir) &&
           copy_cstr(bundle->runtime_source, sizeof(bundle->runtime_source), runtime_source) &&
           copy_cstr(bundle->entry_source, sizeof(bundle->entry_source), entry_source) &&
           copy_cstr(bundle->linker_script, sizeof(bundle->linker_script), linker_script);
}

/*
 * Function overview: choose_baremetal_support_bundle
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "choose baremetal support bundle".
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
static int choose_baremetal_support_bundle(BaremetalBundle *bundle, const char *source_path)
{
    char cwd[PATH_MAX] = {0};
    char source_dir[PATH_MAX] = {0};
    char self_path[PATH_MAX] = {0};
    char self_dir[PATH_MAX] = {0};
    struct Candidate
    {
        char inc[PATH_MAX];
        char runtime[PATH_MAX];
        char entry[PATH_MAX];
        char ld[PATH_MAX];
    } candidates[8];
    size_t count = 0;
    if (!getcwd(cwd, sizeof(cwd)))
        copy_cstr(cwd, sizeof(cwd), ".");
    if (!path_dirname(source_path, source_dir, sizeof(source_dir)))
        copy_cstr(source_dir, sizeof(source_dir), ".");
    ssize_t link_len = readlink("/proc/self/exe", self_path, sizeof(self_path) - 1);
    if (link_len > 0)
    {
        self_path[link_len] = '\0';
        path_dirname(self_path, self_dir, sizeof(self_dir));
    }
#define ADD_BM(base_inc, runtime_src, entry_src, link_src)                                      \
    do                                                                                          \
    {                                                                                           \
        copy_cstr(candidates[count].inc, sizeof(candidates[count].inc), (base_inc));            \
        copy_cstr(candidates[count].runtime, sizeof(candidates[count].runtime), (runtime_src)); \
        copy_cstr(candidates[count].entry, sizeof(candidates[count].entry), (entry_src));       \
        copy_cstr(candidates[count].ld, sizeof(candidates[count].ld), (link_src));              \
        ++count;                                                                                \
    } while (0)
    ADD_BM("/usr/local/include/machine", "/usr/local/lib/machine/machine_runtime_baremetal.c", "/usr/local/lib/machine/machine_runtime_baremetal_entry.S", "/usr/local/lib/machine/machine_runtime_baremetal_link.ld");
    {
        char inc[PATH_MAX], runtime_src[PATH_MAX], entry_src[PATH_MAX], link_src[PATH_MAX];
        join_cstr3(inc, sizeof(inc), cwd, "/include", "");
        join_cstr3(runtime_src, sizeof(runtime_src), cwd, "/src/", "runtime_baremetal.c");
        join_cstr3(entry_src, sizeof(entry_src), cwd, "/src/", "runtime_baremetal_entry.S");
        join_cstr3(link_src, sizeof(link_src), cwd, "/src/", "runtime_baremetal_link.ld");
        ADD_BM(inc, runtime_src, entry_src, link_src);
        join_cstr3(inc, sizeof(inc), source_dir, "/include", "");
        join_cstr3(runtime_src, sizeof(runtime_src), source_dir, "/src/", "runtime_baremetal.c");
        join_cstr3(entry_src, sizeof(entry_src), source_dir, "/src/", "runtime_baremetal_entry.S");
        join_cstr3(link_src, sizeof(link_src), source_dir, "/src/", "runtime_baremetal_link.ld");
        ADD_BM(inc, runtime_src, entry_src, link_src);
        if (self_dir[0])
        {
            join_cstr3(inc, sizeof(inc), self_dir, "/include", "");
            join_cstr3(runtime_src, sizeof(runtime_src), self_dir, "/src/", "runtime_baremetal.c");
            join_cstr3(entry_src, sizeof(entry_src), self_dir, "/src/", "runtime_baremetal_entry.S");
            join_cstr3(link_src, sizeof(link_src), self_dir, "/src/", "runtime_baremetal_link.ld");
            ADD_BM(inc, runtime_src, entry_src, link_src);
        }
    }
#undef ADD_BM
    for (size_t i = 0; i < count; ++i)
    {
        char header_path[PATH_MAX];
        if (!join_cstr3(header_path, sizeof(header_path), candidates[i].inc, "/", "machine_runtime_baremetal.h"))
            continue;
        if (!file_exists(header_path) || !file_exists(candidates[i].runtime) || !file_exists(candidates[i].entry) || !file_exists(candidates[i].ld))
            continue;
        return set_baremetal_bundle(bundle, candidates[i].inc, candidates[i].runtime, candidates[i].entry, candidates[i].ld);
    }
    return 0;
}

/*
 * Function overview: asm_error
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "asm error".
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
static void asm_error(AsmGenerator *g, int line, int column, const char *fmt, ...)
{
    char buffer[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);
    diagnostics_add(NULL, g->errors, line ? line : 1, column ? column : 1, "%s", buffer);
    g->failed = 1;
}

/*
 * Function overview: is_scalar_type
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "is scalar type".
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
static int is_scalar_type(TypeRef t)
{
    return t.kind == TYPE_I64 || t.kind == TYPE_BOOL || t.kind == TYPE_PTR || t.kind == TYPE_STR || t.kind == TYPE_VOID;
}

/*
 * Function overview: stack_slot_bytes
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "stack slot bytes".
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
static int stack_slot_bytes(TypeRef t)
{
    (void)t;
    return 8;
}

/*
 * Function overview: collect_strings_expr
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "collect strings expr".
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
static void collect_strings_expr(AsmGenerator *g, const Expr *e)
{
    if (!e)
        return;
    if (e->kind == EXPR_STRING)
    {
        if (g->string_count < sizeof(g->strings) / sizeof(g->strings[0]))
        {
            g->strings[g->string_count].expr = e;
            g->strings[g->string_count].label_id = g->next_string_label_id++;
            ++g->string_count;
        }
        else
            asm_error(g, e->line, e->column, "too many string literals for asm backend");
    }
    switch (e->kind)
    {
    case EXPR_UNARY:
        collect_strings_expr(g, e->as.unary.operand);
        break;
    case EXPR_BINARY:
        collect_strings_expr(g, e->as.binary.left);
        collect_strings_expr(g, e->as.binary.right);
        break;
    case EXPR_CALL:
        collect_strings_expr(g, e->as.call.callee);
        for (size_t i = 0; i < e->as.call.arg_count; ++i)
            collect_strings_expr(g, e->as.call.args[i]);
        break;
    case EXPR_INDEX:
        collect_strings_expr(g, e->as.index.base);
        collect_strings_expr(g, e->as.index.index);
        break;
    case EXPR_ARRAY:
        for (size_t i = 0; i < e->as.array.item_count; ++i)
            collect_strings_expr(g, e->as.array.items[i]);
        break;
    case EXPR_FIELD:
        collect_strings_expr(g, e->as.field.base);
        break;
    default:
        break;
    }
}

/*
 * Function overview: collect_strings_stmt
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "collect strings stmt".
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
static void collect_strings_stmt(AsmGenerator *g, const Statement *s)
{
    if (!s)
        return;
    switch (s->kind)
    {
    case STMT_VAR:
    case STMT_CONST:
        if (s->as.var_stmt.has_initializer)
            collect_strings_expr(g, s->as.var_stmt.initializer);
        break;
    case STMT_ASSIGN:
        collect_strings_expr(g, s->as.assign_stmt.target);
        collect_strings_expr(g, s->as.assign_stmt.value);
        break;
    case STMT_PRINT:
        collect_strings_expr(g, s->as.print_stmt.value);
        break;
    case STMT_RETURN:
        collect_strings_expr(g, s->as.return_stmt.value);
        break;
    case STMT_IF:
        collect_strings_expr(g, s->as.if_stmt.condition);
        for (size_t i = 0; i < s->as.if_stmt.then_count; ++i)
            collect_strings_stmt(g, &s->as.if_stmt.then_block[i]);
        for (size_t i = 0; i < s->as.if_stmt.else_count; ++i)
            collect_strings_stmt(g, &s->as.if_stmt.else_block[i]);
        break;
    case STMT_WHILE:
        collect_strings_expr(g, s->as.while_stmt.condition);
        for (size_t i = 0; i < s->as.while_stmt.body_count; ++i)
            collect_strings_stmt(g, &s->as.while_stmt.body[i]);
        break;
    case STMT_EXPR:
        collect_strings_expr(g, s->as.expr_stmt.expr);
        break;
    case STMT_UNSAFE:
        for (size_t i = 0; i < s->as.unsafe_stmt.body_count; ++i)
            collect_strings_stmt(g, &s->as.unsafe_stmt.body[i]);
        break;
    case STMT_SWITCH:
        collect_strings_expr(g, s->as.switch_stmt.value);
        for (size_t i = 0; i < s->as.switch_stmt.case_count; ++i)
        {
            collect_strings_expr(g, s->as.switch_stmt.cases[i].match);
            for (size_t j = 0; j < s->as.switch_stmt.cases[i].body_count; ++j)
                collect_strings_stmt(g, &s->as.switch_stmt.cases[i].body[j]);
        }
        break;
    default:
        break;
    }
}

/*
 * Function overview: label_for_string_expr
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "label for string expr".
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
static int label_for_string_expr(const AsmGenerator *g, const Expr *e)
{
    for (size_t i = 0; i < g->string_count; ++i)
        if (g->strings[i].expr == e)
            return g->strings[i].label_id;
    return -1;
}

/*
 * Function overview: emit_asm_string_escaped
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit asm string escaped".
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
static void emit_asm_string_escaped(FILE *out, const char *text)
{
    fputc('"', out);
    for (const unsigned char *p = (const unsigned char *)text; *p; ++p)
    {
        switch (*p)
        {
        case '\\':
            fputs("\\\\", out);
            break;
        case '"':
            fputs("\\\"", out);
            break;
        case '\n':
            fputs("\\n", out);
            break;
        case '\r':
            fputs("\\r", out);
            break;
        case '\t':
            fputs("\\t", out);
            break;
        default:
            if (*p < 32 || *p > 126)
                fprintf(out, "\\x%02x", (unsigned)*p);
            else
                fputc(*p, out);
            break;
        }
    }
    fputc('"', out);
}

/*
 * Function overview: is_machine_builtin_name_asm
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "is machine builtin name asm".
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
static int is_machine_builtin_name_asm(const char *name)
{
    static const char *builtins[] = {
        "hp_add", "hp_sub", "hp_mul", "hp_div", "hp_sqrt", "hp_pow",
        "alloc_bytes", "free_mem", "store_i64", "load_i64", "store_f64", "load_f64", "store_str", "load_str",
        "ptr_from_i64", "ptr_to_i64", "ptr_offset", "ptr_hex", "ptr_bin", "store_u8", "store_u16", "store_u32", "store_u64",
        "load_u8", "load_u16", "load_u32", "load_u64", "volatile_store_u8", "volatile_store_u16", "volatile_store_u32", "volatile_store_u64",
        "volatile_load_u8", "volatile_load_u16", "volatile_load_u32", "volatile_load_u64", "syscall0", "syscall1", "syscall2", "syscall3",
        "syscall4", "syscall5", "syscall6", "mmap_anon", "mmap_anon_exec", "munmap_mem", "fd_open_ro", "fd_open_wo", "fd_open_rw",
        "fd_close", "fd_read", "fd_write", "fd_seek", "ioctl_i64", "asm_nop", "asm_pause", "asm_mfence", "asm_lfence", "asm_sfence",
        "asm_rdtsc", "cpu_in8", "cpu_out8", "pmm_alloc_page", "pmm_alloc_pages", "pmm_total_bytes", "pmm_used_bytes",
        "page_identity_map_2m", "apic_supported", "apic_enable", "apic_eoi", "list_new", "list_push_back", "list_get", "list_size", "list_free", "array_new", "array_push",
        "array_get", "array_set", "array_len", "array_free", "grid_new", "grid_get", "grid_set", "grid_rows", "grid_cols", "grid_fill", "grid_free",
        "term_enable_raw", "term_disable_raw", "term_key_available", "term_read_key", "term_enable_mouse", "term_disable_mouse", "term_poll_event", "term_last_key",
        "term_mouse_x", "term_mouse_y", "term_mouse_button", "term_clear", "term_flush", "term_move_cursor", "term_hide_cursor", "term_show_cursor", "term_draw_text",
        "sleep_ms", "tick_ms", "timer_reset", "timer_elapsed_ms", "win_create", "win_destroy", "win_is_open", "win_poll_event", "win_last_key", "win_mouse_x",
        "win_mouse_y", "win_mouse_button", "win_clear", "win_present", "win_set_title", "win_draw_rect", "win_fill_rect", "win_draw_line", "win_draw_pixel", "win_draw_text",
        "image_load", "image_draw", "image_draw_scaled", "image_width", "image_height", "image_free", "video_play", "video_stop", "video_is_running", NULL};
    for (size_t i = 0; builtins[i]; ++i)
        if (strcmp(name, builtins[i]) == 0)
            return 1;
    return 0;
}

/*
 * Function overview: call_name_from_expr_asm
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "call name from expr asm".
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
static int call_name_from_expr_asm(const Expr *callee, char *buf, size_t n)
{
    if (callee->kind == EXPR_IDENTIFIER)
        return copy_cstr(buf, n, callee->as.text);
    if (callee->kind == EXPR_FIELD && callee->as.field.base && callee->as.field.base->kind == EXPR_IDENTIFIER)
        return join_cstr3(buf, n, callee->as.field.base->as.text, "__", callee->as.field.field);
    return 0;
}

static AsmLocal *find_local(AsmFunctionCtx *ctx, const char *name)
{
    for (size_t i = 0; i < ctx->local_count; ++i)
        if (strcmp(ctx->locals[i].name, name) == 0)
            return &ctx->locals[i];
    return NULL;
}

/*
 * Function overview: add_local
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "add local".
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
static int add_local(AsmFunctionCtx *ctx, const char *name, TypeRef type, int is_param, int param_index, int line)
{
    if (!is_scalar_type(type) || type.kind == TYPE_VOID)
    {
        asm_error(ctx->gen, line, 1, "asm backend currently supports only i64/bool/ptr/str local values");
        return 0;
    }
    if (find_local(ctx, name))
    {
        asm_error(ctx->gen, line, 1, "asm backend does not support duplicate or shadowed local name '%s'", name);
        return 0;
    }
    if (ctx->local_count >= sizeof(ctx->locals) / sizeof(ctx->locals[0]))
    {
        asm_error(ctx->gen, line, 1, "too many locals in function '%s' for asm backend", ctx->func->name);
        return 0;
    }
    AsmLocal *local = &ctx->locals[ctx->local_count++];
    memset(local, 0, sizeof(*local));
    copy_cstr(local->name, sizeof(local->name), name);
    local->type = type;
    local->is_param = is_param;
    local->param_index = param_index;
    local->offset = 0;
    return 1;
}

/*
 * Function overview: collect_locals_stmt
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "collect locals stmt".
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
static void collect_locals_stmt(AsmFunctionCtx *ctx, const Statement *s)
{
    if (!s)
        return;
    switch (s->kind)
    {
    case STMT_VAR:
    case STMT_CONST:
        add_local(ctx, s->as.var_stmt.name, s->as.var_stmt.declared_type, 0, -1, s->as.var_stmt.line);
        break;
    case STMT_IF:
        for (size_t i = 0; i < s->as.if_stmt.then_count; ++i)
            collect_locals_stmt(ctx, &s->as.if_stmt.then_block[i]);
        for (size_t i = 0; i < s->as.if_stmt.else_count; ++i)
            collect_locals_stmt(ctx, &s->as.if_stmt.else_block[i]);
        break;
    case STMT_WHILE:
        for (size_t i = 0; i < s->as.while_stmt.body_count; ++i)
            collect_locals_stmt(ctx, &s->as.while_stmt.body[i]);
        break;
    case STMT_UNSAFE:
        for (size_t i = 0; i < s->as.unsafe_stmt.body_count; ++i)
            collect_locals_stmt(ctx, &s->as.unsafe_stmt.body[i]);
        break;
    case STMT_SWITCH:
        asm_error(ctx->gen, s->as.switch_stmt.line, 1, "asm backend does not support switch yet");
        break;
    case STMT_LABEL:
    case STMT_GOTO:
        asm_error(ctx->gen, s->kind == STMT_LABEL ? s->as.label_stmt.line : s->as.goto_stmt.line, 1, "asm backend does not support labels/goto yet");
        break;
    default:
        break;
    }
}

/*
 * Function overview: validate_expr_supported
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "validate expr supported".
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
static int validate_expr_supported(AsmGenerator *g, const Expr *e)
{
    if (!e)
        return 1;
    switch (e->kind)
    {
    case EXPR_INT:
    case EXPR_STRING:
    case EXPR_BOOL:
    case EXPR_IDENTIFIER:
        return 1;
    case EXPR_FLOAT:
        asm_error(g, e->line, e->column, "asm backend does not support f64 literals yet");
        return 0;
    case EXPR_UNARY:
        return validate_expr_supported(g, e->as.unary.operand);
    case EXPR_BINARY:
        if (e->inferred_type.kind == TYPE_F64 || e->inferred_type.kind == TYPE_HP)
        {
            asm_error(g, e->line, e->column, "asm backend does not support floating-point expressions yet");
            return 0;
        }
        return validate_expr_supported(g, e->as.binary.left) && validate_expr_supported(g, e->as.binary.right);
    case EXPR_CALL:
        if (e->as.call.arg_count > 6)
        {
            asm_error(g, e->line, e->column, "asm backend currently supports up to 6 call arguments");
            return 0;
        }
        if (!validate_expr_supported(g, e->as.call.callee))
            return 0;
        for (size_t i = 0; i < e->as.call.arg_count; ++i)
            if (!validate_expr_supported(g, e->as.call.args[i]))
                return 0;
        return 1;
    case EXPR_INDEX:
    case EXPR_ARRAY:
    case EXPR_FIELD:
        asm_error(g, e->line, e->column, "asm backend does not support this expression form yet");
        return 0;
    default:
        return 1;
    }
}

/*
 * Function overview: validate_stmt_supported
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "validate stmt supported".
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
static int validate_stmt_supported(AsmGenerator *g, const Statement *s)
{
    if (!s)
        return 1;
    switch (s->kind)
    {
    case STMT_VAR:
    case STMT_CONST:
        return !s->as.var_stmt.has_initializer || validate_expr_supported(g, s->as.var_stmt.initializer);
    case STMT_ASSIGN:
        return validate_expr_supported(g, s->as.assign_stmt.target) && validate_expr_supported(g, s->as.assign_stmt.value);
    case STMT_PRINT:
        return validate_expr_supported(g, s->as.print_stmt.value);
    case STMT_RETURN:
        return !s->as.return_stmt.value || validate_expr_supported(g, s->as.return_stmt.value);
    case STMT_IF:
        if (!validate_expr_supported(g, s->as.if_stmt.condition))
            return 0;
        for (size_t i = 0; i < s->as.if_stmt.then_count; ++i)
            if (!validate_stmt_supported(g, &s->as.if_stmt.then_block[i]))
                return 0;
        for (size_t i = 0; i < s->as.if_stmt.else_count; ++i)
            if (!validate_stmt_supported(g, &s->as.if_stmt.else_block[i]))
                return 0;
        return 1;
    case STMT_WHILE:
        if (!validate_expr_supported(g, s->as.while_stmt.condition))
            return 0;
        for (size_t i = 0; i < s->as.while_stmt.body_count; ++i)
            if (!validate_stmt_supported(g, &s->as.while_stmt.body[i]))
                return 0;
        return 1;
    case STMT_EXPR:
        return validate_expr_supported(g, s->as.expr_stmt.expr);
    case STMT_UNSAFE:
        for (size_t i = 0; i < s->as.unsafe_stmt.body_count; ++i)
            if (!validate_stmt_supported(g, &s->as.unsafe_stmt.body[i]))
                return 0;
        return 1;
    default:
        asm_error(g, 1, 1, "asm backend does not support this statement kind yet");
        return 0;
    }
}

/*
 * Function overview: validate_program_supported
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "validate program supported".
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
static int validate_program_supported(AsmGenerator *g)
{
    if (g->program->struct_count > 0)
    {
        asm_error(g, 1, 1, "asm backend does not support struct declarations yet");
        return 0;
    }
    if (g->program->global_count > 0)
    {
        asm_error(g, 1, 1, "asm backend does not support global variables yet");
        return 0;
    }
    for (size_t i = 0; i < g->program->function_count; ++i)
    {
        const FunctionDecl *f = &g->program->functions[i];
        if (!is_scalar_type(f->return_type))
        {
            asm_error(g, f->line, 1, "asm backend only supports i64/bool/ptr/str/void function returns right now");
            return 0;
        }
        if (f->param_count > 6)
        {
            asm_error(g, f->line, 1, "asm backend currently supports up to 6 function parameters");
            return 0;
        }
        for (size_t j = 0; j < f->param_count; ++j)
        {
            if (!is_scalar_type(f->params[j].type) || f->params[j].type.kind == TYPE_VOID)
            {
                asm_error(g, f->line, 1, "asm backend only supports scalar parameters right now");
                return 0;
            }
        }
        for (size_t j = 0; j < f->body_count; ++j)
        {
            if (!validate_stmt_supported(g, &f->body[j]))
                return 0;
        }
    }
    return 1;
}

/*
 * Function overview: emit_line
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit line".
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
static void emit_line(FILE *out, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(out, fmt, ap);
    va_end(ap);
}

// Forward declarations
// These declarations break the mutual dependency cycle among the three main
// expression/statement emission routines used throughout the backend.
static void emit_addr_of_expr(AsmFunctionCtx *ctx, const Expr *e);
static void emit_expr_asm(AsmFunctionCtx *ctx, const Expr *e);
static void emit_stmt_asm(AsmFunctionCtx *ctx, const Statement *s);

/*
 * Function overview: emit_addr_of_expr
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit addr of expr".
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

// Forward declarations
// These declarations break the mutual dependency cycle among the three main
// expression/statement emission routines used throughout the backend.
static void emit_addr_of_expr(AsmFunctionCtx *ctx, const Expr *e)
{
    FILE *out = ctx->gen->out;
    if (!e)
    {
        emit_line(out, "    xor rax, rax\n");
        return;
    }
    switch (e->kind)
    {
    case EXPR_IDENTIFIER:
    {
        AsmLocal *local = find_local(ctx, e->as.text);
        if (!local)
        {
            asm_error(ctx->gen, e->line, e->column, "asm backend unknown local '%s'", e->as.text);
            emit_line(out, "    xor rax, rax\n");
            return;
        }
        emit_line(out, "    lea rax, [rbp-%d]\n", local->offset);
        return;
    }
    case EXPR_UNARY:
        if (strcmp(e->as.unary.op, "^") == 0)
        {
            emit_expr_asm(ctx, e->as.unary.operand);
            return;
        }
        break;
    default:
        break;
    }
    asm_error(ctx->gen, e->line, e->column, "asm backend cannot take address of this expression yet");
    emit_line(out, "    xor rax, rax\n");
}

/*
 * Function overview: emit_call_asm
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit call asm".
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
static void emit_call_asm(AsmFunctionCtx *ctx, const Expr *e)
{
    FILE *out = ctx->gen->out;
    char name[128];
    if (!call_name_from_expr_asm(e->as.call.callee, name, sizeof(name)))
    {
        asm_error(ctx->gen, e->line, e->column, "asm backend cannot resolve this call target yet");
        emit_line(out, "    xor rax, rax\n");
        return;
    }
    if (strcmp(name, "addr") == 0)
    {
        emit_addr_of_expr(ctx, e->as.call.args[0]);
        return;
    }
    for (long i = (long)e->as.call.arg_count - 1; i >= 0; --i)
    {
        emit_expr_asm(ctx, e->as.call.args[i]);
        emit_line(out, "    push rax\n");
    }
    for (size_t i = 0; i < e->as.call.arg_count; ++i)
        emit_line(out, "    pop %s\n", asm_arg_regs[i]);
    if (strcmp(name, "len") == 0)
        emit_line(out, "    call machine_len\n");
    else if (strcmp(name, "index") == 0)
        emit_line(out, "    call machine_index_str\n");
    else if (strcmp(name, "hp") == 0)
    {
        asm_error(ctx->gen, e->line, e->column, "asm backend does not support hp() yet");
        emit_line(out, "    xor rax, rax\n");
    }
    else if (strcmp(name, "sqrt") == 0 || strcmp(name, "sin") == 0 || strcmp(name, "cos") == 0 || strcmp(name, "pow") == 0)
    {
        asm_error(ctx->gen, e->line, e->column, "asm backend does not support libm direct calls yet");
        emit_line(out, "    xor rax, rax\n");
    }
    else if (is_machine_builtin_name_asm(name))
        emit_line(out, "    call machine_%s\n", name);
    else
        emit_line(out, "    call %s\n", name);
}

/*
 * Function overview: emit_expr_asm
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit expr asm".
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
static void emit_expr_asm(AsmFunctionCtx *ctx, const Expr *e)
{
    FILE *out = ctx->gen->out;
    if (!e)
    {
        emit_line(out, "    xor rax, rax\n");
        return;
    }
    switch (e->kind)
    {
    case EXPR_INT:
        emit_line(out, "    mov rax, %lld\n", e->as.int_value);
        return;
    case EXPR_BOOL:
        emit_line(out, "    mov rax, %d\n", e->as.bool_value ? 1 : 0);
        return;
    case EXPR_STRING:
    {
        int label = label_for_string_expr(ctx->gen, e);
        emit_line(out, "    lea rax, [.Lstr_%d]\n", label);
        return;
    }
    case EXPR_IDENTIFIER:
    {
        AsmLocal *local = find_local(ctx, e->as.text);
        if (!local)
        {
            asm_error(ctx->gen, e->line, e->column, "asm backend unknown local '%s'", e->as.text);
            emit_line(out, "    xor rax, rax\n");
            return;
        }
        emit_line(out, "    mov rax, [rbp-%d]\n", local->offset);
        return;
    }
    case EXPR_UNARY:
        if (strcmp(e->as.unary.op, "@") == 0)
        {
            emit_addr_of_expr(ctx, e->as.unary.operand);
            return;
        }
        if (strcmp(e->as.unary.op, "^") == 0)
        {
            emit_expr_asm(ctx, e->as.unary.operand);
            emit_line(out, "    mov rax, [rax]\n");
            return;
        }
        if (strcmp(e->as.unary.op, "-") == 0)
        {
            emit_expr_asm(ctx, e->as.unary.operand);
            emit_line(out, "    neg rax\n");
            return;
        }
        if (strcmp(e->as.unary.op, "!") == 0)
        {
            emit_expr_asm(ctx, e->as.unary.operand);
            emit_line(out, "    test rax, rax\n    sete al\n    movzx rax, al\n");
            return;
        }
        asm_error(ctx->gen, e->line, e->column, "asm backend does not support unary operator '%s' yet", e->as.unary.op);
        emit_line(out, "    xor rax, rax\n");
        return;
    case EXPR_BINARY:
        if (strcmp(e->as.binary.op, "+") == 0 && e->inferred_type.kind == TYPE_STR)
        {
            emit_expr_asm(ctx, e->as.binary.right);
            emit_line(out, "    push rax\n");
            emit_expr_asm(ctx, e->as.binary.left);
            emit_line(out, "    pop rsi\n    mov rdi, rax\n    call machine_concat\n");
            return;
        }
        emit_expr_asm(ctx, e->as.binary.left);
        emit_line(out, "    push rax\n");
        emit_expr_asm(ctx, e->as.binary.right);
        emit_line(out, "    pop rcx\n");
        if (strcmp(e->as.binary.op, "+") == 0)
            emit_line(out, "    add rax, rcx\n");
        else if (strcmp(e->as.binary.op, "-") == 0)
            emit_line(out, "    mov rdx, rcx\n    sub rdx, rax\n    mov rax, rdx\n");
        else if (strcmp(e->as.binary.op, "*") == 0)
            emit_line(out, "    imul rax, rcx\n");
        else if (strcmp(e->as.binary.op, "/") == 0)
            emit_line(out, "    mov rbx, rax\n    mov rax, rcx\n    cqo\n    idiv rbx\n");
        else if (strcmp(e->as.binary.op, "%") == 0)
            emit_line(out, "    mov rbx, rax\n    mov rax, rcx\n    cqo\n    idiv rbx\n    mov rax, rdx\n");
        else if (strcmp(e->as.binary.op, "==") == 0)
            emit_line(out, "    cmp rcx, rax\n    sete al\n    movzx rax, al\n");
        else if (strcmp(e->as.binary.op, "!=") == 0)
            emit_line(out, "    cmp rcx, rax\n    setne al\n    movzx rax, al\n");
        else if (strcmp(e->as.binary.op, "<") == 0)
            emit_line(out, "    cmp rcx, rax\n    setl al\n    movzx rax, al\n");
        else if (strcmp(e->as.binary.op, "<=") == 0)
            emit_line(out, "    cmp rcx, rax\n    setle al\n    movzx rax, al\n");
        else if (strcmp(e->as.binary.op, ">") == 0)
            emit_line(out, "    cmp rcx, rax\n    setg al\n    movzx rax, al\n");
        else if (strcmp(e->as.binary.op, ">=") == 0)
            emit_line(out, "    cmp rcx, rax\n    setge al\n    movzx rax, al\n");
        else if (strcmp(e->as.binary.op, "&&") == 0)
            emit_line(out, "    test rcx, rcx\n    setne cl\n    test rax, rax\n    setne al\n    and al, cl\n    movzx rax, al\n");
        else if (strcmp(e->as.binary.op, "||") == 0)
            emit_line(out, "    test rcx, rcx\n    setne cl\n    test rax, rax\n    setne al\n    or al, cl\n    movzx rax, al\n");
        else
        {
            asm_error(ctx->gen, e->line, e->column, "asm backend does not support binary operator '%s' yet", e->as.binary.op);
            emit_line(out, "    xor rax, rax\n");
        }
        return;
    case EXPR_CALL:
        emit_call_asm(ctx, e);
        return;
    default:
        asm_error(ctx->gen, e->line, e->column, "asm backend does not support this expression yet");
        emit_line(out, "    xor rax, rax\n");
        return;
    }
}

/*
 * Function overview: emit_print_call
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit print call".
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
static void emit_print_call(AsmFunctionCtx *ctx, const Expr *e)
{
    FILE *out = ctx->gen->out;
    emit_expr_asm(ctx, e);
    if (ctx->gen->target_id == MACHINE_TARGET_LINUX_HOSTED)
    {
        if (e->inferred_type.kind == TYPE_STR)
            emit_line(out, "    mov rdi, rax\n    call puts\n");
        else if (e->inferred_type.kind == TYPE_I64 || e->inferred_type.kind == TYPE_BOOL || e->inferred_type.kind == TYPE_PTR)
            emit_line(out, "    mov rsi, rax\n    lea rdi, [.Lfmt_i64]\n    xor eax, eax\n    call printf\n");
        else
            asm_error(ctx->gen, e->line, e->column, "asm backend print currently supports i64/bool/ptr/str only");
        return;
    }
    if (e->inferred_type.kind == TYPE_STR)
        emit_line(out, "    mov rdi, rax\n    call machine_print_str\n");
    else if (e->inferred_type.kind == TYPE_I64 || e->inferred_type.kind == TYPE_BOOL || e->inferred_type.kind == TYPE_PTR)
        emit_line(out, "    mov rdi, rax\n    call machine_print_i64\n");
    else
        asm_error(ctx->gen, e->line, e->column, "asm backend print currently supports i64/bool/ptr/str only");
}

/*
 * Function overview: assign_local_offsets
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "assign local offsets".
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
static void assign_local_offsets(AsmFunctionCtx *ctx)
{
    int offset = 0;
    for (size_t i = 0; i < ctx->local_count; ++i)
    {
        offset += stack_slot_bytes(ctx->locals[i].type);
        ctx->locals[i].offset = offset;
    }
    ctx->stack_size = (offset + 15) & ~15;
    if (ctx->stack_size == 0)
        ctx->stack_size = 0;
}

/*
 * Function overview: emit_stmt_asm
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit stmt asm".
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
static void emit_stmt_asm(AsmFunctionCtx *ctx, const Statement *s)
{
    FILE *out = ctx->gen->out;
    if (!s)
        return;
    switch (s->kind)
    {
    case STMT_VAR:
    case STMT_CONST:
    {
        AsmLocal *local = find_local(ctx, s->as.var_stmt.name);
        if (!local)
            return;
        if (s->as.var_stmt.has_initializer)
            emit_expr_asm(ctx, s->as.var_stmt.initializer);
        else
            emit_line(out, "    xor rax, rax\n");
        emit_line(out, "    mov [rbp-%d], rax\n", local->offset);
        break;
    }
    case STMT_ASSIGN:
        if (s->as.assign_stmt.target->kind == EXPR_IDENTIFIER)
        {
            AsmLocal *local = find_local(ctx, s->as.assign_stmt.target->as.text);
            if (!local)
            {
                asm_error(ctx->gen, s->as.assign_stmt.line, 1, "asm backend unknown assignment target '%s'", s->as.assign_stmt.target->as.text);
                break;
            }
            emit_expr_asm(ctx, s->as.assign_stmt.value);
            emit_line(out, "    mov [rbp-%d], rax\n", local->offset);
        }
        else if (s->as.assign_stmt.target->kind == EXPR_UNARY && strcmp(s->as.assign_stmt.target->as.unary.op, "^") == 0)
        {
            emit_expr_asm(ctx, s->as.assign_stmt.target->as.unary.operand);
            emit_line(out, "    push rax\n");
            emit_expr_asm(ctx, s->as.assign_stmt.value);
            emit_line(out, "    pop rcx\n    mov [rcx], rax\n");
        }
        else
            asm_error(ctx->gen, s->as.assign_stmt.line, 1, "asm backend assignment currently supports locals and ^ptr targets only");
        break;
    case STMT_PRINT:
        emit_print_call(ctx, s->as.print_stmt.value);
        break;
    case STMT_RETURN:
        if (s->as.return_stmt.value)
            emit_expr_asm(ctx, s->as.return_stmt.value);
        else
            emit_line(out, "    xor rax, rax\n");
        emit_line(out, "    jmp %s\n", ctx->return_label);
        break;
    case STMT_EXPR:
        emit_expr_asm(ctx, s->as.expr_stmt.expr);
        break;
    case STMT_IF:
    {
        int lid = ctx->next_label_id++;
        emit_expr_asm(ctx, s->as.if_stmt.condition);
        emit_line(out, "    test rax, rax\n    jz .Lelse_%d\n", lid);
        for (size_t i = 0; i < s->as.if_stmt.then_count; ++i)
            emit_stmt_asm(ctx, &s->as.if_stmt.then_block[i]);
        emit_line(out, "    jmp .Lendif_%d\n.Lelse_%d:\n", lid, lid);
        for (size_t i = 0; i < s->as.if_stmt.else_count; ++i)
            emit_stmt_asm(ctx, &s->as.if_stmt.else_block[i]);
        emit_line(out, ".Lendif_%d:\n", lid);
        break;
    }
    case STMT_WHILE:
    {
        int lid = ctx->next_label_id++;
        emit_line(out, ".Lwhile_%d:\n", lid);
        emit_expr_asm(ctx, s->as.while_stmt.condition);
        emit_line(out, "    test rax, rax\n    jz .Lwhile_end_%d\n", lid);
        for (size_t i = 0; i < s->as.while_stmt.body_count; ++i)
            emit_stmt_asm(ctx, &s->as.while_stmt.body[i]);
        emit_line(out, "    jmp .Lwhile_%d\n.Lwhile_end_%d:\n", lid, lid);
        break;
    }
    case STMT_UNSAFE:
        for (size_t i = 0; i < s->as.unsafe_stmt.body_count; ++i)
            emit_stmt_asm(ctx, &s->as.unsafe_stmt.body[i]);
        break;
    default:
        asm_error(ctx->gen, 1, 1, "asm backend encountered unsupported statement during emission");
        break;
    }
}

/*
 * Function overview: emit_function_asm
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "emit function asm".
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
static void emit_function_asm(AsmGenerator *g, const FunctionDecl *f)
{
    AsmFunctionCtx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.gen = g;
    ctx.func = f;
    snprintf(ctx.return_label, sizeof(ctx.return_label), ".Lreturn_%s", f->name);
    ctx.next_label_id = 1;
    for (size_t i = 0; i < f->param_count; ++i)
        add_local(&ctx, f->params[i].name, f->params[i].type, 1, (int)i, f->line);
    for (size_t i = 0; i < f->body_count; ++i)
        collect_locals_stmt(&ctx, &f->body[i]);
    assign_local_offsets(&ctx);
    fprintf(g->out, ".globl %s\n.type %s, @function\n%s:\n", (f->is_main && g->target_id == MACHINE_TARGET_LINUX_HOSTED) ? "main" : (f->is_main ? "machine_user_main" : f->name), (f->is_main && g->target_id == MACHINE_TARGET_LINUX_HOSTED) ? "main" : (f->is_main ? "machine_user_main" : f->name), (f->is_main && g->target_id == MACHINE_TARGET_LINUX_HOSTED) ? "main" : (f->is_main ? "machine_user_main" : f->name));
    emit_line(g->out, "    push rbp\n    mov rbp, rsp\n");
    if (ctx.stack_size > 0)
        emit_line(g->out, "    sub rsp, %d\n", ctx.stack_size);
    for (size_t i = 0; i < f->param_count; ++i)
    {
        AsmLocal *local = find_local(&ctx, f->params[i].name);
        if (local)
            emit_line(g->out, "    mov [rbp-%d], %s\n", local->offset, asm_arg_regs[i]);
    }
    for (size_t i = 0; i < f->body_count; ++i)
        emit_stmt_asm(&ctx, &f->body[i]);
    emit_line(g->out, "    xor rax, rax\n%s:\n", ctx.return_label);
    emit_line(g->out, "    mov rsp, rbp\n    pop rbp\n    ret\n\n");
}

/*
 * Function overview: generate_asm_file
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "generate asm file".
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
bool generate_asm_file(const Program *program, const char *output_path, DiagnosticList *errors)
{
    AsmGenerator g;
    memset(&g, 0, sizeof(g));
    g.program = program;
    g.errors = errors;
    g.target_id = program->target_id;
    FILE *out = fopen(output_path, "w");
    if (!out)
    {
        diagnostics_add(NULL, errors, 1, 1, "failed to open asm output '%s'", output_path);
        return false;
    }
    g.out = out;
    if (!validate_program_supported(&g))
    {
        fclose(out);
        return false;
    }
    for (size_t i = 0; i < program->function_count; ++i)
        for (size_t j = 0; j < program->functions[i].body_count; ++j)
            collect_strings_stmt(&g, &program->functions[i].body[j]);
    fputs(".intel_syntax noprefix\n", out);
    if (g.string_count > 0 || g.target_id == MACHINE_TARGET_LINUX_HOSTED)
    {
        fputs(".section .rodata\n", out);
        if (g.target_id == MACHINE_TARGET_LINUX_HOSTED)
            fputs(".Lfmt_i64:\n    .asciz \"%lld\\n\"\n", out);
        for (size_t i = 0; i < g.string_count; ++i)
        {
            fprintf(out, ".Lstr_%d:\n    .asciz ", g.strings[i].label_id);
            emit_asm_string_escaped(out, g.strings[i].expr->as.text);
            fputc('\n', out);
        }
    }
    fputs("\n.section .text\n", out);
    for (size_t i = 0; i < program->function_count; ++i)
        emit_function_asm(&g, &program->functions[i]);
    fclose(out);
    return !g.failed;
}

/*
 * Function overview: compile_asm_to_binary
 *
 * High-level purpose:
 * - This routine belongs to asm_backend.c.
 * - It exists to generate x86_64 assembly output from the machine ast and coordinate assembly-oriented code emission decisions.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "compile asm to binary".
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
bool compile_asm_to_binary(const char *asm_path, const char *binary_path, const char *source_path, const MachineCompileOptions *options, DiagnosticList *errors)
{
    MachineCompileOptions local = {0};
    local.target = MACHINE_TARGET_LINUX_HOSTED;
    local.backend = MACHINE_BACKEND_X86_64_ASM;
    if (options)
        local = *options;
    char asm_obj[PATH_MAX] = {0};
    if (!append_cstr_suffix(asm_obj, sizeof(asm_obj), binary_path, ".machine_tmp.o"))
    {
        diagnostics_add(NULL, errors, 1, 1, "temporary asm object path is too long");
        return false;
    }
    char *quoted_asm = shell_quote_single(asm_path);
    char *quoted_asm_obj = shell_quote_single(asm_obj);
    char *assemble = format_alloc("cc -c -x assembler %s -o %s", quoted_asm, quoted_asm_obj);
    if (!run_compiler_command(errors, binary_path, "failed to assemble Machine asm output", assemble))
        return false;
    free(assemble);
    if (local.target == MACHINE_TARGET_LINUX_HOSTED)
    {
        RuntimeBundle bundle = {0};
        char runtime_tmp_obj[PATH_MAX] = {0};
        const char *runtime_obj = NULL;
        if (!choose_runtime_bundle(&bundle, source_path, local.prefer_system_runtime))
        {
            diagnostics_add(NULL, errors, 1, 1, "no compatible Machine runtime bundle was found for asm backend");
            remove(asm_obj);
            return false;
        }
        runtime_obj = bundle.object_path;
        if (!file_exists(bundle.object_path))
        {
            if (!append_cstr_suffix(runtime_tmp_obj, sizeof(runtime_tmp_obj), binary_path, ".machine_runtime_tmp.o"))
            {
                diagnostics_add(NULL, errors, 1, 1, "temporary runtime object path is too long");
                remove(asm_obj);
                return false;
            }
            char *qi = shell_quote_single(bundle.include_dir);
            char *qs = shell_quote_single(bundle.runtime_source);
            char *qo = shell_quote_single(runtime_tmp_obj);
            char *cmd = format_alloc("cc -std=gnu17 -Wall -Wextra -Werror -O2 -I%s -c %s -o %s $(pkg-config --cflags sdl2 SDL2_image 2>/dev/null || true)", qi, qs, qo);
            int ran = run_compiler_command(errors, binary_path, "failed to build local Machine runtime for asm backend", cmd);
            free(qi);
            free(qs);
            free(qo);
            free(cmd);
            if (!ran)
            {
                remove(asm_obj);
                return false;
            }
            runtime_obj = runtime_tmp_obj;
        }
        char *qa = shell_quote_single(asm_obj);
        char *qr = shell_quote_single(runtime_obj);
        char *qb = shell_quote_single(binary_path);
        char *cmd = format_alloc("cc -no-pie %s %s -o %s -lm $(pkg-config --cflags --libs sdl2 SDL2_image 2>/dev/null || true)", qa, qr, qb);
        int ran = run_compiler_command(errors, binary_path, "system linker failed while building asm backend output", cmd);
        free(qa);
        free(qr);
        free(qb);
        free(cmd);
        remove(asm_obj);
        if (runtime_tmp_obj[0])
            remove(runtime_tmp_obj);
        return ran;
    }
    if (local.target == MACHINE_TARGET_FREESTANDING_X86_64)
    {
        FreestandingBundle fs = {0};
        if (!choose_freestanding_support_bundle(&fs, source_path))
        {
            diagnostics_add(NULL, errors, 1, 1, "no freestanding support bundle was found for asm backend");
            remove(asm_obj);
            return false;
        }
        char *qa = shell_quote_single(asm_obj);
        char *qi = shell_quote_single(fs.include_dir);
        char *qr = shell_quote_single(fs.runtime_source);
        char *qe = shell_quote_single(fs.entry_source);
        char *qb = shell_quote_single(binary_path);
        char *cmd = format_alloc("cc -std=gnu17 -Wall -Wextra -Wpedantic -Werror -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -no-pie -O2 -I%s %s %s %s -o %s -lgcc", qi, qa, qr, qe, qb);
        int ran = run_compiler_command(errors, binary_path, "freestanding asm backend build failed", cmd);
        free(qa);
        free(qi);
        free(qr);
        free(qe);
        free(qb);
        free(cmd);
        remove(asm_obj);
        return ran;
    }
    if (local.target == MACHINE_TARGET_BAREMETAL_X86_64)
    {
        BaremetalBundle bm = {0};
        if (!choose_baremetal_support_bundle(&bm, source_path))
        {
            diagnostics_add(NULL, errors, 1, 1, "no baremetal support bundle was found for asm backend");
            remove(asm_obj);
            return false;
        }
        char *qa = shell_quote_single(asm_obj);
        char *qi = shell_quote_single(bm.include_dir);
        char *qr = shell_quote_single(bm.runtime_source);
        char *qe = shell_quote_single(bm.entry_source);
        char *ql = shell_quote_single(bm.linker_script);
        char *qb = shell_quote_single(binary_path);
        char *cmd = format_alloc("cc -m64 -Wall -Wextra -Wpedantic -Werror -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -mno-red-zone -mcmodel=kernel -nostdlib -nostartfiles -nodefaultlibs -no-pie -O2 -I%s %s %s %s -T %s -o %s -lgcc", qi, qa, qr, qe, ql, qb);
        int ran = run_compiler_command(errors, binary_path, "baremetal asm backend link failed", cmd);
        free(qa);
        free(qi);
        free(qr);
        free(qe);
        free(ql);
        free(qb);
        free(cmd);
        remove(asm_obj);
        return ran;
    }
    remove(asm_obj);
    diagnostics_add(NULL, errors, 1, 1, "unknown target for asm backend");
    return false;
}

// End-of-file note
// This annotated copy intentionally preserves the original code and existing block
// comments. The additional comments above are single-line comments only, chosen to
// avoid interfering with C parsing if someone studies the file in place.
