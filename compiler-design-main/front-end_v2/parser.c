#include "parser.h"
#include "parserDef.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const char *getNonTerminalName(NonTerminal nt) {
  switch (nt) {
  case NT_PROGRAM:
    return "<program>";
  case NT_MAINFUNCTION:
    return "<mainFunction>";
  case NT_OTHERFUNCTIONS:
    return "<otherFunctions>";
  case NT_FUNCTION:
    return "<function>";
  case NT_INPUT_PAR:
    return "<input_par>";
  case NT_OUTPUT_PAR:
    return "<output_par>";
  case NT_PARAMETER_LIST:
    return "<parameter_list>";
  case NT_DATATYPE:
    return "<dataType>";
  case NT_PRIMITIVEDATATYPE:
    return "<primitiveDatatype>";
  case NT_CONSTRUCTEDDATATYPE:
    return "<constructedDatatype>";
  case NT_REMAINING_LIST:
    return "<remaining_list>";
  case NT_STMTS:
    return "<stmts>";
  case NT_TYPEDEFINITIONS:
    return "<typeDefinitions>";
  case NT_ACTUALORREDEFINED:
    return "<actualOrRedefined>";
  case NT_TYPEDEFINITION:
    return "<typeDefinition>";
  case NT_FIELDDEFINITIONS:
    return "<fieldDefinitions>";
  case NT_FIELDTYPE:
    return "<fieldType>";
  case NT_FIELDDEFINITION:
    return "<fieldDefinition>";
  case NT_MOREFIELDS:
    return "<moreFields>";
  case NT_DECLARATIONS:
    return "<declarations>";
  case NT_DECLARATION:
    return "<declaration>";
  case NT_GLOBAL_OR_NOT:
    return "<global_or_not>";
  case NT_OTHERSTMTS:
    return "<otherStmts>";
  case NT_STMT:
    return "<stmt>";
  case NT_ASSIGNMENTSTMT:
    return "<assignmentStmt>";
  case NT_SINGLEORRECID:
    return "<singleOrRecId>";
  case NT_OPTION_SINGLE_CONSTRUCTED:
    return "<option_single_constructed>";
  case NT_ONEEXPANSION:
    return "<oneExpansion>";
  case NT_MOREEXPANSIONS:
    return "<moreExpansions>";
  case NT_FUNCALLSTMT:
    return "<funCallStmt>";
  case NT_OUTPUTPARAMETERS:
    return "<outputParameters>";
  case NT_INPUTPARAMETERS:
    return "<inputParameters>";
  case NT_ITERATIVESTMT:
    return "<iterativeStmt>";
  case NT_CONDITIONALSTMT:
    return "<conditionalStmt>";
  case NT_ELSEPART:
    return "<elsePart>";
  case NT_IOSTMT:
    return "<ioStmt>";
  case NT_ARITHMETICEXPRESSION:
    return "<arithmeticExpression>";
  case NT_EXPPRIME:
    return "<expPrime>";
  case NT_TERM:
    return "<term>";
  case NT_TERMPRIME:
    return "<termPrime>";
  case NT_FACTOR:
    return "<factor>";
  case NT_HIGHPRECEDENCEOP:
    return "<highPrecedenceOp>";
  case NT_LOWPRECEDENCEOP:
    return "<lowPrecedenceOp>";
  case NT_BOOLEANEXPRESSION:
    return "<booleanExpression>";
  case NT_VAR:
    return "<var>";
  case NT_LOGICALOP:
    return "<logicalOp>";
  case NT_RELATIONALOP:
    return "<relationalOp>";
  case NT_RETURNSTMT:
    return "<returnStmt>";
  case NT_OPTIONALRETURN:
    return "<optionalReturn>";
  case NT_IDLIST:
    return "<idList>";
  case NT_MORE_IDS:
    return "<more_ids>";
  case NT_DEFINETYPESTMT:
    return "<definetypestmt>";
  case NT_A:
    return "<A>";
  default:
    return "<UNKNOWN>";
  }
}


static grammar *G_global;

#define START_RULE(nt)                                                         \
  do {                                                                         \
    G_global->rules[G_global->ruleCount].lhs = (nt);                           \
    G_global->rules[G_global->ruleCount].rhsCount = 0;                         \
    G_global->rules[G_global->ruleCount].ruleNumber = G_global->ruleCount + 1;

#define T(tok)                                                                 \
  do {                                                                         \
    int _i = G_global->rules[G_global->ruleCount].rhsCount++;                  \
    G_global->rules[G_global->ruleCount].rhs[_i].type = SYMBOL_TERMINAL;       \
    G_global->rules[G_global->ruleCount].rhs[_i].symbol.terminal = (tok);      \
  } while (0)

#define NT(nterm)                                                              \
  do {                                                                         \
    int _i = G_global->rules[G_global->ruleCount].rhsCount++;                  \
    G_global->rules[G_global->ruleCount].rhs[_i].type = SYMBOL_NON_TERMINAL;   \
    G_global->rules[G_global->ruleCount].rhs[_i].symbol.nonTerminal = (nterm); \
  } while (0)

#define END_RULE()                                                             \
  G_global->ruleCount++;                                                       \
  }                                                                            \
  while (0)
