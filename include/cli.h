/*
 * Annotated reading edition of cli.h
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
#ifndef MACHINE_CLI_H
/*
 * Header guard / one-time inclusion control.
 *
 * This prevents duplicate declarations when the same header is included multiple times.
 */
#define MACHINE_CLI_H

/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void print_help(void);
/*
 * Function declaration.
 *
 * This prototype describes a service implemented elsewhere and documents how other modules are expected to call it.
 */
void print_version(void);

/*
 * Preprocessor directive.
 *
 * Directives here usually define compile-time constants, feature switches, or version identifiers.
 */
#endif
