/* Converts the parse tree produced by the LL(1) parser into an abstract syntax tree.
   Each builder function handles one nonterminal; the parse tree has one child per
   symbol of the grammar rule that was applied, so alternatives are told apart by
   their first child. */
#include "ast.h"

static char *dup(const char *s) {
    char *r = malloc(strlen(s) + 1);
    strcpy(r, s);
    return r;
}

static parseTreeNode *ch(parseTreeNode *n, int i) { return n->children[i]; }

static int is_term(parseTreeNode *n, TokenType t) {
    return n->symbol.type == SYMBOL_TERMINAL && n->symbol.symbol.terminal == t;
}

static int is_nonterm(parseTreeNode *n, NonTerminal nt) {
    return n->symbol.type == SYMBOL_NON_TERMINAL && n->symbol.symbol.nonTerminal == nt;
}

/* an epsilon alternative has a single TK_EPSILON child */
static int is_eps(parseTreeNode *n) { return n->childCount == 0 || is_term(ch(n, 0), TK_EPSILON); }

static int first_line(parseTreeNode *n) {
    if (n->symbol.type == SYMBOL_TERMINAL) return n->token.lineNumber;
    for (int i = 0; i < n->childCount; i++) {
        int l = first_line(ch(n, i));
        if (l > 0) return l;
    }
    return 0;
}

/* ------------------------------ types ------------------------------------- */

/* <primitiveDatatype> | <constructedDatatype> | TK_RUID (field types) */
static TypeSpec build_typespec(parseTreeNode *n) {
    TypeSpec t = {TS_INT, NULL, first_line(n)};
    if (is_nonterm(n, NT_DATATYPE) || is_nonterm(n, NT_FIELDTYPE)) {
        if (is_term(ch(n, 0), TK_RUID)) {  /* <fieldType> ===> TK_RUID */
            t.kind = TS_NAMED;
            t.ruid = dup(ch(n, 0)->token.lexeme);
            return t;
        }
        return build_typespec(ch(n, 0));
    }
    if (is_nonterm(n, NT_PRIMITIVEDATATYPE)) {
        t.kind = is_term(ch(n, 0), TK_INT) ? TS_INT : TS_REAL;
        return t;
    }
    /* <constructedDatatype> ===> TK_RECORD TK_RUID | TK_UNION TK_RUID | TK_RUID */
    if (is_term(ch(n, 0), TK_RUID)) {
        t.kind = TS_NAMED;
        t.ruid = dup(ch(n, 0)->token.lexeme);
    } else {
        t.kind = is_term(ch(n, 0), TK_RECORD) ? TS_RECORD : TS_UNION;
        t.ruid = dup(ch(n, 1)->token.lexeme);
    }
    return t;
}

/* <parameter_list> ===> <dataType> TK_ID <remaining_list> */
static void build_params(parseTreeNode *n, ParamList *out) {
    for (;;) {
        Param p = {build_typespec(ch(n, 0)), dup(ch(n, 1)->token.lexeme), ch(n, 1)->token.lineNumber};
        VEC_PUSH(*out, p);
        parseTreeNode *rest = ch(n, 2);
        if (is_eps(rest)) return;
        n = ch(rest, 1);
    }
}

/* ------------------------- variables and expressions ---------------------- */

/* <singleOrRecId> ===> TK_ID <option_single_constructed> */
static VarRef build_varref(parseTreeNode *n) {
    VarRef v;
    memset(&v, 0, sizeof v);
    v.id = dup(ch(n, 0)->token.lexeme);
    v.line = ch(n, 0)->token.lineNumber;
    parseTreeNode *opt = ch(n, 1);
    if (is_eps(opt)) return v;
    VEC_PUSH(v.fields, dup(ch(ch(opt, 0), 1)->token.lexeme)); /* <oneExpansion> ===> TK_DOT TK_FIELDID */
    for (parseTreeNode *more = ch(opt, 1); !is_eps(more); more = ch(more, 1))
        VEC_PUSH(v.fields, dup(ch(ch(more, 0), 1)->token.lexeme));
    return v;
}