#define EPS() T(TK_EPSILON)


grammar *initializeGrammar(void) {
  grammar *G = (grammar *)malloc(sizeof(grammar));
  if (!G) {
    fprintf(stderr, "malloc failed for grammar\n");
    exit(1);
  }
  G->ruleCount = 0;
  G->startSymbol = NT_PROGRAM;
  G_global = G;

  START_RULE(NT_PROGRAM);
  NT(NT_OTHERFUNCTIONS);
  NT(NT_MAINFUNCTION);
  END_RULE();

  START_RULE(NT_MAINFUNCTION);
  T(TK_MAIN);
  NT(NT_STMTS);
  T(TK_END);
  END_RULE();

  START_RULE(NT_OTHERFUNCTIONS);
  NT(NT_FUNCTION);
  NT(NT_OTHERFUNCTIONS);
  END_RULE();

  START_RULE(NT_OTHERFUNCTIONS);
  EPS();
  END_RULE();

  START_RULE(NT_FUNCTION);
  T(TK_FUNID);
  NT(NT_INPUT_PAR);
  NT(NT_OUTPUT_PAR);
  T(TK_SEM);
  NT(NT_STMTS);
  T(TK_END);
  END_RULE();

  START_RULE(NT_INPUT_PAR);
  T(TK_INPUT);
  T(TK_PARAMETER);
  T(TK_LIST);
  T(TK_SQL);
  NT(NT_PARAMETER_LIST);
  T(TK_SQR);
  END_RULE();

  START_RULE(NT_OUTPUT_PAR);
  T(TK_OUTPUT);
  T(TK_PARAMETER);
  T(TK_LIST);
  T(TK_SQL);
  NT(NT_PARAMETER_LIST);
  T(TK_SQR);
  END_RULE();

  START_RULE(NT_OUTPUT_PAR);
  EPS();
  END_RULE();

  START_RULE(NT_PARAMETER_LIST);
  NT(NT_DATATYPE);
  T(TK_ID);
  NT(NT_REMAINING_LIST);
  END_RULE();

  START_RULE(NT_DATATYPE);
  NT(NT_PRIMITIVEDATATYPE);
  END_RULE();
  START_RULE(NT_DATATYPE);
  NT(NT_CONSTRUCTEDDATATYPE);
  END_RULE();

  START_RULE(NT_PRIMITIVEDATATYPE);
  T(TK_INT);
  END_RULE();
  START_RULE(NT_PRIMITIVEDATATYPE);
  T(TK_REAL);
  END_RULE();

  START_RULE(NT_CONSTRUCTEDDATATYPE);
  T(TK_RECORD);
  T(TK_RUID);
  END_RULE();
  START_RULE(NT_CONSTRUCTEDDATATYPE);
  T(TK_UNION);
  T(TK_RUID);
  END_RULE();
  START_RULE(NT_CONSTRUCTEDDATATYPE);
  T(TK_RUID);
  END_RULE();

  START_RULE(NT_REMAINING_LIST);
  T(TK_COMMA);
  NT(NT_PARAMETER_LIST);
  END_RULE();
  START_RULE(NT_REMAINING_LIST);
  EPS();
  END_RULE();

  START_RULE(NT_STMTS);
  NT(NT_TYPEDEFINITIONS);
  NT(NT_DECLARATIONS);
  NT(NT_OTHERSTMTS);
  NT(NT_RETURNSTMT);
  END_RULE();

  START_RULE(NT_TYPEDEFINITIONS);
  NT(NT_ACTUALORREDEFINED);
  NT(NT_TYPEDEFINITIONS);
  END_RULE();
  START_RULE(NT_TYPEDEFINITIONS);
  EPS();
  END_RULE();

  START_RULE(NT_ACTUALORREDEFINED);
  NT(NT_TYPEDEFINITION);
  END_RULE();
  START_RULE(NT_ACTUALORREDEFINED);
  NT(NT_DEFINETYPESTMT);
  END_RULE();

  START_RULE(NT_TYPEDEFINITION);
  T(TK_RECORD);
  T(TK_RUID);
  NT(NT_FIELDDEFINITIONS);
  T(TK_ENDRECORD);
  END_RULE();

  START_RULE(NT_TYPEDEFINITION);
  T(TK_UNION);
  T(TK_RUID);
  NT(NT_FIELDDEFINITIONS);
  T(TK_ENDUNION);
  END_RULE();

  START_RULE(NT_FIELDDEFINITIONS);
  NT(NT_FIELDDEFINITION);
  NT(NT_FIELDDEFINITION);
  NT(NT_MOREFIELDS);
  END_RULE();

  START_RULE(NT_FIELDDEFINITION);
  T(TK_TYPE);
  NT(NT_FIELDTYPE);
  T(TK_COLON);
  T(TK_FIELDID);
  T(TK_SEM);
  END_RULE();

  START_RULE(NT_FIELDTYPE);
  NT(NT_PRIMITIVEDATATYPE);
  END_RULE();
  START_RULE(NT_FIELDTYPE);
  T(TK_RUID);
  END_RULE();

  START_RULE(NT_MOREFIELDS);
  NT(NT_FIELDDEFINITION);
  NT(NT_MOREFIELDS);
  END_RULE();
  START_RULE(NT_MOREFIELDS);
  EPS();
  END_RULE();

  START_RULE(NT_DECLARATIONS);
  NT(NT_DECLARATION);
  NT(NT_DECLARATIONS);
  END_RULE();
  START_RULE(NT_DECLARATIONS);
  EPS();
  END_RULE();

  START_RULE(NT_DECLARATION);
  T(TK_TYPE);
  NT(NT_DATATYPE);
  T(TK_COLON);
  T(TK_ID);
  NT(NT_GLOBAL_OR_NOT);
  T(TK_SEM);
  END_RULE();

  START_RULE(NT_GLOBAL_OR_NOT);
  T(TK_COLON);
  T(TK_GLOBAL);
  END_RULE();
  START_RULE(NT_GLOBAL_OR_NOT);
  EPS();
  END_RULE();

  START_RULE(NT_OTHERSTMTS);
  NT(NT_STMT);
  NT(NT_OTHERSTMTS);
  END_RULE();
  START_RULE(NT_OTHERSTMTS);
  EPS();
  END_RULE();

  START_RULE(NT_STMT);
  NT(NT_ASSIGNMENTSTMT);
  END_RULE();
  START_RULE(NT_STMT);
  NT(NT_ITERATIVESTMT);
  END_RULE();
  START_RULE(NT_STMT);
  NT(NT_CONDITIONALSTMT);
  END_RULE();
  START_RULE(NT_STMT);
  NT(NT_IOSTMT);
  END_RULE();
  START_RULE(NT_STMT);
  NT(NT_FUNCALLSTMT);
  END_RULE();

  START_RULE(NT_ASSIGNMENTSTMT);
  NT(NT_SINGLEORRECID);
  T(TK_ASSIGNOP);
  NT(NT_ARITHMETICEXPRESSION);
  T(TK_SEM);
  END_RULE();

  START_RULE(NT_SINGLEORRECID);
  T(TK_ID);
  NT(NT_OPTION_SINGLE_CONSTRUCTED);
  END_RULE();

  START_RULE(NT_OPTION_SINGLE_CONSTRUCTED);
  EPS();
  END_RULE();
  START_RULE(NT_OPTION_SINGLE_CONSTRUCTED);
  NT(NT_ONEEXPANSION);
  NT(NT_MOREEXPANSIONS);
  END_RULE();

  START_RULE(NT_ONEEXPANSION);
  T(TK_DOT);
  T(TK_FIELDID);
  END_RULE();

  START_RULE(NT_MOREEXPANSIONS);
  NT(NT_ONEEXPANSION);
  NT(NT_MOREEXPANSIONS);
  END_RULE();
  START_RULE(NT_MOREEXPANSIONS);
  EPS();
  END_RULE();

  START_RULE(NT_FUNCALLSTMT);
  NT(NT_OUTPUTPARAMETERS);
  T(TK_CALL);
  T(TK_FUNID);
  T(TK_WITH);
  T(TK_PARAMETERS);
  NT(NT_INPUTPARAMETERS);
  T(TK_SEM);
  END_RULE();

  START_RULE(NT_OUTPUTPARAMETERS);
  T(TK_SQL);
  NT(NT_IDLIST);
  T(TK_SQR);
  T(TK_ASSIGNOP);
  END_RULE();
  START_RULE(NT_OUTPUTPARAMETERS);
  EPS();
  END_RULE();

  START_RULE(NT_INPUTPARAMETERS);
  T(TK_SQL);
  NT(NT_IDLIST);
  T(TK_SQR);
  END_RULE();

  START_RULE(NT_ITERATIVESTMT);
  T(TK_WHILE);
  T(TK_OP);
  NT(NT_BOOLEANEXPRESSION);
  T(TK_CL);
  NT(NT_STMT);
  NT(NT_OTHERSTMTS);
  T(TK_ENDWHILE);
  END_RULE();

  START_RULE(NT_CONDITIONALSTMT);
  T(TK_IF);
  T(TK_OP);
  NT(NT_BOOLEANEXPRESSION);
  T(TK_CL);
  T(TK_THEN);
  NT(NT_STMT);
  NT(NT_OTHERSTMTS);
  NT(NT_ELSEPART);
  END_RULE();

  START_RULE(NT_ELSEPART);
  T(TK_ELSE);
  NT(NT_STMT);
  NT(NT_OTHERSTMTS);
  T(TK_ENDIF);
  END_RULE();
  START_RULE(NT_ELSEPART);
  T(TK_ENDIF);
  END_RULE();

  START_RULE(NT_IOSTMT);
  T(TK_READ);
  T(TK_OP);
  NT(NT_SINGLEORRECID);
  T(TK_CL);
  T(TK_SEM);
  END_RULE();
  START_RULE(NT_IOSTMT);
  T(TK_WRITE);
  T(TK_OP);
  NT(NT_VAR);
  T(TK_CL);
  T(TK_SEM);
  END_RULE();

  START_RULE(NT_ARITHMETICEXPRESSION);
  NT(NT_TERM);
  NT(NT_EXPPRIME);
  END_RULE();

  START_RULE(NT_EXPPRIME);
  NT(NT_LOWPRECEDENCEOP);
  NT(NT_TERM);
  NT(NT_EXPPRIME);
  END_RULE();
  START_RULE(NT_EXPPRIME);
  EPS();
  END_RULE();

  START_RULE(NT_TERM);
  NT(NT_FACTOR);
  NT(NT_TERMPRIME);
  END_RULE();

  START_RULE(NT_TERMPRIME);
  NT(NT_HIGHPRECEDENCEOP);
  NT(NT_FACTOR);
  NT(NT_TERMPRIME);
  END_RULE();
  START_RULE(NT_TERMPRIME);
  EPS();
  END_RULE();

  START_RULE(NT_FACTOR);
  T(TK_OP);
  NT(NT_ARITHMETICEXPRESSION);
  T(TK_CL);
  END_RULE();
  START_RULE(NT_FACTOR);
  NT(NT_VAR);
  END_RULE();

  START_RULE(NT_HIGHPRECEDENCEOP);
  T(TK_MUL);
  END_RULE();
  START_RULE(NT_HIGHPRECEDENCEOP);
  T(TK_DIV);
  END_RULE();

  START_RULE(NT_LOWPRECEDENCEOP);
  T(TK_PLUS);
  END_RULE();
  START_RULE(NT_LOWPRECEDENCEOP);
  T(TK_MINUS);
  END_RULE();

  START_RULE(NT_BOOLEANEXPRESSION);
  T(TK_OP);
  NT(NT_BOOLEANEXPRESSION);
  T(TK_CL);
  NT(NT_LOGICALOP);
  T(TK_OP);
  NT(NT_BOOLEANEXPRESSION);
  T(TK_CL);
  END_RULE();

  START_RULE(NT_BOOLEANEXPRESSION);
  NT(NT_VAR);
  NT(NT_RELATIONALOP);
  NT(NT_VAR);
  END_RULE();

  START_RULE(NT_BOOLEANEXPRESSION);
  T(TK_NOT);
  T(TK_OP);
  NT(NT_BOOLEANEXPRESSION);
  T(TK_CL);
  END_RULE();

  START_RULE(NT_VAR);
  NT(NT_SINGLEORRECID);
  END_RULE();
  START_RULE(NT_VAR);
  T(TK_NUM);
  END_RULE();
  START_RULE(NT_VAR);
  T(TK_RNUM);
  END_RULE();

  START_RULE(NT_LOGICALOP);
  T(TK_AND);
  END_RULE();
  START_RULE(NT_LOGICALOP);
  T(TK_OR);
  END_RULE();

  START_RULE(NT_RELATIONALOP);
  T(TK_LT);
  END_RULE();
  START_RULE(NT_RELATIONALOP);
  T(TK_LE);
  END_RULE();
  START_RULE(NT_RELATIONALOP);
  T(TK_EQ);
  END_RULE();
  START_RULE(NT_RELATIONALOP);
  T(TK_GT);
  END_RULE();
  START_RULE(NT_RELATIONALOP);
  T(TK_GE);
  END_RULE();
  START_RULE(NT_RELATIONALOP);
  T(TK_NE);
  END_RULE();

  START_RULE(NT_RETURNSTMT);
  T(TK_RETURN);
  NT(NT_OPTIONALRETURN);
  T(TK_SEM);
  END_RULE();

  START_RULE(NT_OPTIONALRETURN);
  T(TK_SQL);
  NT(NT_IDLIST);
  T(TK_SQR);
  END_RULE();
  START_RULE(NT_OPTIONALRETURN);
  EPS();
  END_RULE();

  START_RULE(NT_IDLIST);
  T(TK_ID);
  NT(NT_MORE_IDS);
  END_RULE();

  START_RULE(NT_MORE_IDS);
  T(TK_COMMA);
  NT(NT_IDLIST);
  END_RULE();
  START_RULE(NT_MORE_IDS);
  EPS();
  END_RULE();

  START_RULE(NT_DEFINETYPESTMT);
  T(TK_DEFINETYPE);
  NT(NT_A);
  T(TK_RUID);
  T(TK_AS);
  T(TK_RUID);
  END_RULE();

  START_RULE(NT_A);
  T(TK_RECORD);
  END_RULE();
  START_RULE(NT_A);
  T(TK_UNION);
  END_RULE();

  return G;
}

