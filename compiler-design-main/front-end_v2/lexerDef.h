#ifndef LEXERDEF_H
#define LEXERDEF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 4096
#define TWIN_BUFFER_SIZE (2 * BUFFER_SIZE)
#define MAX_LEXEME_LENGTH 256

typedef enum {
    TK_ASSIGNOP,    // <---
    TK_COMMENT,     // %
    TK_FIELDID,     // [a-z][a-z]*
    TK_ID,          // [b-d][2-7][b-d]*[2-7]*
    TK_NUM,         // [0-9][0-9]*
    TK_RNUM,        // [0-9][0-9]*.[0-9][0-9] with optional E[+|-][0-9][0-9]
    TK_FUNID,       // _[a-z|A-Z][a-z|A-Z]*[0-9]*
    TK_RUID,        // #[a-z][a-z]*
    TK_WITH,        // with
    TK_PARAMETERS,  // parameters
    TK_END,         // end
    TK_WHILE,       // while
    TK_UNION,       // union
    TK_ENDUNION,    // endunion
    TK_DEFINETYPE,  // definetype
    TK_AS,          // as
    TK_TYPE,        // type
    TK_MAIN,        // _main
    TK_GLOBAL,      // global
    TK_PARAMETER,   // parameter
    TK_LIST,        // list
    TK_SQL,         // [
    TK_SQR,         // ]
    TK_INPUT,       // input
    TK_OUTPUT,      // output
    TK_INT,         // int
    TK_REAL,        // real
    TK_COMMA,       // ,
    TK_SEM,         // ;
    TK_COLON,       // :
    TK_DOT,         // .
    TK_ENDWHILE,    // endwhile
    TK_OP,          // (
    TK_CL,          // )
    TK_IF,          // if
    TK_THEN,        // then
    TK_ENDIF,       // endif
    TK_READ,        // read
    TK_WRITE,       // write
    TK_RETURN,      // return
    TK_PLUS,        // +
    TK_MINUS,       // -
    TK_MUL,         // *
    TK_DIV,         // /
    TK_CALL,        // call
    TK_RECORD,      // record
    TK_ENDRECORD,   // endrecord
    TK_ELSE,        // else
    TK_AND,         // &&&
    TK_OR,          // @@@
    TK_NOT,         // ~
    TK_LT,          // <
    TK_LE,          // <=
    TK_EQ,          // ==
    TK_GT,          // >
    TK_GE,          // >=
    TK_NE,          // !=
    TK_EOF,         // End of file
    TK_ERROR,       // Lexical error
    TK_EPSILON      // For grammar purposes
} TokenType;

typedef struct {
    TokenType tokenType;
    char lexeme[MAX_LEXEME_LENGTH];
    int lineNumber;
    union {
        int intValue;
        double realValue;
    } value;
    int hasValue;
    int errorType;
    char errorMsg[256];
} tokenInfo;

typedef struct {
    char buffer[TWIN_BUFFER_SIZE + 1];
    int forward;
    int lexemeBegin;
    int currentBuffer;
    FILE *fp;
    int eof1, eof2;
    int lineNumber;
} twinBuffer;

typedef struct {
    char *keyword;
    TokenType token;
} KeywordEntry;

typedef struct {
    int lineNumber;
    char message[256];
} LexError;

typedef enum {
    ERR_NONE = 0,
    ERR_UNKNOWN_SYMBOL,
    ERR_UNKNOWN_PATTERN,
    ERR_ID_TOO_LONG,
    ERR_FUNID_TOO_LONG
} LexErrorType;

#endif