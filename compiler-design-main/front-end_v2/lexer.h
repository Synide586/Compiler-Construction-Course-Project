#ifndef LEXER_H
#define LEXER_H
#include "lexerDef.h"

twinBuffer* initializeTwinBuffer(FILE *fp);

char getNextChar(twinBuffer *tb);

void retract(twinBuffer *tb, int n);

void getLexeme(twinBuffer *tb, char *lexeme);

void freeTwinBuffer(twinBuffer *tb);

TokenType lookupKeyword(const char *lexeme);

tokenInfo getNextToken(twinBuffer *tb);

void removeComments(char *testcaseFile, char *cleanFile);

const char* getTokenName(TokenType token);

#endif
