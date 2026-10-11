/* Semantic analysis: resolves record/union types and variables, and type-checks every statement.
   Type rules:
   - int and real mix freely in arithmetic and comparisons; an int is promoted to real (as in C)
   - an int value may be assigned to a real variable, not the other way round
   - records of the same type can be added and subtracted field by field; a record can be multiplied
     by a scalar (either side) or divided by one (right side only)
   - record and union types are visible in the whole program, wherever they are defined
   - a function must be defined before it is called, and cannot call itself */
#include "semantic.h"

#include <stdarg.h>

static Sema *S;
static Program *P;
static int cur_func;  /* index of the function being checked */

static void error(int line, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "Line %d\tSemantic error: ", line);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    S->errors++;
}

static void warning(int line, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "Line %d\tWarning: ", line);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    S->warnings++;
}

static const Type ERR = {TY_ERROR, NULL};

int type_eq(Type a, Type b) {
    if (a.kind != b.kind) return 0;
    if (a.kind == TY_RECORD || a.kind == TY_UNION) return a.rec == b.rec;
    return 1;
}

const char *type_name(Type t, char *buf) {
    switch (t.kind) {
        case TY_INT: return "int";
        case TY_REAL: return "real";
        case TY_RECORD: sprintf(buf, "record %s", t.rec->name); return buf;
        case TY_UNION: sprintf(buf, "union %s", t.rec->name); return buf;
        default: return "<error>";
    }
}

static int is_num(Type t) { return t.kind == TY_INT || t.kind == TY_REAL; }

static int assignable(Type dst, Type src) {
    return type_eq(dst, src) || (dst.kind == TY_REAL && src.kind == TY_INT);
}

/* ----------------------------- type names -------------------------------- */

typedef struct { char *name; RecordDef *rec; } TypeName;
static VEC(TypeName) tnames;

static RecordDef *find_type(const char *name) {
    for (int i = 0; i < tnames.n; i++)
        if (strcmp(tnames.items[i].name, name) == 0) return tnames.items[i].rec;
    return NULL;
}

static Type resolve_typespec(const TypeSpec *ts) {
    if (ts->kind == TS_INT) return (Type){TY_INT, NULL};
    if (ts->kind == TS_REAL) return (Type){TY_REAL, NULL};
    RecordDef *r = find_type(ts->ruid);
    if (!r) {
        error(ts->line, "type %s is not defined", ts->ruid);
        return ERR;
    }
    if (ts->kind == TS_RECORD && r->is_union) { error(ts->line, "%s is a union, not a record", ts->ruid); return ERR; }
    if (ts->kind == TS_UNION && !r->is_union) { error(ts->line, "%s is a record, not a union", ts->ruid); return ERR; }
    return (Type){r->is_union ? TY_UNION : TY_RECORD, r};
}

static void resolve_record(RecordDef *r) {
    if (r->state == 2) return;
    if (r->state == 1) {
        error(r->line, "%s %s contains itself", r->is_union ? "union" : "record", r->name);
        return;
    }
    r->state = 1;
    for (int i = 0; i < r->nfields; i++) {
        FieldDef *fd = &r->def->fields.items[i];
        for (int j = 0; j < i; j++)
            if (strcmp(r->field_names[j], fd->name) == 0)
                error(fd->line, "field %s is defined twice in %s", fd->name, r->name);
        Type t = resolve_typespec(&fd->type);
        if (t.rec) resolve_record(t.rec);
        r->field_types[i] = t;
    }
    r->state = 2;
}

static void collect_types(void) {
    for (int i = 0; i < P->funcs.n; i++) {
        Function *f = P->funcs.items[i];
        for (int j = 0; j < f->typedefs.n; j++) {
            TypeDef *td = &f->typedefs.items[j];
            if (find_type(td->ruid)) {
                error(td->line, "type %s is defined more than once", td->ruid);
                continue;
            }
            RecordDef *r = calloc(1, sizeof *r);
            r->name = td->ruid;
            r->is_union = td->is_union;
            r->line = td->line;
            r->def = td;
            r->nfields = td->fields.n;
            r->field_names = calloc(r->nfields, sizeof(char *));
            r->field_types = calloc(r->nfields, sizeof(Type));
            for (int k = 0; k < r->nfields; k++) r->field_names[k] = td->fields.items[k].name;
            VEC_PUSH(S->records, r);
            TypeName tn = {td->ruid, r};
            VEC_PUSH(tnames, tn);
        }
    }
    /* definetype aliases may refer to types defined anywhere, including later in the file */
    for (int i = 0; i < P->funcs.n; i++) {
        Function *f = P->funcs.items[i];
        for (int j = 0; j < f->aliases.n; j++) {
            DefineType *d = &f->aliases.items[j];
            RecordDef *r = find_type(d->from);
            if (!r) { error(d->line, "definetype refers to undefined type %s", d->from); continue; }
            if (r->is_union != d->is_union) {
                error(d->line, "definetype says %s is a %s, but it is a %s", d->from, d->is_union ? "union" : "record",
                      r->is_union ? "union" : "record");
                continue;
            }
            if (find_type(d->to)) { error(d->line, "type name %s is already in use", d->to); continue; }
            TypeName tn = {d->to, r};
            VEC_PUSH(tnames, tn);
        }
    }
    for (int i = 0; i < S->records.n; i++) resolve_record(S->records.items[i]);
}