static void addToFirst(FirstSet *s, TokenType t) {
  for (int i = 0; i < s->firstCount; i++)
    if (s->first[i] == t)
      return;
  if (s->firstCount < MAX_FIRST_FOLLOW)
    s->first[s->firstCount++] = t;
}

static void addToFollow(FollowSet *s, TokenType t) {
  for (int i = 0; i < s->followCount; i++)
    if (s->follow[i] == t)
      return;
  if (s->followCount < MAX_FIRST_FOLLOW)
    s->follow[s->followCount++] = t;
}

static FirstSet *getFirstSet(NonTerminal nt, FirstAndFollow *F) {
  for (int i = 0; i < F->firstCount; i++)
    if (F->firstSets[i].nonTerminal == nt)
      return &F->firstSets[i];
  return NULL;
}

static FollowSet *getFollowSet(NonTerminal nt, FirstAndFollow *F) {
  for (int i = 0; i < F->followCount; i++)
    if (F->followSets[i].nonTerminal == nt)
      return &F->followSets[i];
  return NULL;
}

static void computeFirstOne(NonTerminal nt, grammar *G, FirstAndFollow *F) {
  FirstSet *fs = getFirstSet(nt, F);
  if (!fs)
    return;

  for (int i = 0; i < G->ruleCount; i++) {
    GrammarRule *rule = &G->rules[i];
    if (rule->lhs != nt)
      continue;

    if (rule->rhsCount == 1 && rule->rhs[0].type == SYMBOL_TERMINAL &&
        rule->rhs[0].symbol.terminal == TK_EPSILON) {
      fs->hasEpsilon = 1;
      continue;
    }

    int allEps = 1;
    for (int j = 0; j < rule->rhsCount; j++) {
      GrammarSymbol *sym = &rule->rhs[j];
      if (sym->type == SYMBOL_TERMINAL) {
        addToFirst(fs, sym->symbol.terminal);
        allEps = 0;
        break;
      } else {
        FirstSet *sf = getFirstSet(sym->symbol.nonTerminal, F);
        if (sf) {
          for (int k = 0; k < sf->firstCount; k++)
            addToFirst(fs, sf->first[k]);
          if (!sf->hasEpsilon) {
            allEps = 0;
            break;
          }
        } else {
          allEps = 0;
          break;
        }
      }
    }
    if (allEps)
      fs->hasEpsilon = 1;
  }
}

