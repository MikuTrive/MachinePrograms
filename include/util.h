/*
 * Annotated reading edition of util.h
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
#ifndef MACHINE_UTIL_H
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_UTIL_H

/*
 * Miscellaneous utility helpers shared by the compiler front-end.
 *
 * This header intentionally contains only small, side-effect-free helpers
 * plus one source preprocessor used before lexing.  The preprocessor removes
 * Machine single-line comments while preserving line numbers, so diagnostics
 * still point to the correct source lines.
 */

/*
 * Dependency include.
 *
 * This brings in declarations required by the current header.
 */
#include <stdbool.h>
/*
 * Dependency include.
 *
 * This brings in declarations required by the current header.
 */
#include <stddef.h>

/* Read a UTF-8 / text file fully into memory. */
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
bool read_text_file(const char *path, char **buffer, size_t *length);

/* Release a buffer returned by read_text_file(). */
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void free_text_file(char *buffer);

/* Simple suffix test used for .mne validation. */
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
bool ends_with(const char *value, const char *suffix);

/* Replace the file extension of a path. */
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void path_change_extension(const char *input, const char *new_ext, char *output, size_t output_size);

/* Extract the directory portion of a path. Returns "." when no separator exists. */
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
bool path_dirname(const char *input, char *output, size_t output_size);

/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
bool file_exists(const char *path);

/* Copy a C string into a fixed-size destination without truncation. */
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
bool copy_cstr(char *dst, size_t dst_size, const char *src);

/* Join two strings with a separator into a fixed-size destination without truncation. */
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
bool join_cstr3(char *dst, size_t dst_size, const char *left, const char *sep, const char *right);

/* Append a suffix into a fixed-size destination without truncation. */
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
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
                               /*
                                * Function declaration.
                                *
                                * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
                                */
                               int *err_col);

/*
 * Preprocessor directive.
 *
 * Directives here usually define compile-time constants, feature switches, or version identifiers.
 */
#endif
