#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

typedef enum { SYM_LOCAL, SYM_INPUT, SYM_OUTPUT, SYM_GLOBAL } SymKind;

struct Symbol {
    char *name;
    Type type;
    SymKind kind;
    int line;
    int assigned;
};

struct RecordDef {
    char *name;   /* #book (the original name, not an alias) */
    int is_union;
    int line;
    int nfields;
    char **field_names;
    Type *field_types;
    int state;    /* 0 = unresolved, 1 = resolving (cycle check), 2 = resolved */
    TypeDef *def;
};

typedef struct {
    Function *f;
    VEC(Symbol *) syms;  /* inputs, outputs, then locals */
} FuncInfo;

typedef struct {
    VEC(RecordDef *) records;  /* real definitions in source order (aliases point at these) */
    VEC(Symbol *) globals;
    FuncInfo *funcs;           /* parallel to prog->funcs */
    int errors, warnings;
} Sema;

Sema *sema_check(Program *prog);

int type_eq(Type a, Type b);
const char *type_name(Type t, char *buf);

#endif
