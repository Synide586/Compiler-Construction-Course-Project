/* Generates textual LLVM IR from a type-checked AST. clang then turns the IR into an executable.
   Mapping:
   - int -> i32, real -> double, record #x -> %rec.x (a struct), union #x -> %union.x (8-byte-aligned storage
     whose fields all start at offset 0)
   - every variable lives in stack memory (alloca); globals are LLVM globals; all start at zero
   - input parameters are passed by value; output parameters are passed as pointers that the callee
     fills in when it returns
   - record arithmetic is done field by field with extractvalue / insertvalue */
#include "codegen.h"

#include <stdarg.h>
#include <stdint.h>

typedef struct { char *s; size_t n, cap; } Buf;

static void bprintf(Buf *b, const char *fmt, ...) {
    va_list ap;
    for (;;) {
        va_start(ap, fmt);
        int need = vsnprintf(b->s ? b->s + b->n : NULL, b->s ? b->cap - b->n : 0, fmt, ap);
        va_end(ap);
        if (b->s && b->n + (size_t)need < b->cap) { b->n += need; return; }
        b->cap = (b->cap + need + 1) * 2;
        b->s = realloc(b->s, b->cap);
    }
}

static Buf body;            /* instructions of the current function */
static Buf strings;         /* format-string constants */
static int ntemp, nlabel, nstr;
static Sema *SM;
static Program *PR;
static int cur;             /* index of the function being generated */

typedef struct { char v[64]; Type t; } Val;

static const Type T_REAL = {TY_REAL, NULL};

static void tmp(char *out) { sprintf(out, "%%t%d", ++ntemp); }

static const char *bare(const char *ruid) { return ruid[0] == '#' ? ruid + 1 : ruid; }

static const char *llty(Type t, char *buf) {
    switch (t.kind) {
        case TY_INT: return "i32";
        case TY_REAL: return "double";
        case TY_RECORD: sprintf(buf, "%%rec.%s", bare(t.rec->name)); return buf;
        case TY_UNION: sprintf(buf, "%%union.%s", bare(t.rec->name)); return buf;
        default: return "void";
    }
}

/* ABI size and alignment, needed to size union storage */
static void size_align(Type t, int *size, int *align) {
    if (t.kind == TY_INT) { *size = 4; *align = 4; return; }
    if (t.kind == TY_REAL) { *size = 8; *align = 8; return; }
    int sz = 0, al = 1;
    for (int i = 0; i < t.rec->nfields; i++) {
        int fs, fa;
        size_align(t.rec->field_types[i], &fs, &fa);
        if (fa > al) al = fa;
        if (t.kind == TY_UNION) { if (fs > sz) sz = fs; }
        else { sz = (sz + fa - 1) / fa * fa + fs; }
    }
    if (t.kind == TY_UNION) al = 8;
    *size = (sz + al - 1) / al * al;
    *align = al;
}

static void real_const(double d, char *out) {
    union { double d; uint64_t u; } x = {d};
    sprintf(out, "0x%016llX", (unsigned long long)x.u);  /* exact bit pattern, as LLVM requires */
}

/* returns the name of a private constant holding the string (with a terminating NUL) */
static int string_const(const char *s) {
    int id = ++nstr, len = (int)strlen(s) + 1;
    bprintf(&strings, "@.str.%d = private unnamed_addr constant [%d x i8] c\"", id, len);
    for (const char *p = s; *p; p++) {
        if (*p == '\n') bprintf(&strings, "\\0A");
        else bprintf(&strings, "%c", *p);
    }
    bprintf(&strings, "\\00\"\n");
    return id;
}

/* ------------------------------- variables ---------------------------------- */

static void sym_ptr(Symbol *s, char *out) {
    if (s->kind == SYM_GLOBAL) sprintf(out, "@g.%s", s->name);
    else sprintf(out, "%%v.%s", s->name);
}

