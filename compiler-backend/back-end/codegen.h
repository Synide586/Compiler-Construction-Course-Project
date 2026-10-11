#ifndef CODEGEN_H
#define CODEGEN_H

#include "semantic.h"

/* Writes the LLVM IR for a type-checked program (sema->errors must be 0). */
int codegen_emit(Program *prog, Sema *sema, const char *source_name, FILE *out);

#endif
