#ifndef AST_H
#define AST_H

#include "parserDef.h"

#define VEC(T) struct { T *items; int n, cap; }
#define VEC_PUSH(v, x)                                                           \
    do {                                                                         \
        if ((v).n == (v).cap) {                                                  \
            (v).cap = (v).cap ? 2 * (v).cap : 4;                                 \
            (v).items = realloc((v).items, (size_t)(v).cap * sizeof(*(v).items)); \
        }                                                                        \
        (v).items[(v).n++] = (x);                                                \
    } while (0)

/* ---------- semantic types (filled in by the type checker) ---------- */
typedef struct RecordDef RecordDef;
typedef enum { TY_ERROR, TY_INT, TY_REAL, TY_RECORD, TY_UNION } TyKind;
typedef struct { TyKind kind; RecordDef *rec; } Type;
typedef struct Symbol Symbol;

/* ---------- syntax ---------- */
typedef enum { TS_INT, TS_REAL, TS_RECORD, TS_UNION, TS_NAMED } TypeSpecKind;  /* TS_NAMED: bare #name */
typedef struct { TypeSpecKind kind; char *ruid; int line; } TypeSpec;

typedef struct { char *name; int line; } Ident;
typedef VEC(Ident) IdList;

typedef struct {
    char *id;
    VEC(char *) fields;   /* b2.s.x -> fields = [s, x] */
    int line;
    Symbol *sym;          /* resolved by the type checker */
    VEC(int) field_index; /* position of each field inside its record or union */
} VarRef;

typedef enum { E_NUM, E_RNUM, E_VAR, E_BIN } ExprKind;
typedef struct Expr {
    ExprKind kind;
    int line;
    int ival;
    double rval;
    VarRef var;
    char op;  /* + - * / */
    struct Expr *l, *r;
    Type type;
} Expr;

typedef enum { B_REL, B_AND, B_OR, B_NOT } BoolKind;
typedef struct BoolExpr {
    BoolKind kind;
    int line;
    TokenType relop;
    Expr *lv, *rv;
    struct BoolExpr *l, *r;
} BoolExpr;

typedef enum { S_ASSIGN, S_WHILE, S_IF, S_READ, S_WRITE, S_CALL } StmtKind;
typedef struct Stmt Stmt;
typedef VEC(Stmt *) StmtList;
struct Stmt {
    StmtKind kind;
    int line;
    VarRef target;   /* assignment and read target */
    Expr *expr;      /* assignment value, write operand */
    BoolExpr *cond;
    StmtList body, els;
    int has_else;
    char *fun;
    IdList outs, ins;
};

typedef struct { TypeSpec type; char *name; int line; } FieldDef;
typedef struct { int is_union; char *ruid; VEC(FieldDef) fields; int line; } TypeDef;
typedef struct { int is_union; char *from, *to; int line; } DefineType;
typedef struct { TypeSpec type; char *id; int global; int line; } Decl;
typedef struct { TypeSpec type; char *id; int line; } Param;
typedef VEC(Param) ParamList;

typedef struct Function {
    char *name;
    int is_main;
    int line;
    ParamList ins, outs;
    VEC(TypeDef) typedefs;
    VEC(DefineType) aliases;
    VEC(Decl) decls;
    StmtList stmts;
    int has_return_list;
    IdList ret;
    int ret_line;
} Function;

typedef struct { VEC(Function *) funcs; } Program;  /* the last function is _main */

Program *build_ast(parseTreeNode *root);
void ast_print(const Program *p, FILE *out);

#endif