static void computeFollowOne(NonTerminal nt, grammar *G, FirstAndFollow *F) {
  if (nt == G->startSymbol) {
    FollowSet *fw = getFollowSet(nt, F);
    if (fw)
      addToFollow(fw, TK_EOF);
  }
  for (int i = 0; i < G->ruleCount; i++) {
    GrammarRule *rule = &G->rules[i];
    for (int j = 0; j < rule->rhsCount; j++) {
      if (rule->rhs[j].type != SYMBOL_NON_TERMINAL)
        continue;
      if (rule->rhs[j].symbol.nonTerminal != nt)
        continue;

      FollowSet *fw = getFollowSet(nt, F);
      if (!fw)
        continue;

      int allBetaEps = 1;
      for (int k = j + 1; k < rule->rhsCount; k++) {
        GrammarSymbol *beta = &rule->rhs[k];
        if (beta->type == SYMBOL_TERMINAL) {
          if (beta->symbol.terminal != TK_EPSILON)
            addToFollow(fw, beta->symbol.terminal);
          allBetaEps = 0;
          break;
        } else {
          FirstSet *bf = getFirstSet(beta->symbol.nonTerminal, F);
          if (bf) {
            for (int m = 0; m < bf->firstCount; m++)
              addToFollow(fw, bf->first[m]);
            if (!bf->hasEpsilon) {
              allBetaEps = 0;
              break;
            }
          } else {
            allBetaEps = 0;
            break;
          }
        }
      }
      if (allBetaEps) {
        FollowSet *lf = getFollowSet(rule->lhs, F);
        if (lf)
          for (int k = 0; k < lf->followCount; k++)
            addToFollow(fw, lf->follow[k]);
      }
    }
  }
}