static Expr *new_expr(ExprKind k, int line) {
    Expr *e = calloc(1, sizeof *e);
    e->kind = k;
    e->line = line;
    return e;
}

/* <var> ===> <singleOrRecId> | TK_NUM | TK_RNUM */
static Expr *build_var(parseTreeNode *n) {
    parseTreeNode *c = ch(n, 0);
    if (is_term(c, TK_NUM)) {
        Expr *e = new_expr(E_NUM, c->token.lineNumber);
        e->ival = c->token.value.intValue;
        return e;
    }
    if (is_term(c, TK_RNUM)) {
        Expr *e = new_expr(E_RNUM, c->token.lineNumber);
        e->rval = c->token.value.realValue;
        return e;
    }
    Expr *e = new_expr(E_VAR, first_line(c));
    e->var = build_varref(c);
    return e;
}

static Expr *build_arith(parseTreeNode *n);

/* <factor> ===> TK_OP <arithmeticExpression> TK_CL | <var> */
static Expr *build_factor(parseTreeNode *n) {
    if (is_term(ch(n, 0), TK_OP)) return build_arith(ch(n, 1));
    return build_var(ch(n, 0));
}

static Expr *binary(char op, Expr *l, Expr *r, int line) {
    Expr *e = new_expr(E_BIN, line);
    e->op = op;
    e->l = l;
    e->r = r;
    return e;
}

/* <term> ===> <factor> <termPrime>;  <termPrime> ===> <highPrecedenceOp> <factor> <termPrime> | eps
   The right-recursive grammar is folded back into left-associative operations. */
static Expr *build_term(parseTreeNode *n) {
    Expr *e = build_factor(ch(n, 0));
    for (parseTreeNode *p = ch(n, 1); !is_eps(p); p = ch(p, 2)) {
        parseTreeNode *op = ch(ch(p, 0), 0);
        e = binary(is_term(op, TK_MUL) ? '*' : '/', e, build_factor(ch(p, 1)), op->token.lineNumber);
    }
    return e;
}

/* <arithmeticExpression> ===> <term> <expPrime>;  <expPrime> ===> <lowPrecedenceOp> <term> <expPrime> | eps */
static Expr *build_arith(parseTreeNode *n) {
    Expr *e = build_term(ch(n, 0));
    for (parseTreeNode *p = ch(n, 1); !is_eps(p); p = ch(p, 2)) {
        parseTreeNode *op = ch(ch(p, 0), 0);
        e = binary(is_term(op, TK_PLUS) ? '+' : '-', e, build_term(ch(p, 1)), op->token.lineNumber);
    }
    return e;
}

static BoolExpr *build_bool(parseTreeNode *n) {
    BoolExpr *b = calloc(1, sizeof *b);
    b->line = first_line(n);
    parseTreeNode *c0 = ch(n, 0);
    if (is_term(c0, TK_NOT)) {  /* TK_NOT TK_OP <booleanExpression> TK_CL */
        b->kind = B_NOT;
        b->l = build_bool(ch(n, 2));
    } else if (is_term(c0, TK_OP)) {  /* ( <bool> ) <logicalOp> ( <bool> ) */
        b->kind = is_term(ch(ch(n, 3), 0), TK_AND) ? B_AND : B_OR;
        b->l = build_bool(ch(n, 1));
        b->r = build_bool(ch(n, 5));
    } else {  /* <var> <relationalOp> <var> */
        b->kind = B_REL;
        b->lv = build_var(c0);
        b->relop = ch(ch(n, 1), 0)->symbol.symbol.terminal;
        b->rv = build_var(ch(n, 2));
    }
    return b;
}

/* ---------------------------------- statements ----------------------------- */

