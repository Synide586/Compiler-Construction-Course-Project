/* Back-end driver: source file -> LL(1) parse tree -> AST -> semantic checks -> LLVM IR -> executable.
   Usage: ./compiler <source.txt> [-o <executable>] [--emit-llvm <file.ll>] [--ast] [--check] [-O] */
#include "codegen.h"
#include "parser.h"

static void usage(void) {
    fprintf(stderr,
            "usage: ./compiler <source.txt> [options]\n"
            "  -o <file>             name of the executable to build (default: a.out)\n"
            "  --emit-llvm <file>    also keep the generated LLVM IR in <file>\n"
            "  --ast                 print the abstract syntax tree and stop\n"
            "  --check               stop after semantic analysis\n"
            "  -O                    optimize the executable (clang -O2)\n");
}

int main(int argc, char **argv) {
    const char *src = NULL, *exe = "a.out", *ll = NULL;
    int print_ast = 0, check_only = 0, optimize = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-o") && i + 1 < argc) exe = argv[++i];
        else if (!strcmp(argv[i], "--emit-llvm") && i + 1 < argc) ll = argv[++i];
        else if (!strcmp(argv[i], "--ast")) print_ast = 1;
        else if (!strcmp(argv[i], "--check")) check_only = 1;
        else if (!strcmp(argv[i], "-O")) optimize = 1;
        else if (argv[i][0] != '-' && !src) src = argv[i];
        else { usage(); return 2; }
    }
    if (!src) { usage(); return 2; }

    /* front end: the LL(1) parser builds the parse tree */
    grammar *G = initializeGrammar();
    FirstAndFollow F = computeFirstAndFollowSets(G);
    static table T;
    createParseTable(&F, &T, G);
    parseTree *pt = parseInputSourceCode((char *)src, &T, G);
    if (!pt) return 1;
    if (parseErrorCount > 0) {
        fprintf(stderr, "Compilation stopped: %d lexical/syntax error(s)\n", parseErrorCount);
        return 1;
    }

    /* back end */
    Program *prog = build_ast(pt->root);
    if (print_ast) { ast_print(prog, stdout); return 0; }

    Sema *sema = sema_check(prog);
    if (sema->errors > 0) {
        fprintf(stderr, "Compilation stopped: %d semantic error(s)\n", sema->errors);
        return 1;
    }
    printf("Semantic analysis passed (%d warning(s))\n", sema->warnings);
    if (check_only) return 0;

    char ll_path[1024];
    if (ll) snprintf(ll_path, sizeof ll_path, "%s", ll);
    else snprintf(ll_path, sizeof ll_path, "%s.ll", exe);
    FILE *out = fopen(ll_path, "w");
    if (!out) { fprintf(stderr, "error: cannot write %s\n", ll_path); return 1; }
    codegen_emit(prog, sema, src, out);
    fclose(out);

    char cmd[3000];
    snprintf(cmd, sizeof cmd, "clang %s -Wno-override-module -o '%s' '%s'", optimize ? "-O2" : "", exe, ll_path);
    int rc = system(cmd);
    if (!ll) remove(ll_path);
    if (rc != 0) { fprintf(stderr, "error: clang failed to build the executable\n"); return 1; }
    printf("Built executable: %s\n", exe);
    return 0;
}