FirstAndFollow computeFirstAndFollowSets(grammar *G) {
  FirstAndFollow F;
  F.firstCount = 0;
  F.followCount = 0;

  for (int i = 0; i < (int)NT_COUNT; i++) {
    F.firstSets[F.firstCount].nonTerminal = (NonTerminal)i;
    F.firstSets[F.firstCount].firstCount = 0;
    F.firstSets[F.firstCount].hasEpsilon = 0;
    F.firstCount++;
    F.followSets[F.followCount].nonTerminal = (NonTerminal)i;
    F.followSets[F.followCount].followCount = 0;
    F.followCount++;
  }

  for (int iter = 0; iter < 50; iter++) {
    int oldTotal = 0, newTotal = 0;
    for (int i = 0; i < F.firstCount; i++)
      oldTotal += F.firstSets[i].firstCount;
    for (int i = 0; i < (int)NT_COUNT; i++)
      computeFirstOne((NonTerminal)i, G, &F);
    for (int i = 0; i < F.firstCount; i++)
      newTotal += F.firstSets[i].firstCount;
    if (newTotal == oldTotal)
      break;
  }

  for (int iter = 0; iter < 50; iter++) {
    int oldTotal = 0, newTotal = 0;
    for (int i = 0; i < F.followCount; i++)
      oldTotal += F.followSets[i].followCount;
    for (int i = 0; i < (int)NT_COUNT; i++)
      computeFollowOne((NonTerminal)i, G, &F);
    for (int i = 0; i < F.followCount; i++)
      newTotal += F.followSets[i].followCount;
    if (newTotal == oldTotal)
      break;
  }

  return F;
}