/* <idList> ===> TK_ID <more_ids>;  <more_ids> ===> TK_COMMA <idList> | eps */
static void build_idlist(parseTreeNode *n, IdList *out) {
    for (;;) {
        Ident id = {dup(ch(n, 0)->token.lexeme), ch(n, 0)->token.lineNumber};
        VEC_PUSH(*out, id);
        if (is_eps(ch(n, 1))) return;
        n = ch(ch(n, 1), 1);
    }
}

static void build_stmt_list(parseTreeNode *otherStmts, StmtList *out);

static Stmt *build_stmt(parseTreeNode *stmt) {
    parseTreeNode *n = ch(stmt, 0);
    Stmt *s = calloc(1, sizeof *s);
    s->line = first_line(n);
    switch (n->symbol.symbol.nonTerminal) {
        case NT_ASSIGNMENTSTMT:
            s->kind = S_ASSIGN;
            s->target = build_varref(ch(n, 0));
            s->expr = build_arith(ch(n, 2));
            break;
        case NT_ITERATIVESTMT:
            s->kind = S_WHILE;
            s->cond = build_bool(ch(n, 2));
            VEC_PUSH(s->body, build_stmt(ch(n, 4)));
            build_stmt_list(ch(n, 5), &s->body);
            break;
        case NT_CONDITIONALSTMT: {
            s->kind = S_IF;
            s->cond = build_bool(ch(n, 2));
            VEC_PUSH(s->body, build_stmt(ch(n, 5)));
            build_stmt_list(ch(n, 6), &s->body);
            parseTreeNode *elsePart = ch(n, 7);
            if (is_term(ch(elsePart, 0), TK_ELSE)) {
                s->has_else = 1;
                VEC_PUSH(s->els, build_stmt(ch(elsePart, 1)));
                build_stmt_list(ch(elsePart, 2), &s->els);
            }
            break;
        }
        case NT_IOSTMT:
            if (is_term(ch(n, 0), TK_READ)) {
                s->kind = S_READ;
                s->target = build_varref(ch(n, 2));
            } else {
                s->kind = S_WRITE;
                s->expr = build_var(ch(n, 2));
            }
            break;
        case NT_FUNCALLSTMT:
            s->kind = S_CALL;
            if (!is_eps(ch(n, 0))) build_idlist(ch(ch(n, 0), 1), &s->outs);
            s->fun = dup(ch(n, 2)->token.lexeme);
            build_idlist(ch(ch(n, 5), 1), &s->ins);
            break;
        default:
            break;
    }
    return s;
}

/* <otherStmts> ===> <stmt> <otherStmts> | eps */
static void build_stmt_list(parseTreeNode *n, StmtList *out) {
    for (; !is_eps(n); n = ch(n, 1)) VEC_PUSH(*out, build_stmt(ch(n, 0)));
}

/* --------------------------- definitions and functions --------------------- */

static void build_fielddef(parseTreeNode *n, TypeDef *t) {
    FieldDef f = {build_typespec(ch(n, 1)), dup(ch(n, 3)->token.lexeme), ch(n, 3)->token.lineNumber};
    VEC_PUSH(t->fields, f);
}

static void build_typedefs(parseTreeNode *n, Function *f) {
    for (; !is_eps(n); n = ch(n, 1)) {
        parseTreeNode *def = ch(ch(n, 0), 0);
        if (is_nonterm(def, NT_DEFINETYPESTMT)) {
            DefineType d = {is_term(ch(ch(def, 1), 0), TK_UNION), dup(ch(def, 2)->token.lexeme),
                            dup(ch(def, 4)->token.lexeme), first_line(def)};
            VEC_PUSH(f->aliases, d);
            continue;
        }
        TypeDef t;
        memset(&t, 0, sizeof t);
        t.is_union = is_term(ch(def, 0), TK_UNION);
        t.ruid = dup(ch(def, 1)->token.lexeme);
        t.line = ch(def, 1)->token.lineNumber;
        parseTreeNode *fds = ch(def, 2);  /* <fieldDefinitions> ===> <fd> <fd> <moreFields> */
        build_fielddef(ch(fds, 0), &t);
        build_fielddef(ch(fds, 1), &t);
        for (parseTreeNode *m = ch(fds, 2); !is_eps(m); m = ch(m, 1)) build_fielddef(ch(m, 0), &t);
        VEC_PUSH(f->typedefs, t);
    }
}