/* address of a variable or of a (nested) field; *t receives its type */
static void var_ptr(VarRef *v, char *out, Type *t) {
    char ptr[64], buf[300];
    sym_ptr(v->sym, ptr);
    Type cur_t = v->sym->type;
    for (int i = 0; i < v->field_index.n; i++) {
        int idx = v->field_index.items[i];
        if (cur_t.kind == TY_RECORD) {
            char next[64];
            tmp(next);
            bprintf(&body, "  %s = getelementptr inbounds %s, ptr %s, i32 0, i32 %d\n", next, llty(cur_t, buf), ptr, idx);
            strcpy(ptr, next);
        } /* a union field starts at offset 0: the address stays the same */
        cur_t = cur_t.rec->field_types[idx];
    }
    strcpy(out, ptr);
    *t = cur_t;
}

static Val load_var(VarRef *v) {
    Val r;
    char ptr[64], buf[300];
    var_ptr(v, ptr, &r.t);
    tmp(r.v);
    bprintf(&body, "  %s = load %s, ptr %s\n", r.v, llty(r.t, buf), ptr);
    return r;
}

static Val to_real(Val v) {
    if (v.t.kind != TY_INT) return v;
    Val r = {"", T_REAL};
    tmp(r.v);
    bprintf(&body, "  %s = sitofp i32 %s to double\n", r.v, v.v);
    return r;
}

/* -------------------------------- expressions --------------------------------- */

static void scalar_op(char op, TyKind k, const char *a, const char *b, char *out) {
    const char *ins;
    if (k == TY_REAL) ins = op == '+' ? "fadd" : op == '-' ? "fsub" : op == '*' ? "fmul" : "fdiv";
    else ins = op == '+' ? "add" : op == '-' ? "sub" : op == '*' ? "mul" : "sdiv";
    tmp(out);
    bprintf(&body, "  %s = %s %s %s, %s\n", out, ins, k == TY_REAL ? "double" : "i32", a, b);
}

/* a op b field by field; b is either a record of the same type or a scalar */
static void elementwise(char op, RecordDef *r, const char *a, Val b, int b_is_scalar, char *out) {
    char rty[300], fty[300], acc[64] = "undef";
    Type rt = {TY_RECORD, r};
    llty(rt, rty);
    for (int i = 0; i < r->nfields; i++) {
        Type ft = r->field_types[i];
        char ai[64], res[64];
        tmp(ai);
        bprintf(&body, "  %s = extractvalue %s %s, %d\n", ai, rty, a, i);
        Val bi = b;
        if (!b_is_scalar) {
            bi.t = ft;
            tmp(bi.v);
            bprintf(&body, "  %s = extractvalue %s %s, %d\n", bi.v, rty, b.v, i);
        }
        if (ft.kind == TY_RECORD) {
            elementwise(op, ft.rec, ai, bi, b_is_scalar, res);
        } else {
            if (ft.kind == TY_REAL) bi = to_real(bi);
            scalar_op(op, ft.kind, ai, bi.v, res);
        }
        char next[64];
        tmp(next);
        bprintf(&body, "  %s = insertvalue %s %s, %s %s, %d\n", next, rty, acc, llty(ft, fty), res, i);
        strcpy(acc, next);
    }
    strcpy(out, acc);
}

static Val gen_expr(Expr *e) {
    Val r;
    r.t = e->type;
    switch (e->kind) {
        case E_NUM: sprintf(r.v, "%d", e->ival); return r;
        case E_RNUM: real_const(e->rval, r.v); return r;
        case E_VAR: return load_var(&e->var);
        case E_BIN: break;
    }
    Val l = gen_expr(e->l), rr = gen_expr(e->r);
    if (e->type.kind == TY_INT || e->type.kind == TY_REAL) {
        if (e->type.kind == TY_REAL) { l = to_real(l); rr = to_real(rr); }
        scalar_op(e->op, e->type.kind, l.v, rr.v, r.v);
    } else if (l.t.kind == TY_RECORD && rr.t.kind == TY_RECORD) {
        elementwise(e->op, e->type.rec, l.v, rr, 0, r.v);
    } else if (l.t.kind == TY_RECORD) {
        elementwise(e->op, e->type.rec, l.v, rr, 1, r.v);
    } else {  /* scalar * record */
        elementwise(e->op, e->type.rec, rr.v, l, 1, r.v);
    }
    return r;
}

