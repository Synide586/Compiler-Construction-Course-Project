#ifndef PARSER_H
#define PARSER_H
#include "parserDef.h"
#include "lexer.h"

grammar* initializeGrammar(void);

FirstAndFollow computeFirstAndFollowSets(grammar *G);

void printFirstAndFollow(FirstAndFollow *F);

void createParseTable(FirstAndFollow *F, table *T, grammar *G);

void printParseTable(table *T, grammar *G);

parseTree* parseInputSourceCode(char *testcaseFile, table *T, grammar *G);

void printParseTree(parseTree *PT, char *outfile);

void freeParseTree(parseTree *PT);

const char* getNonTerminalName(NonTerminal nt);

#endif
