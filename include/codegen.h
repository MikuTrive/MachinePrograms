#ifndef MACHINE_CODEGEN_H
#define MACHINE_CODEGEN_H

/* Code generation interface. */

#include "common.h"
#include "parser.h"

bool generate_c_file(const Program *program, const char *output_path, DiagnosticList *errors);
bool compile_c_to_binary(const char *c_path, const char *binary_path, DiagnosticList *errors);

#endif