static void gen_bool(BoolExpr *b, char *out) {
    char x[64], y[64];
    switch (b->kind) {
        case B_REL: {
            Val l = gen_expr(b->lv), r = gen_expr(b->rv);
            int real = l.t.kind == TY_REAL || r.t.kind == TY_REAL;
            const char *pred;
            switch (b->relop) {
                case TK_LT: pred = real ? "olt" : "slt"; break;
                case TK_LE: pred = real ? "ole" : "sle"; break;
                case TK_EQ: pred = real ? "oeq" : "eq"; break;
                case TK_GT: pred = real ? "ogt" : "sgt"; break;
                case TK_GE: pred = real ? "oge" : "sge"; break;
                default: pred = real ? "une" : "ne"; break;
            }
            if (real) { l = to_real(l); r = to_real(r); }
            tmp(out);
            bprintf(&body, "  %s = %s %s %s %s, %s\n", out, real ? "fcmp" : "icmp", pred, real ? "double" : "i32", l.v, r.v);
            return;
        }
        case B_NOT:
            gen_bool(b->l, x);
            tmp(out);
            bprintf(&body, "  %s = xor i1 %s, true\n", out, x);
            return;
        default:
            gen_bool(b->l, x);
            gen_bool(b->r, y);
            tmp(out);
            bprintf(&body, "  %s = %s i1 %s, %s\n", out, b->kind == B_AND ? "and" : "or", x, y);
    }
}

/* -------------------------------- statements ----------------------------------- */

/* collects printf format pieces and arguments for every field of a record */
static void flatten(Val v, Buf *fmt, Buf *args) {
    if (v.t.kind == TY_INT) { bprintf(fmt, "%s%%d", fmt->n ? " " : ""); bprintf(args, ", i32 %s", v.v); return; }
    if (v.t.kind == TY_REAL) { bprintf(fmt, "%s%%.2f", fmt->n ? " " : ""); bprintf(args, ", double %s", v.v); return; }
    char rty[300];
    llty(v.t, rty);
    for (int i = 0; i < v.t.rec->nfields; i++) {
        Val f = {"", v.t.rec->field_types[i]};
        tmp(f.v);
        bprintf(&body, "  %s = extractvalue %s %s, %d\n", f.v, rty, v.v, i);
        flatten(f, fmt, args);
    }
}

static void gen_stmts(StmtList *l);

static void gen_call(Stmt *s) {
    int j = 0;
    while (strcmp(PR->funcs.items[j]->name, s->fun) != 0) j++;
    Function *callee = PR->funcs.items[j];
    FuncInfo *ci = &SM->funcs[j];
    Buf args = {0};
    char buf[300], ptr[64];
    for (int k = 0; k < s->ins.n; k++) {
        VarRef v = {0};
        v.id = s->ins.items[k].name;
        v.sym = NULL;
        for (int q = 0; q < SM->funcs[cur].syms.n && !v.sym; q++)
            if (strcmp(SM->funcs[cur].syms.items[q]->name, v.id) == 0) v.sym = SM->funcs[cur].syms.items[q];
        for (int q = 0; q < SM->globals.n && !v.sym; q++)
            if (strcmp(SM->globals.items[q]->name, v.id) == 0) v.sym = SM->globals.items[q];
        Val a = load_var(&v);
        Type pt = ci->syms.items[k]->type;
        if (pt.kind == TY_REAL) a = to_real(a);
        bprintf(&args, "%s%s %s", k ? ", " : "", llty(pt, buf), a.v);
    }
    for (int k = 0; k < s->outs.n; k++) {
        Symbol *sym = NULL;
        for (int q = 0; q < SM->funcs[cur].syms.n && !sym; q++)
            if (strcmp(SM->funcs[cur].syms.items[q]->name, s->outs.items[k].name) == 0) sym = SM->funcs[cur].syms.items[q];
        for (int q = 0; q < SM->globals.n && !sym; q++)
            if (strcmp(SM->globals.items[q]->name, s->outs.items[k].name) == 0) sym = SM->globals.items[q];
        sym_ptr(sym, ptr);
        bprintf(&args, "%sptr %s", (s->ins.n + k) ? ", " : "", ptr);
    }
    bprintf(&body, "  call void @fn.%s(%s)\n", callee->name, args.s ? args.s : "");
    free(args.s);
}