/* --------------------------------- symbols --------------------------------- */

static Symbol *new_symbol(char *name, Type t, SymKind k, int line) {
    Symbol *s = calloc(1, sizeof *s);
    s->name = name;
    s->type = t;
    s->kind = k;
    s->line = line;
    return s;
}

static Symbol *find_global(const char *name) {
    for (int i = 0; i < S->globals.n; i++)
        if (strcmp(S->globals.items[i]->name, name) == 0) return S->globals.items[i];
    return NULL;
}

static Symbol *find_local(FuncInfo *fi, const char *name) {
    for (int i = 0; i < fi->syms.n; i++)
        if (strcmp(fi->syms.items[i]->name, name) == 0) return fi->syms.items[i];
    return NULL;
}

static Symbol *lookup(const char *name) {
    Symbol *s = find_local(&S->funcs[cur_func], name);
    return s ? s : find_global(name);
}

static void collect_globals(void) {
    for (int i = 0; i < P->funcs.n; i++) {
        Function *f = P->funcs.items[i];
        for (int j = 0; j < f->decls.n; j++) {
            Decl *d = &f->decls.items[j];
            if (!d->global) continue;
            if (find_global(d->id)) { error(d->line, "global variable %s is declared more than once", d->id); continue; }
            VEC_PUSH(S->globals, new_symbol(d->id, resolve_typespec(&d->type), SYM_GLOBAL, d->line));
        }
    }
}

static void add_local(FuncInfo *fi, char *name, const TypeSpec *ts, SymKind k, int line) {
    if (find_local(fi, name)) { error(line, "variable %s is declared more than once in %s", name, fi->f->name); return; }
    if (find_global(name)) { error(line, "%s is already declared as a global variable", name); return; }
    VEC_PUSH(fi->syms, new_symbol(name, resolve_typespec(ts), k, line));
}

static void collect_signatures(void) {
    for (int i = 0; i < P->funcs.n; i++) {
        FuncInfo *fi = &S->funcs[i];
        Function *f = fi->f;
        for (int j = 0; j < i; j++)
            if (strcmp(P->funcs.items[j]->name, f->name) == 0)
                error(f->line, "function %s is defined more than once", f->name);
        for (int j = 0; j < f->ins.n; j++) add_local(fi, f->ins.items[j].id, &f->ins.items[j].type, SYM_INPUT, f->ins.items[j].line);
        for (int j = 0; j < f->outs.n; j++) add_local(fi, f->outs.items[j].id, &f->outs.items[j].type, SYM_OUTPUT, f->outs.items[j].line);
    }
}

/* ------------------------------- expressions -------------------------------- */

static Type check_varref(VarRef *v) {
    Symbol *s = lookup(v->id);
    if (!s) { error(v->line, "variable %s is not declared", v->id); return ERR; }
    v->sym = s;
    Type t = s->type;
    for (int i = 0; i < v->fields.n; i++) {
        if (t.kind == TY_ERROR) return ERR;
        char buf[300];
        if (t.kind != TY_RECORD && t.kind != TY_UNION) {
            error(v->line, "%s is of type %s, so it has no field %s", v->id, type_name(t, buf), v->fields.items[i]);
            return ERR;
        }
        int idx = -1;
        for (int k = 0; k < t.rec->nfields; k++)
            if (strcmp(t.rec->field_names[k], v->fields.items[i]) == 0) idx = k;
        if (idx < 0) {
            error(v->line, "%s has no field named %s", type_name(t, buf), v->fields.items[i]);
            return ERR;
        }
        VEC_PUSH(v->field_index, idx);
        t = t.rec->field_types[idx];
    }
    return t;
}

/* can every leaf field of the record be combined with a scalar of type s? (no unions inside) */
static int record_scalable(RecordDef *r, TyKind s) {
    for (int i = 0; i < r->nfields; i++) {
        Type t = r->field_types[i];
        if (t.kind == TY_RECORD) { if (!record_scalable(t.rec, s)) return 0; }
        else if (t.kind == TY_UNION || t.kind == TY_ERROR) return 0;
        else if (t.kind == TY_INT && s == TY_REAL) return 0;
    }
    return 1;
}

