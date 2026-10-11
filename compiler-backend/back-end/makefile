# Back end: AST construction, semantic analysis and LLVM IR code generation.
# Uses the lexer and LL(1) parser from ../front-end_v2.
# Requires clang to turn the generated LLVM IR into an executable.

CC = gcc
FE = ../front-end_v2
CFLAGS = -Wall -Wextra -std=c11 -g -I. -I$(FE)
LDFLAGS = -lm

SOURCES = compiler.c ast_builder.c semantic.c codegen.c $(FE)/lexer.c $(FE)/parser.c
HEADERS = ast.h semantic.h codegen.h $(FE)/lexerDef.h $(FE)/lexer.h $(FE)/parserDef.h $(FE)/parser.h

compiler: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -Wno-format-truncation -o $@ $(SOURCES) $(LDFLAGS)

test: compiler
	./run_tests.sh

clean:
	rm -rf compiler build

.PHONY: test clean
