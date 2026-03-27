/*
 * Annotated reading copy of util.c
 *
 * What this file is for:
 * - Provide generic utility helpers for strings, files, memory management, and other reusable compiler support tasks.
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

#include "util.h"

/*
 * Compiler utility implementation.
 *
 * This file contains low-level helpers only.  There is deliberately no parser
 * or lexer knowledge here except for the comment preprocessor.  Keeping the
 * preprocessor here makes the lexer smaller and lets us strip Machine line comments in one place.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* we implement a set of utility functions for string manipulation,
 *   dynamic memory allocation, list and array management, grid handling,
 *   timing, terminal input/output, and window management.
 *   these functions provide the core functionality needed for
 *   the runtime environment of our programming language,
 *   allowing us to perform common operations such as string duplication,
 *   concatenation, array resizing, grid indexing, timing measurements, and terminal interactions. */
/*
 * Function overview: read_text_file
 *
 * High-level purpose:
 * - This routine belongs to util.c.
 * - It exists to provide generic utility helpers for strings, files, memory management, and other reusable compiler support tasks.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "read text file".
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
bool read_text_file(const char *path, char **buffer, size_t *length)
{
    FILE *fp = fopen(path, "rb");
    if (!fp)
    {
        return false;
    }

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        return false;
    }

    long size = ftell(fp);
    if (size < 0)
    {
        fclose(fp);
        return false;
    }
    rewind(fp);

    char *data = (char *)malloc((size_t)size + 1);
    if (!data)
    {
        fclose(fp);
        return false;
    }

    size_t read_count = fread(data, 1, (size_t)size, fp);
    fclose(fp);
    data[read_count] = '\0';

    *buffer = data;
    if (length)
    {
        *length = read_count;
    }
    return true;
}

/*
 * Function overview: free_text_file
 *
 * High-level purpose:
 * - This routine belongs to util.c.
 * - It exists to provide generic utility helpers for strings, files, memory management, and other reusable compiler support tasks.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "free text file".
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
void free_text_file(char *buffer)
{
    free(buffer);
}

/*
 * Function overview: copy_cstr
 *
 * High-level purpose:
 * - This routine belongs to util.c.
 * - It exists to provide generic utility helpers for strings, files, memory management, and other reusable compiler support tasks.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "copy cstr".
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
bool copy_cstr(char *dst, size_t dst_size, const char *src)
{
    size_t len;
    if (!dst || dst_size == 0 || !src)
    {
        return false;
    }
    len = strlen(src);
    if (len >= dst_size)
    {
        return false;
    }
    memcpy(dst, src, len + 1);
    return true;
}

/* we implement a function to read the contents of a text file into a
 *   dynamically allocated buffer, along with a function to free that buffer.
 *   we also implement functions for copying C strings with size checks,
 *   concatenating strings with separators, checking string suffixes, and changing file extensions.
 *   these functions provide essential utilities for
 *   handling file input/output and string manipulation in our programming language. */
/*
 * Function overview: join_cstr3
 *
 * High-level purpose:
 * - This routine belongs to util.c.
 * - It exists to provide generic utility helpers for strings, files, memory management, and other reusable compiler support tasks.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "join cstr3".
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
bool join_cstr3(char *dst, size_t dst_size, const char *left, const char *sep, const char *right)
{
    size_t left_len;
    size_t sep_len;
    size_t right_len;
    if (!dst || dst_size == 0 || !left || !sep || !right)
    {
        return false;
    }
    left_len = strlen(left);
    sep_len = strlen(sep);
    right_len = strlen(right);
    if (left_len + sep_len + right_len >= dst_size)
    {
        return false;
    }
    memcpy(dst, left, left_len);
    memcpy(dst + left_len, sep, sep_len);
    memcpy(dst + left_len + sep_len, right, right_len + 1);
    return true;
}

/*
 * Function overview: append_cstr_suffix
 *
 * High-level purpose:
 * - This routine belongs to util.c.
 * - It exists to provide generic utility helpers for strings, files, memory management, and other reusable compiler support tasks.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "append cstr suffix".
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
bool append_cstr_suffix(char *dst, size_t dst_size, const char *base, const char *suffix)
{
    return join_cstr3(dst, dst_size, base, "", suffix);
}

/*
 * Function overview: ends_with
 *
 * High-level purpose:
 * - This routine belongs to util.c.
 * - It exists to provide generic utility helpers for strings, files, memory management, and other reusable compiler support tasks.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "ends with".
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
bool ends_with(const char *value, const char *suffix)
{
    size_t value_len = strlen(value);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > value_len)
    {
        return false;
    }
    return strcmp(value + value_len - suffix_len, suffix) == 0;
}

/*
 * Function overview: path_change_extension
 *
 * High-level purpose:
 * - This routine belongs to util.c.
 * - It exists to provide generic utility helpers for strings, files, memory management, and other reusable compiler support tasks.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "path change extension".
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
void path_change_extension(const char *input, const char *new_ext, char *output, size_t output_size)
{
    const char *dot = strrchr(input, '.');
    size_t base_len = dot ? (size_t)(dot - input) : strlen(input);
    if (output_size == 0)
    {
        return;
    }
    if (base_len + strlen(new_ext) >= output_size)
    {
        output[0] = '\0';
        return;
    }
    memcpy(output, input, base_len);
    memcpy(output + base_len, new_ext, strlen(new_ext) + 1);
    /* we implement a function to join three C strings with a separator,
     *       a function to append a suffix to a base string, a function to check if a
     *       string ends with a given suffix, and a function to change the file extension of a path.
     *       these functions provide essential utilities for string manipulation and
     *       file path handling in our programming language. */
}