void createParseTable(FirstAndFollow *F, table *T, grammar *G) {
  for (int i = 0; i < MAX_NT; i++)
    for (int j = 0; j < TABLE_TERMINALS; j++) {
      T->entries[i][j].isValid = 0;
      T->entries[i][j].ruleNumber = -1;
      T->entries[i][j].rule = NULL;
    }

  for (int ri = 0; ri < G->ruleCount; ri++) {
    GrammarRule *rule = &G->rules[ri];
    NonTerminal A = rule->lhs;

    FirstSet alpha;
    alpha.firstCount = 0;
    alpha.hasEpsilon = 0;

    int allEps = 1;
    for (int j = 0; j < rule->rhsCount; j++) {
      GrammarSymbol *sym = &rule->rhs[j];
      if (sym->type == SYMBOL_TERMINAL) {
        if (sym->symbol.terminal == TK_EPSILON)
          alpha.hasEpsilon = 1;
        else
          addToFirst(&alpha, sym->symbol.terminal);
        allEps = 0;
        break;
      } else {
        FirstSet *sf = getFirstSet(sym->symbol.nonTerminal, F);
        if (sf) {
          for (int k = 0; k < sf->firstCount; k++)
            addToFirst(&alpha, sf->first[k]);
          if (!sf->hasEpsilon) {
            allEps = 0;
            break;
          }
        } else {
          allEps = 0;
          break;
        }
      }
    }
    if (allEps)
      alpha.hasEpsilon = 1;

    for (int j = 0; j < alpha.firstCount; j++) {
      int a = (int)alpha.first[j];
      if (a >= 0 && a < TABLE_TERMINALS) {
        if (!T->entries[A][a].isValid) {
          T->entries[A][a].isValid = 1;
          T->entries[A][a].ruleNumber = rule->ruleNumber;
          T->entries[A][a].rule = rule;
        }
      }
    }

    if (alpha.hasEpsilon) {
      FollowSet *fw = getFollowSet(A, F);
      if (fw) {
        for (int j = 0; j < fw->followCount; j++) {
          int b = (int)fw->follow[j];
          if (b >= 0 && b < TABLE_TERMINALS) {
            if (!T->entries[A][b].isValid) {
              T->entries[A][b].isValid = 1;
              T->entries[A][b].ruleNumber = rule->ruleNumber;
              T->entries[A][b].rule = rule;
            }
          }
        }
      }
    }
  }
}

static void push(stackNode **top, GrammarSymbol sym, parseTreeNode *tn) {
  stackNode *n = (stackNode *)malloc(sizeof(stackNode));
  if (!n) {
    fprintf(stderr, "malloc failed in push\n");
    exit(1);
  }
  n->symbol = sym;
  n->treeNode = tn;
  n->next = *top;
  *top = n;
}

static stackNode *pop(stackNode **top) {
  if (!*top)
    return NULL;
  stackNode *n = *top;
  *top = (*top)->next;
  return n;
}

static parseTreeNode *createNode(GrammarSymbol sym) {
  parseTreeNode *n = (parseTreeNode *)calloc(1, sizeof(parseTreeNode));
  if (!n) {
    fprintf(stderr, "calloc failed in createNode\n");
    exit(1);
  }
  n->symbol = sym;
  n->isLeaf = (sym.type == SYMBOL_TERMINAL) ? 1 : 0;
  return n;
}

static void addChild(parseTreeNode *parent, parseTreeNode *child) {
  if (parent && child && parent->childCount < MAX_CHILDREN) {
    parent->children[parent->childCount++] = child;
    child->parent = parent;
  }
}

static int isSyncToken(TokenType t) {
  return (t == TK_SEM || t == TK_ENDRECORD || t == TK_ENDUNION ||
          t == TK_ENDIF || t == TK_ENDWHILE || t == TK_ELSE || t == TK_CL ||
          t == TK_SQR || t == TK_END || t == TK_EOF);
}

static tokenInfo nextMeaningfulToken(twinBuffer *tb, int *errorCount,
                                     int *syntaxOK) {
  tokenInfo cur;
  do {
    cur = getNextToken(tb);
    if (cur.tokenType == TK_COMMENT)
      continue;
    if (cur.tokenType == TK_ERROR) {
      fprintf(stderr, "Line %d\tError: %s\n", cur.lineNumber,
              cur.errorMsg[0] ? cur.errorMsg : cur.lexeme);
      (*errorCount)++;
      *syntaxOK = 0;
    }
  } while (cur.tokenType == TK_COMMENT || cur.tokenType == TK_ERROR);
  return cur;
}

