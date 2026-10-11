#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void printImplementationStatus(void);
void printMenu(void);
void optionCommentRemoval(char *sourceFile);
void optionPrintTokens(char *sourceFile);
void optionParsing(char *sourceFile, char *parseTreeFile);
void optionTiming(char *sourceFile);

int main(int argc, char *argv[])
{
  printImplementationStatus();

  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s <source_file.txt> <parsetree_output.txt>\n",
            argv[0]);
    fprintf(stderr, "Example: ./stage1exe testcase.txt parsetreeOutFile.txt\n");
    return 1;
  }

  char *sourceFile = argv[1];
  char *parseTreeFile = argv[2];

  FILE *check = fopen(sourceFile, "r");
  if (!check)
  {
    fprintf(stderr, "Error: Cannot open source file %s\n", sourceFile);
    return 1;
  }
  fclose(check);

  int option;
  printf("COMPILER FRONT-END\n");

  do
  {
    printMenu();
    printf("Enter option: ");

    int scanResult = scanf("%d", &option);

    if (scanResult == EOF)
    {
      break;
    }

    if (scanResult != 1)
    {
      int c;
      while ((c = getchar()) != '\n' && c != EOF)
        ;
      printf("Invalid input. Please enter a number.\n");
      continue;
    }

    int temp;
    while ((temp = getchar()) != '\n' && temp != EOF)
      ;

    printf("\n");

    switch (option)
    {
    case 0:
      printf("Exiting compiler...\n");
      break;

    case 1:
      printf(" COMMENT REMOVAL \n");
      optionCommentRemoval(sourceFile);
      break;

    case 2:
      printf(" LEXICAL ANALYSIS \n");
      optionPrintTokens(sourceFile);
      break;

    case 3:
      printf(" SYNTAX ANALYSIS \n");
      optionParsing(sourceFile, parseTreeFile);
      break;

    case 4:
      printf(" PERFORMANCE TIMING \n");
      optionTiming(sourceFile);
      break;

    default:
      printf("Invalid option. Please choose 0-4.\n");
      break;
    }

    if (option != 0)
    {
      if (feof(stdin))
      {
        break;
      }

      printf("\nPress Enter to continue...");
      int c;
      while ((c = getchar()) != '\n' && c != EOF)
        ;
    }

  } while (option != 0);

  printf("\nThank you for using our compiler!\n");
  return 0;
}

void printImplementationStatus(void)
{
  printf("\n IMPLEMENTATION STATUS \n");
  printf("(a) FIRST and FOLLOW set automated:              YES\n");
  printf("(b) Only Lexical analyzer module developed:      NO\n");
  printf("(c) Both lexical and syntax analysis modules:    YES\n");
  printf("(d) Modules compile and run:                     YES\n");
  printf("(e) Modules work with all given test cases:      YES\n");
  printf("(f) Parse tree constructed:                      YES\n");
}

void printMenu(void)
{
  printf("\n MENU \n");
  printf("0 : Exit\n");
  printf("1 : Remove comments and print clean code on console\n");
  printf("2 : Print token list (lexical analysis)\n");
  printf("3 : Parse and verify syntax (produces parse tree file)\n");
  printf("4 : Print total time taken by lexer and parser\n");
}

void optionCommentRemoval(char *sourceFile)
{
  char cleanFile[] = "clean_code.txt";
  removeComments(sourceFile, cleanFile);

  FILE *fp = fopen(cleanFile, "r");
  if (fp)
  {
    printf("\n CLEAN CODE (comments removed) \n");
    int ch;
    while ((ch = fgetc(fp)) != EOF)
      printf("%c", (char)ch);
    fclose(fp);
  }
}

void optionPrintTokens(char *sourceFile)
{
  FILE *fp = fopen(sourceFile, "r");
  if (!fp)
  {
    fprintf(stderr, "Error: Cannot open file %s\n", sourceFile);
    return;
  }

  twinBuffer *tb = initializeTwinBuffer(fp);

  printf("\n%-10s  %-30s  %-22s  %-8s  %-15s\n", "Line No.", "Lexeme", "Token", "HasVal", "Value");

  tokenInfo token;
  int tokenCount = 0;
  int errorCount = 0;

  do
  {
    token = getNextToken(tb);

    if (token.tokenType == TK_ERROR)
    {
      fprintf(stderr, "Line %d\tError: %s\n", token.lineNumber, token.errorMsg[0] ? token.errorMsg : token.lexeme);
      errorCount++;

      printf("%-10d  %-30s  %-22s  %-8s  %-15s\n", token.lineNumber, token.lexeme, "TK_ERROR", "---", token.errorMsg[0] ? token.errorMsg : "lexical error");
      tokenCount++;
    }
    else if (token.tokenType == TK_EOF)
    {
      break;
    }
    else
    {
      char valueStr[32] = "----";
      char hasValStr[8] = "no";
      if (token.hasValue)
      {
        strcpy(hasValStr, "yes");
        if (token.tokenType == TK_NUM)
          snprintf(valueStr, sizeof(valueStr), "%d", token.value.intValue);
        else if (token.tokenType == TK_RNUM)
          snprintf(valueStr, sizeof(valueStr), "%.4f", token.value.realValue);
      }

      printf("%-10d  %-30s  %-22s  %-8s  %-15s\n", token.lineNumber, token.lexeme, getTokenName(token.tokenType), hasValStr, valueStr);
      tokenCount++;
    }

  } while (token.tokenType != TK_EOF);

  printf("Total tokens listed: %d\n", tokenCount);
  if (errorCount > 0)
    printf("Total lexical errors: %d  (details on stderr)\n", errorCount);

  freeTwinBuffer(tb);
}

void optionParsing(char *sourceFile, char *parseTreeFile)
{
  printf("Initializing grammar...\n");
  grammar *G = initializeGrammar();
  printf("Grammar initialized with %d rules.\n", G->ruleCount);

  printf("Computing FIRST and FOLLOW sets...\n");
  FirstAndFollow F = computeFirstAndFollowSets(G);
  printf("FIRST and FOLLOW sets computed.\n");

  printf("Building predictive parse table...\n");
  table T;
  createParseTable(&F, &T, G);
  printf("Parse table ready.\n\n");

  printf("Parsing: %s\n", sourceFile);
  parseTree *PT = parseInputSourceCode(sourceFile, &T, G);

  if (PT && PT->root)
  {
    printParseTree(PT, parseTreeFile);
    freeParseTree(PT);
  }
  else
  {
    fprintf(stderr, "Parsing failed – no parse tree generated.\n");
  }

  free(G);
}

void optionTiming(char *sourceFile)
{
  clock_t start_time, end_time;
  double total_CPU_time, total_CPU_time_in_seconds;

  start_time = clock();

  grammar *G = initializeGrammar();
  FirstAndFollow F = computeFirstAndFollowSets(G);
  table T;
  createParseTable(&F, &T, G);

  parseTree *PT = parseInputSourceCode(sourceFile, &T, G);

  end_time = clock();

  total_CPU_time = (double)(end_time - start_time);
  total_CPU_time_in_seconds = total_CPU_time / CLOCKS_PER_SEC;

  printf("\n PERFORMANCE METRICS \n");
  printf("Total CPU time (clock ticks) : %.0f\n", total_CPU_time);
  printf("Total CPU time (seconds)     : %.6f\n", total_CPU_time_in_seconds);
  printf("\n");

  if (PT)
    freeParseTree(PT);
  free(G);
}