/*
 * Function overview: path_dirname
 *
 * High-level purpose:
 * - This routine belongs to util.c.
 * - It exists to provide generic utility helpers for strings, files, memory management, and other reusable compiler support tasks.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "path dirname".
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
bool path_dirname(const char *input, char *output, size_t output_size)
{
    const char *slash;
    size_t len;
    if (!input || !output || output_size == 0)
    {
        return false;
    }
    slash = strrchr(input, '/');
    if (!slash)
    {
        if (output_size < 2)
            return false;
        output[0] = '.';
        output[1] = '\0';
        return true;
    }
    if (slash == input)
    {
        if (output_size < 2)
            return false;
        output[0] = '/';
        output[1] = '\0';
        return true;
    }
    len = (size_t)(slash - input);
    if (len >= output_size)
        return false;
    memcpy(output, input, len);
    output[len] = '\0';
    return true;
}

/*
 * Function overview: preprocess_machine_source
 *
 * High-level purpose:
 * - This routine belongs to util.c.
 * - It exists to provide generic utility helpers for strings, files, memory management, and other reusable compiler support tasks.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "preprocess machine source".
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
bool preprocess_machine_source(const char *input,
                               char **output,
                               int *err_line,
                               int *err_col)
{
    size_t n = strlen(input);
    char *out = (char *)malloc(n + 1);
    if (!out)
    {
        return false;
    }
    /* we implement a preprocessor function to strip Machine line comments (which start with -- and
     *       continue to the end of the line) from the source code.
     *       the preprocessor also handles string literals,
     *       ensuring that comment markers inside strings are not treated as actual comments.
     *       this allows us to clean the source code before tokenization and parsing,
     *       making it easier to analyze the structure of the program without being affected by comments. */

    size_t i = 0;
    size_t o = 0;
    while (input[i])
    {
        /* Triple-quoted strings keep everything literally until closing triple quotes. */
        if (input[i] == '"' && input[i + 1] == '"' && input[i + 2] == '"')
        {
            out[o++] = input[i++];
            out[o++] = input[i++];
            out[o++] = input[i++];
            while (input[i])
            {
                if (input[i] == '"' && input[i + 1] == '"' && input[i + 2] == '"')
                {
                    out[o++] = input[i++];
                    out[o++] = input[i++];
                    out[o++] = input[i++];
                    break;
                }
                out[o++] = input[i++];
            }
            continue;
        }

        /* Normal strings also suppress comment detection. */
        if (input[i] == '"')
        {
            out[o++] = input[i++];
            while (input[i])
            {
                out[o++] = input[i];
                if (input[i] == '\\' && input[i + 1])
                {
                    ++i;
                    out[o++] = input[i];
                    ++i;
                    continue;
                }
                if (input[i] == '"')
                {
                    ++i;
                    break;
                }
                ++i;
            }
            continue;
        }

        /* Machine comments begin with -- and continue to end-of-line. */
        if (input[i] == '-' && input[i + 1] == '-')
        {
            out[o++] = ' ';
            out[o++] = ' ';
            i += 2;
            while (input[i] && input[i] != '\n')
            {
                out[o++] = ' ';
                ++i;
                /* we replace comment characters with spaces to preserve line and column information
                 *                for error reporting, while effectively removing the comments from the source code. */
            }
            continue;
        }

        out[o++] = input[i++];
    }

    out[o] = '\0';
    *output = out;
    if (err_line)
        *err_line = 0;
    if (err_col)
        *err_col = 0;
    return true;
}

/*
 * Function overview: file_exists
 *
 * High-level purpose:
 * - This routine belongs to util.c.
 * - It exists to provide generic utility helpers for strings, files, memory management, and other reusable compiler support tasks.
 * - Within that larger job, this specific function handles the step suggested by its name:
 *   "file exists".
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
bool file_exists(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp)
        return false;
    fclose(fp);
    return true;
}
