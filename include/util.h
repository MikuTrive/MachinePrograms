#ifndef MACHINE_UTIL_H
#define MACHINE_UTIL_H

/*
 * Miscellaneous utility helpers shared by the compiler front-end.
 *
 * This header intentionally contains only small, side-effect-free helpers
 * plus one source preprocessor used before lexing.  The preprocessor removes
 * Machine single-line comments while preserving line numbers, so diagnostics
 * still point to the correct source lines.
 */

#include <stdbool.h>
#include <stddef.h>

/* Read a UTF-8 / text file fully into memory. */
bool read_text_file(const char *path, char **buffer, size_t *length);

/* Release a buffer returned by read_text_file(). */
void free_text_file(char *buffer);

/* Simple suffix test used for .mne validation. */
bool ends_with(const char *value, const char *suffix);

/* Replace the file extension of a path. */
void path_change_extension(const char *input, const char *new_ext, char *output, size_t output_size);
bool file_exists(const char *path);

/* Copy a C string into a fixed-size destination without truncation. */
bool copy_cstr(char *dst, size_t dst_size, const char *src);

/* Join two strings with a separator into a fixed-size destination without truncation. */
bool join_cstr3(char *dst, size_t dst_size, const char *left, const char *sep, const char *right);

/* Append a suffix into a fixed-size destination without truncation. */
bool append_cstr_suffix(char *dst, size_t dst_size, const char *base, const char *suffix);

/*
 * Remove Machine single-line comments that start with '--'.
 *
 * Everything from '--' to the end of the line is ignored, except when the
 * marker appears inside a normal string or a triple-quoted multiline string.
 * The function preserves newlines so later diagnostics still report correct
 * line numbers. err_line / err_col are reserved for future preprocess errors.
 */
bool preprocess_machine_source(const char *input,
                               char **output,
                               int *err_line,
                               int *err_col);

#endif