parseTree *parseInputSourceCode(char *testcaseFile, table *T, grammar *G) {
  FILE *fp = fopen(testcaseFile, "r");
  if (!fp) {
    fprintf(stderr, "Error: Cannot open %s\n", testcaseFile);
    return NULL;
  }

  twinBuffer *tb = initializeTwinBuffer(fp);
  parseTree *PT = (parseTree *)calloc(1, sizeof(parseTree));
  if (!PT) {
    fprintf(stderr, "calloc failed for parseTree\n");
    exit(1);
  }

  GrammarSymbol start;
  start.type = SYMBOL_NON_TERMINAL;
  start.symbol.nonTerminal = G->startSymbol;
  PT->root = createNode(start);

  stackNode *stack = NULL;
  GrammarSymbol dollar;
  dollar.type = SYMBOL_TERMINAL;
  dollar.symbol.terminal = TK_EOF;
  push(&stack, dollar, NULL);
  push(&stack, start, PT->root);

  int errorCount = 0;
  int syntaxOK = 1;
  tokenInfo cur = nextMeaningfulToken(tb, &errorCount, &syntaxOK);

  int lastErrorLine = -1;

  while (stack != NULL && errorCount < 60) {
    stackNode *topNode = pop(&stack);
    GrammarSymbol X = topNode->symbol;
    parseTreeNode *tn = topNode->treeNode;
    free(topNode);

    if (X.type == SYMBOL_TERMINAL && X.symbol.terminal == TK_EPSILON)
      continue;

    if (X.type == SYMBOL_TERMINAL && X.symbol.terminal == TK_EOF) {
      if (cur.tokenType != TK_EOF) {
        if (cur.lineNumber != lastErrorLine) {
          fprintf(stderr,
                  "Line %d\tError: Unexpected token %s encountered at end\n",
                  cur.lineNumber, getTokenName(cur.tokenType));
          lastErrorLine = cur.lineNumber;
          errorCount++;
          syntaxOK = 0;
        }
      }
      break;
    }

    if (X.type == SYMBOL_TERMINAL) {
      if (X.symbol.terminal == cur.tokenType) {
        if (tn) {
          tn->token = cur;
          tn->isLeaf = 1;
        }
        cur = nextMeaningfulToken(tb, &errorCount, &syntaxOK);
      } else {
     
        if (cur.lineNumber != lastErrorLine) {
          fprintf(stderr,
                  "Line %d\tError: The token %s for lexeme %s  does not match "
                  "with the expected token %s\n",
                  cur.lineNumber, getTokenName(cur.tokenType), cur.lexeme,
                  getTokenName(X.symbol.terminal));
          lastErrorLine = cur.lineNumber;
          errorCount++;
          syntaxOK = 0;
        }
      }
    } else {
      NonTerminal A = X.symbol.nonTerminal;
      int a = (int)cur.tokenType;

      if (a >= 0 && a < TABLE_TERMINALS && T->entries[A][a].isValid) {
        GrammarRule *rule = T->entries[A][a].rule;
        parseTreeNode *children[MAX_RHS];
        int nc = 0;

        for (int i = 0; i < rule->rhsCount; i++) {
          children[nc] = createNode(rule->rhs[i]);
          nc++;
        }
        for (int i = 0; i < nc; i++)
          addChild(tn, children[i]);

        for (int i = nc - 1; i >= 0; i--)
          push(&stack, rule->rhs[i], children[i]);

      } else {
        if (cur.tokenType != TK_EOF && cur.lineNumber != lastErrorLine) {
          fprintf(stderr,
                  "Line %d\tError: Invalid token %s encountered with value %s "
                  "stack top %s\n",
                  cur.lineNumber, getTokenName(a), cur.lexeme,
                  getNonTerminalName(A) + 1); 
          lastErrorLine = cur.lineNumber;
          errorCount++;
          syntaxOK = 0;
        }

        if (!isSyncToken(cur.tokenType)) {
          while (cur.tokenType != TK_EOF && !isSyncToken(cur.tokenType)) {
            cur = nextMeaningfulToken(tb, &errorCount, &syntaxOK);
          }
        }
      }
    }
  }

  while (stack) {
    stackNode *n = pop(&stack);
    free(n);
  }

  freeTwinBuffer(tb);

  if (syntaxOK && errorCount == 0) {
    printf("Input source code is syntactically correct...........\n");
  }

  return PT;
}

