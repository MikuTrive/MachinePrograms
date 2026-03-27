/*
 * Annotated reading edition of version.h
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
#ifndef MACHINE_VERSION_H
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_VERSION_H

/* Public language version.  This stays fixed until the user explicitly changes it. */
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_LANG_VERSION "2.0"

/* Internal source / compiler iteration used during development. */
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_ITERATION_VERSION "1.21"

/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_COMPILER_NAME "machine"

/*
 * Preprocessor directive.
 *
 * Directives here usually define compile-time constants, feature switches, or version identifiers.
 */
#endif