static void gen_stmt(Stmt *s) {
    char buf[300], ptr[64], c[64];
    switch (s->kind) {
        case S_ASSIGN: {
            Val v = gen_expr(s->expr);
            Type t;
            var_ptr(&s->target, ptr, &t);
            if (t.kind == TY_REAL) v = to_real(v);
            bprintf(&body, "  store %s %s, ptr %s\n", llty(t, buf), v.v, ptr);
            break;
        }
        case S_READ: {
            Type t;
            var_ptr(&s->target, ptr, &t);
            int fmt = string_const(t.kind == TY_INT ? "%d" : "%lf");
            tmp(c);
            bprintf(&body, "  %s = call i32 (ptr, ...) @scanf(ptr @.str.%d, ptr %s)\n", c, fmt, ptr);
            break;
        }
        case S_WRITE: {
            Val v = gen_expr(s->expr);
            Buf fmt = {0}, args = {0};
            flatten(v, &fmt, &args);
            bprintf(&fmt, "\n");
            int id = string_const(fmt.s);
            tmp(c);
            bprintf(&body, "  %s = call i32 (ptr, ...) @printf(ptr @.str.%d%s)\n", c, id, args.s);
            free(fmt.s);
            free(args.s);
            break;
        }
        case S_CALL: gen_call(s); break;
        case S_WHILE: {
            int n = ++nlabel;
            bprintf(&body, "  br label %%while.cond.%d\nwhile.cond.%d:\n", n, n);
            gen_bool(s->cond, c);
            bprintf(&body, "  br i1 %s, label %%while.body.%d, label %%while.end.%d\nwhile.body.%d:\n", c, n, n, n);
            gen_stmts(&s->body);
            bprintf(&body, "  br label %%while.cond.%d\nwhile.end.%d:\n", n, n);
            break;
        }
        case S_IF: {
            int n = ++nlabel;
            gen_bool(s->cond, c);
            bprintf(&body, "  br i1 %s, label %%if.then.%d, label %%if.%s.%d\nif.then.%d:\n", c, n,
                    s->has_else ? "else" : "end", n, n);
            gen_stmts(&s->body);
            bprintf(&body, "  br label %%if.end.%d\n", n);
            if (s->has_else) {
                bprintf(&body, "if.else.%d:\n", n);
                gen_stmts(&s->els);
                bprintf(&body, "  br label %%if.end.%d\n", n);
            }
            bprintf(&body, "if.end.%d:\n", n);
            break;
        }
    }
}

static void gen_stmts(StmtList *l) {
    for (int i = 0; i < l->n; i++) gen_stmt(l->items[i]);
}

/* --------------------------------- functions ----------------------------------- */