static Type check_expr(Expr *e) {
    char b1[300], b2[300];
    switch (e->kind) {
        case E_NUM: e->type = (Type){TY_INT, NULL}; break;
        case E_RNUM: e->type = (Type){TY_REAL, NULL}; break;
        case E_VAR: e->type = check_varref(&e->var); break;
        case E_BIN: {
            Type l = check_expr(e->l), r = check_expr(e->r);
            e->type = ERR;
            if (l.kind == TY_ERROR || r.kind == TY_ERROR) break;
            if (is_num(l) && is_num(r)) {
                e->type = (l.kind == TY_REAL || r.kind == TY_REAL) ? (Type){TY_REAL, NULL} : (Type){TY_INT, NULL};
            } else if (l.kind == TY_RECORD && type_eq(l, r) && (e->op == '+' || e->op == '-') &&
                       record_scalable(l.rec, TY_INT)) {
                e->type = l;
            } else if (l.kind == TY_RECORD && is_num(r) && (e->op == '*' || e->op == '/') && record_scalable(l.rec, r.kind)) {
                e->type = l;
            } else if (is_num(l) && r.kind == TY_RECORD && e->op == '*' && record_scalable(r.rec, l.kind)) {
                e->type = r;
            } else {
                error(e->line, "operator '%c' cannot be applied to %s and %s", e->op, type_name(l, b1), type_name(r, b2));
            }
            break;
        }
    }
    return e->type;
}

static void check_bool(BoolExpr *b) {
    char b1[300], b2[300];
    if (b->kind == B_REL) {
        Type l = check_expr(b->lv), r = check_expr(b->rv);
        if (l.kind != TY_ERROR && r.kind != TY_ERROR && !(is_num(l) && is_num(r)))
            error(b->line, "comparison needs int or real values, found %s and %s", type_name(l, b1), type_name(r, b2));
        return;
    }
    check_bool(b->l);
    if (b->r) check_bool(b->r);
}

/* -------------------------------- statements -------------------------------- */

typedef VEC(Symbol *) SymSet;

static void bool_vars(BoolExpr *b, SymSet *out) {
    if (b->kind == B_REL) {
        if (b->lv->kind == E_VAR && b->lv->var.sym) VEC_PUSH(*out, b->lv->var.sym);
        if (b->rv->kind == E_VAR && b->rv->var.sym) VEC_PUSH(*out, b->rv->var.sym);
        return;
    }
    bool_vars(b->l, out);
    if (b->r) bool_vars(b->r, out);
}

static void assigned_vars(StmtList *l, SymSet *out) {
    for (int i = 0; i < l->n; i++) {
        Stmt *s = l->items[i];
        if ((s->kind == S_ASSIGN || s->kind == S_READ) && s->target.sym) VEC_PUSH(*out, s->target.sym);
        if (s->kind == S_CALL)
            for (int j = 0; j < s->outs.n; j++) {
                Symbol *sym = lookup(s->outs.items[j].name);
                if (sym) VEC_PUSH(*out, sym);
            }
        assigned_vars(&s->body, out);
        assigned_vars(&s->els, out);
    }
}

static void check_stmts(StmtList *l);

static void mark_assigned(VarRef *v) {
    if (v->sym) v->sym->assigned = 1;
}

static void check_call(Stmt *s) {
    char b1[300], b2[300];
    int j = -1;
    for (int k = 0; k < P->funcs.n; k++)
        if (strcmp(P->funcs.items[k]->name, s->fun) == 0) { j = k; break; }
    if (j < 0) { error(s->line, "function %s is not defined", s->fun); return; }
    if (j == cur_func) { error(s->line, "function %s cannot call itself (recursion is not allowed)", s->fun); return; }
    if (j > cur_func) { error(s->line, "function %s is called before it is defined", s->fun); return; }
    Function *callee = P->funcs.items[j];
    FuncInfo *ci = &S->funcs[j];

    if (s->ins.n != callee->ins.n) {
        error(s->line, "%s expects %d input parameter(s) but is called with %d", s->fun, callee->ins.n, s->ins.n);
    } else {
        for (int k = 0; k < s->ins.n; k++) {
            Symbol *a = lookup(s->ins.items[k].name);
            Type pt = ci->syms.items[k]->type;
            if (!a) { error(s->line, "variable %s is not declared", s->ins.items[k].name); continue; }
            if (a->type.kind != TY_ERROR && pt.kind != TY_ERROR && !assignable(pt, a->type))
                error(s->line, "input %d of %s must be %s, but %s is %s", k + 1, s->fun, type_name(pt, b1),
                      a->name, type_name(a->type, b2));
        }
    }
    if (s->outs.n != callee->outs.n) {
        error(s->line, "%s returns %d value(s) but the call receives %d", s->fun, callee->outs.n, s->outs.n);
        return;
    }
    for (int k = 0; k < s->outs.n; k++) {
        Symbol *a = lookup(s->outs.items[k].name);
        Type pt = ci->syms.items[callee->ins.n + k]->type;
        if (!a) { error(s->line, "variable %s is not declared", s->outs.items[k].name); continue; }
        a->assigned = 1;
        if (a->type.kind != TY_ERROR && pt.kind != TY_ERROR && !type_eq(pt, a->type))
            error(s->line, "output %d of %s is %s, but %s is %s", k + 1, s->fun, type_name(pt, b1), a->name,
                  type_name(a->type, b2));
    }
}

