#ifndef PARSERDEF_H
#define PARSERDEF_H
#include "lexerDef.h"

#define MAX_CHILDREN 15
#define MAX_RHS 15
#define MAX_RULES 140
#define MAX_FIRST_FOLLOW 45
#define MAX_NT 60
#define TABLE_TERMINALS 70

typedef enum {
  NT_PROGRAM = 0,
  NT_MAINFUNCTION,
  NT_OTHERFUNCTIONS,
  NT_FUNCTION,
  NT_INPUT_PAR,
  NT_OUTPUT_PAR,
  NT_PARAMETER_LIST,
  NT_DATATYPE,
  NT_PRIMITIVEDATATYPE,
  NT_CONSTRUCTEDDATATYPE,
  NT_REMAINING_LIST,
  NT_STMTS,
  NT_TYPEDEFINITIONS,
  NT_ACTUALORREDEFINED,
  NT_TYPEDEFINITION,
  NT_FIELDDEFINITIONS,
  NT_FIELDTYPE,
  NT_FIELDDEFINITION,
  NT_MOREFIELDS,
  NT_DECLARATIONS,
  NT_DECLARATION,
  NT_GLOBAL_OR_NOT,
  NT_OTHERSTMTS,
  NT_STMT,
  NT_ASSIGNMENTSTMT,
  NT_SINGLEORRECID,
  NT_OPTION_SINGLE_CONSTRUCTED,
  NT_ONEEXPANSION,
  NT_MOREEXPANSIONS,
  NT_FUNCALLSTMT,
  NT_OUTPUTPARAMETERS,
  NT_INPUTPARAMETERS,
  NT_ITERATIVESTMT,
  NT_CONDITIONALSTMT,
  NT_ELSEPART,
  NT_IOSTMT,
  NT_ARITHMETICEXPRESSION,
  NT_EXPPRIME,
  NT_TERM,
  NT_TERMPRIME,
  NT_FACTOR,
  NT_HIGHPRECEDENCEOP,
  NT_LOWPRECEDENCEOP,
  NT_BOOLEANEXPRESSION,
  NT_VAR,
  NT_LOGICALOP,
  NT_RELATIONALOP,
  NT_RETURNSTMT,
  NT_OPTIONALRETURN,
  NT_IDLIST,
  NT_MORE_IDS,
  NT_DEFINETYPESTMT,
  NT_A,
  NT_COUNT
} NonTerminal;

typedef enum { SYMBOL_TERMINAL, SYMBOL_NON_TERMINAL } SymbolType;

typedef struct {
  SymbolType type;
  union {
    TokenType terminal;
    NonTerminal nonTerminal;
  } symbol;
} GrammarSymbol;

typedef struct {
  NonTerminal lhs;
  GrammarSymbol rhs[MAX_RHS];
  int rhsCount;
  int ruleNumber;
} GrammarRule;

typedef struct {
  GrammarRule rules[MAX_RULES];
  int ruleCount;
  NonTerminal startSymbol;
} grammar;

typedef struct {
  NonTerminal nonTerminal;
  TokenType first[MAX_FIRST_FOLLOW];
  int firstCount;
  int hasEpsilon;
} FirstSet;

typedef struct {
  NonTerminal nonTerminal;
  TokenType follow[MAX_FIRST_FOLLOW];
  int followCount;
} FollowSet;

typedef struct {
  FirstSet firstSets[MAX_NT];
  int firstCount;
  FollowSet followSets[MAX_NT];
  int followCount;
} FirstAndFollow;

typedef struct {
  int isValid;
  int ruleNumber;
  GrammarRule *rule;
} ParseTableEntry;

typedef struct {
  ParseTableEntry entries[MAX_NT][TABLE_TERMINALS];
} table;

typedef struct parseTreeNode {
  GrammarSymbol symbol;
  tokenInfo token;
  struct parseTreeNode *children[MAX_CHILDREN];
  int childCount;
  struct parseTreeNode *parent;
  int isLeaf;
  int visited;
} parseTreeNode;

typedef struct {
  parseTreeNode *root;
} parseTree;

typedef struct stackNode {
  GrammarSymbol symbol;
  parseTreeNode *treeNode;
  struct stackNode *next;
} stackNode;

#endif