/* <stmts> ===> <typeDefinitions> <declarations> <otherStmts> <returnStmt> */
static void build_body(parseTreeNode *stmts, Function *f) {
    build_typedefs(ch(stmts, 0), f);
    for (parseTreeNode *d = ch(stmts, 1); !is_eps(d); d = ch(d, 1)) {
        parseTreeNode *dn = ch(d, 0);
        Decl decl = {build_typespec(ch(dn, 1)), dup(ch(dn, 3)->token.lexeme), !is_eps(ch(dn, 4)),
                     ch(dn, 3)->token.lineNumber};
        VEC_PUSH(f->decls, decl);
    }
    build_stmt_list(ch(stmts, 2), &f->stmts);
    parseTreeNode *ret = ch(stmts, 3);
    f->ret_line = ch(ret, 0)->token.lineNumber;
    if (!is_eps(ch(ret, 1))) {
        f->has_return_list = 1;
        build_idlist(ch(ch(ret, 1), 1), &f->ret);
    }
}

Program *build_ast(parseTreeNode *root) {
    Program *prog = calloc(1, sizeof *prog);
    /* <program> ===> <otherFunctions> <mainFunction> */
    for (parseTreeNode *of = ch(root, 0); !is_eps(of); of = ch(of, 1)) {
        parseTreeNode *fn = ch(of, 0);  /* TK_FUNID <input_par> <output_par> TK_SEM <stmts> TK_END */
        Function *f = calloc(1, sizeof *f);
        f->name = dup(ch(fn, 0)->token.lexeme);
        f->line = ch(fn, 0)->token.lineNumber;
        build_params(ch(ch(fn, 1), 4), &f->ins);
        if (!is_eps(ch(fn, 2))) build_params(ch(ch(fn, 2), 4), &f->outs);
        build_body(ch(fn, 4), f);
        VEC_PUSH(prog->funcs, f);
    }
    parseTreeNode *mainFn = ch(root, 1);  /* TK_MAIN <stmts> TK_END */
    Function *m = calloc(1, sizeof *m);
    m->name = dup("_main");
    m->is_main = 1;
    m->line = ch(mainFn, 0)->token.lineNumber;
    build_body(ch(mainFn, 1), m);
    VEC_PUSH(prog->funcs, m);
    return prog;
}

/* ------------------------------------ printing ------------------------------ */

static const char *typespec_str(const TypeSpec *t, char *buf) {
    switch (t->kind) {
        case TS_INT: return "int";
        case TS_REAL: return "real";
        case TS_RECORD: sprintf(buf, "record %s", t->ruid); return buf;
        case TS_UNION: sprintf(buf, "union %s", t->ruid); return buf;
        default: return t->ruid;
    }
}

static void print_var(const VarRef *v, FILE *o) {
    fprintf(o, "%s", v->id);
    for (int i = 0; i < v->fields.n; i++) fprintf(o, ".%s", v->fields.items[i]);
}

static void print_expr(const Expr *e, FILE *o) {
    switch (e->kind) {
        case E_NUM: fprintf(o, "%d", e->ival); break;
        case E_RNUM: fprintf(o, "%.2f", e->rval); break;
        case E_VAR: print_var(&e->var, o); break;
        case E_BIN:
            fprintf(o, "(");
            print_expr(e->l, o);
            fprintf(o, " %c ", e->op);
            print_expr(e->r, o);
            fprintf(o, ")");
    }
}

static const char *relop_str(TokenType t) {
    switch (t) {
        case TK_LT: return "<";
        case TK_LE: return "<=";
        case TK_EQ: return "==";
        case TK_GT: return ">";
        case TK_GE: return ">=";
        default: return "!=";
    }
}