static void gen_function(int i, FILE *out) {
    char buf[300], ptr[64];
    cur = i;
    FuncInfo *fi = &SM->funcs[i];
    Function *f = fi->f;
    body.n = 0;
    ntemp = nlabel = 0;

    if (f->is_main) {
        fprintf(out, "define i32 @main() {\nentry:\n");
    } else {
        fprintf(out, "define void @fn.%s(", f->name);
        for (int k = 0; k < f->ins.n; k++)
            fprintf(out, "%s%s %%a.%s", k ? ", " : "", llty(fi->syms.items[k]->type, buf), f->ins.items[k].id);
        for (int k = 0; k < f->outs.n; k++) fprintf(out, "%sptr %%o.%s", (f->ins.n + k) ? ", " : "", f->outs.items[k].id);
        fprintf(out, ") {\nentry:\n");
    }
    /* stack slots for parameters and locals: inputs start with the passed value, everything else with zero */
    for (int k = 0; k < fi->syms.n; k++) {
        Symbol *s = fi->syms.items[k];
        const char *ty = llty(s->type, buf);
        fprintf(out, "  %%v.%s = alloca %s, align 8\n", s->name, ty);
        if (s->kind == SYM_INPUT) fprintf(out, "  store %s %%a.%s, ptr %%v.%s\n", ty, s->name, s->name);
        else fprintf(out, "  store %s zeroinitializer, ptr %%v.%s\n", ty, s->name);
    }
    gen_stmts(&f->stmts);
    if (f->is_main) {
        bprintf(&body, "  ret i32 0\n");
    } else {
        /* return [x, y]: copy each returned variable into the caller's output slot */
        for (int k = 0; k < f->ret.n; k++) {
            VarRef v = {0};
            v.id = f->ret.items[k].name;
            Symbol *sym = NULL;
            for (int q = 0; q < fi->syms.n && !sym; q++)
                if (strcmp(fi->syms.items[q]->name, v.id) == 0) sym = fi->syms.items[q];
            for (int q = 0; q < SM->globals.n && !sym; q++)
                if (strcmp(SM->globals.items[q]->name, v.id) == 0) sym = SM->globals.items[q];
            v.sym = sym;
            Val val = load_var(&v);
            sprintf(ptr, "%%o.%s", f->outs.items[k].id);
            bprintf(&body, "  store %s %s, ptr %s\n", llty(val.t, buf), val.v, ptr);
        }
        bprintf(&body, "  ret void\n");
    }
    fwrite(body.s, 1, body.n, out);
    fprintf(out, "}\n\n");
}

int codegen_emit(Program *prog, Sema *sema, const char *source_name, FILE *out) {
    char buf[300], buf2[300];
    SM = sema;
    PR = prog;
    strings.n = 0;
    nstr = 0;

    fprintf(out, "; LLVM IR generated from %s\n\n", source_name);
    for (int i = 0; i < sema->records.n; i++) {
        RecordDef *r = sema->records.items[i];
        Type t = {r->is_union ? TY_UNION : TY_RECORD, r};
        if (r->is_union) {
            int size, align;
            size_align(t, &size, &align);
            fprintf(out, "%s = type { [%d x i64] }\n", llty(t, buf), size / 8 > 0 ? size / 8 : 1);
            continue;
        }
        fprintf(out, "%s = type { ", llty(t, buf));
        for (int k = 0; k < r->nfields; k++) fprintf(out, "%s%s", k ? ", " : "", llty(r->field_types[k], buf2));
        fprintf(out, " }\n");
    }
    fprintf(out, "\n");
    for (int i = 0; i < sema->globals.n; i++) {
        Symbol *g = sema->globals.items[i];
        fprintf(out, "@g.%s = global %s zeroinitializer, align 8\n", g->name, llty(g->type, buf));
    }
    fprintf(out, "\ndeclare i32 @printf(ptr, ...)\ndeclare i32 @scanf(ptr, ...)\n\n");
    for (int i = 0; i < prog->funcs.n; i++) gen_function(i, out);
    if (strings.n) fwrite(strings.s, 1, strings.n, out);
    return 0;
}