static void printInorder(parseTreeNode *node, FILE *fp) {
  if (!node)
    return;

  char parentStr[64];
  if (node->parent == NULL) {
    strcpy(parentStr, "ROOT");
  } else if (node->parent->symbol.type == SYMBOL_NON_TERMINAL) {
    const char *full =
        getNonTerminalName(node->parent->symbol.symbol.nonTerminal);
    int len = (int)strlen(full);
    if (len >= 2 && full[0] == '<' && full[len - 1] == '>') {
      strncpy(parentStr, full + 1, len - 2);
      parentStr[len - 2] = '\0';
    } else {
      strncpy(parentStr, full, 63);
      parentStr[63] = '\0';
    }
  } else {
    strncpy(parentStr, getTokenName(node->parent->symbol.symbol.terminal), 63);
    parentStr[63] = '\0';
  }

  if (node->isLeaf || node->childCount == 0) {
    char valueStr[32];
    if (node->token.tokenType == TK_NUM && node->token.hasValue)
      snprintf(valueStr, sizeof(valueStr), "%d", node->token.value.intValue);
    else if (node->token.tokenType == TK_RNUM && node->token.hasValue)
      snprintf(valueStr, sizeof(valueStr), "%.2f", node->token.value.realValue);
    else
      strcpy(valueStr, "----");

    const char *lex =
        (node->token.lexeme[0] != '\0') ? node->token.lexeme : "----";

    const char *currentNode = getTokenName(node->token.tokenType);

    fprintf(fp, "%-20s  %-30s  %-6d  %-22s  %-12s  %-30s  %-5s  %-s\n", lex,
            currentNode, node->token.lineNumber,
            getTokenName(node->token.tokenType), valueStr, parentStr, "yes",
            "----");
  } else {
    const char *fullNT = getNonTerminalName(node->symbol.symbol.nonTerminal);
    char ntStr[64];
    int len = (int)strlen(fullNT);
    if (len >= 2 && fullNT[0] == '<' && fullNT[len - 1] == '>') {
      strncpy(ntStr, fullNT + 1, len - 2);
      ntStr[len - 2] = '\0';
    } else {
      strncpy(ntStr, fullNT, 63);
      ntStr[63] = '\0';
    }

    if (node->childCount > 0) {
      printInorder(node->children[0], fp);
    }

    fprintf(fp, "%-20s  %-30s  %-6s  %-22s  %-12s  %-30s  %-5s  %-s\n", "----",
            ntStr, "----", "----", "----", parentStr, "no", ntStr);

    for (int i = 1; i < node->childCount; i++) {
      printInorder(node->children[i], fp);
    }
  }
}

void printParseTree(parseTree *PT, char *outfile) {
  if (!PT || !PT->root) {
    fprintf(stderr, "Warning: empty parse tree, nothing to print\n");
    return;
  }

  FILE *fp = fopen(outfile, "w");
  if (!fp) {
    fprintf(stderr, "Error: Cannot open output file %s\n", outfile);
    return;
  }

  fprintf(fp, "%-20s  %-30s  %-6s  %-22s  %-12s  %-30s  %-5s  %-s\n", "lexeme",
          "CurrentNode", "lineno", "tokenName", "valueIfNumber",
          "parentNodeSymbol", "isLeaf", "NodeSymbol");
  fprintf(fp, "%s\n",
          "--------------------------------------------------------------------"
          "---------------------------------"
          "-----------------------------------------------------");

  printInorder(PT->root, fp);
  fclose(fp);
  printf("Parse tree written to %s\n", outfile);
}

static void freeNode(parseTreeNode *node) {
  if (!node)
    return;
  for (int i = 0; i < node->childCount; i++)
    freeNode(node->children[i]);
  free(node);
}

void freeParseTree(parseTree *PT) {
  if (!PT)
    return;
  freeNode(PT->root);
  free(PT);
}

void printFirstAndFollow(FirstAndFollow *F) {
  printf("\n========== FIRST SETS ==========\n");
  for (int i = 0; i < F->firstCount; i++) {
    FirstSet *fs = &F->firstSets[i];
    printf("FIRST(%-35s) = { ", getNonTerminalName(fs->nonTerminal));
    for (int j = 0; j < fs->firstCount; j++)
      printf("%s ", getTokenName(fs->first[j]));
    if (fs->hasEpsilon)
      printf("EPSILON ");
    printf("}\n");
  }
  printf("\n========== FOLLOW SETS ==========\n");
  for (int i = 0; i < F->followCount; i++) {
    FollowSet *fw = &F->followSets[i];
    printf("FOLLOW(%-35s) = { ", getNonTerminalName(fw->nonTerminal));
    for (int j = 0; j < fw->followCount; j++)
      printf("%s ", getTokenName(fw->follow[j]));
    printf("}\n");
  }
}

void printParseTable(table *T, grammar *G) {
  printf("\n========== PARSE TABLE (non-empty entries) ==========\n");
  for (int i = 0; i < (int)NT_COUNT; i++) {
    for (int j = 0; j < TABLE_TERMINALS; j++) {
      if (T->entries[i][j].isValid) {
        printf("T[%-35s][%-15s] = Rule %d\n",
               getNonTerminalName((NonTerminal)i), getTokenName((TokenType)j),
               T->entries[i][j].ruleNumber);
      }
    }
  }
  (void)G; 
}