static void print_bool(const BoolExpr *b, FILE *o) {
    if (b->kind == B_REL) {
        print_expr(b->lv, o);
        fprintf(o, " %s ", relop_str(b->relop));
        print_expr(b->rv, o);
    } else if (b->kind == B_NOT) {
        fprintf(o, "~(");
        print_bool(b->l, o);
        fprintf(o, ")");
    } else {
        fprintf(o, "(");
        print_bool(b->l, o);
        fprintf(o, ") %s (", b->kind == B_AND ? "&&&" : "@@@");
        print_bool(b->r, o);
        fprintf(o, ")");
    }
}

static void print_ids(const IdList *l, FILE *o) {
    fprintf(o, "[");
    for (int i = 0; i < l->n; i++) fprintf(o, "%s%s", i ? ", " : "", l->items[i].name);
    fprintf(o, "]");
}

static void print_stmts(const StmtList *l, int depth, FILE *o) {
    for (int i = 0; i < l->n; i++) {
        const Stmt *s = l->items[i];
        fprintf(o, "%*s", 2 * depth, "");
        switch (s->kind) {
            case S_ASSIGN: fprintf(o, "Assign "); print_var(&s->target, o); fprintf(o, " <--- "); print_expr(s->expr, o); break;
            case S_READ: fprintf(o, "Read "); print_var(&s->target, o); break;
            case S_WRITE: fprintf(o, "Write "); print_expr(s->expr, o); break;
            case S_CALL:
                fprintf(o, "Call %s inputs ", s->fun);
                print_ids(&s->ins, o);
                fprintf(o, " outputs ");
                print_ids(&s->outs, o);
                break;
            case S_WHILE: fprintf(o, "While "); print_bool(s->cond, o); break;
            case S_IF: fprintf(o, "If "); print_bool(s->cond, o); break;
        }
        fprintf(o, "\n");
        if (s->kind == S_WHILE || s->kind == S_IF) print_stmts(&s->body, depth + 1, o);
        if (s->has_else) {
            fprintf(o, "%*sElse\n", 2 * depth, "");
            print_stmts(&s->els, depth + 1, o);
        }
    }
}

void ast_print(const Program *p, FILE *o) {
    char buf[300];
    for (int i = 0; i < p->funcs.n; i++) {
        const Function *f = p->funcs.items[i];
        fprintf(o, "Function %s (line %d)\n", f->name, f->line);
        for (int j = 0; j < f->ins.n; j++) fprintf(o, "  input  %s %s\n", typespec_str(&f->ins.items[j].type, buf), f->ins.items[j].id);
        for (int j = 0; j < f->outs.n; j++) fprintf(o, "  output %s %s\n", typespec_str(&f->outs.items[j].type, buf), f->outs.items[j].id);
        for (int j = 0; j < f->typedefs.n; j++) {
            const TypeDef *t = &f->typedefs.items[j];
            fprintf(o, "  %s %s\n", t->is_union ? "union" : "record", t->ruid);
            for (int k = 0; k < t->fields.n; k++)
                fprintf(o, "    field %s %s\n", typespec_str(&t->fields.items[k].type, buf), t->fields.items[k].name);
        }
        for (int j = 0; j < f->aliases.n; j++)
            fprintf(o, "  definetype %s %s as %s\n", f->aliases.items[j].is_union ? "union" : "record",
                    f->aliases.items[j].from, f->aliases.items[j].to);
        for (int j = 0; j < f->decls.n; j++)
            fprintf(o, "  declare %s %s%s\n", typespec_str(&f->decls.items[j].type, buf), f->decls.items[j].id,
                    f->decls.items[j].global ? " (global)" : "");
        print_stmts(&f->stmts, 1, o);
        fprintf(o, "  Return ");
        if (f->has_return_list) print_ids(&f->ret, o);
        fprintf(o, "\n");
    }
}