static void check_stmt(Stmt *s) {
    char b1[300], b2[300];
    switch (s->kind) {
        case S_ASSIGN: {
            Type t = check_varref(&s->target), v = check_expr(s->expr);
            mark_assigned(&s->target);
            if (t.kind != TY_ERROR && v.kind != TY_ERROR && !assignable(t, v))
                error(s->line, "cannot assign a value of type %s to %s, which is %s", type_name(v, b1), s->target.id,
                      type_name(t, b2));
            break;
        }
        case S_READ: {
            Type t = check_varref(&s->target);
            mark_assigned(&s->target);
            if (t.kind != TY_ERROR && !is_num(t)) error(s->line, "read needs an int or real variable, found %s", type_name(t, b1));
            break;
        }
        case S_WRITE: {
            Type t = check_expr(s->expr);
            if (t.kind == TY_UNION || (t.kind == TY_RECORD && !record_scalable(t.rec, TY_INT)))
                error(s->line, "cannot write %s: write its fields instead", type_name(t, b1));
            break;
        }
        case S_CALL: check_call(s); break;
        case S_WHILE: {
            check_bool(s->cond);
            check_stmts(&s->body);
            SymSet in_cond = {0}, changed = {0};
            bool_vars(s->cond, &in_cond);
            assigned_vars(&s->body, &changed);
            int updated = in_cond.n == 0;
            for (int i = 0; i < in_cond.n && !updated; i++)
                for (int k = 0; k < changed.n; k++)
                    if (in_cond.items[i] == changed.items[k]) updated = 1;
            if (!updated) warning(s->line, "none of the variables in this while condition change inside the loop");
            free(in_cond.items);
            free(changed.items);
            break;
        }
        case S_IF:
            check_bool(s->cond);
            check_stmts(&s->body);
            check_stmts(&s->els);
            break;
    }
}

static void check_stmts(StmtList *l) {
    for (int i = 0; i < l->n; i++) check_stmt(l->items[i]);
}

static void check_function(int i) {
    char b1[300], b2[300];
    cur_func = i;
    FuncInfo *fi = &S->funcs[i];
    Function *f = fi->f;
    for (int j = 0; j < f->decls.n; j++) {
        Decl *d = &f->decls.items[j];
        if (!d->global) add_local(fi, d->id, &d->type, SYM_LOCAL, d->line);
    }
    check_stmts(&f->stmts);

    if (f->is_main) {
        if (f->has_return_list) error(f->ret_line, "_main cannot return values");
        return;
    }
    if (f->ret.n != f->outs.n) {
        error(f->ret_line, "%s has %d output parameter(s) but returns %d value(s)", f->name, f->outs.n, f->ret.n);
    } else {
        for (int k = 0; k < f->ret.n; k++) {
            Symbol *s = lookup(f->ret.items[k].name);
            Type ot = fi->syms.items[f->ins.n + k]->type;
            if (!s) { error(f->ret_line, "variable %s is not declared", f->ret.items[k].name); continue; }
            if (s->type.kind != TY_ERROR && ot.kind != TY_ERROR && !type_eq(s->type, ot))
                error(f->ret_line, "returned value %s is %s, but output %d of %s is %s", s->name, type_name(s->type, b1),
                      k + 1, f->name, type_name(ot, b2));
        }
    }
    for (int k = 0; k < fi->syms.n; k++) {
        Symbol *s = fi->syms.items[k];
        if (s->kind == SYM_OUTPUT && !s->assigned)
            error(s->line, "output parameter %s of %s is never assigned a value", s->name, f->name);
    }
}

Sema *sema_check(Program *prog) {
    S = calloc(1, sizeof *S);
    P = prog;
    tnames.n = 0;
    S->funcs = calloc(prog->funcs.n, sizeof(FuncInfo));
    for (int i = 0; i < prog->funcs.n; i++) S->funcs[i].f = prog->funcs.items[i];
    collect_types();
    collect_globals();
    collect_signatures();
    for (int i = 0; i < prog->funcs.n; i++) check_function(i);
    return S;
}
