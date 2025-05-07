#ifndef SCANNER_H
#define SCANNER_H

#ifdef __cplusplus
extern "C" {
#endif

enum TokenType {
    /* Keywords */
    PROGRAM, INT, FLOAT, IF, ELSE, WHILE, VOID,
    /* Identifiers and constants */
    ID, NUM,
    /* Special Symbols */
    LBRACE, RBRACE, LBRACKET, RBRACKET, LPAREN, RPAREN,
    SEMICOLON, COMMA, DOT, ASSIGN,
    PLUS, MINUS, MUL, DIV,
    LT, LTE, GT, GTE, EQ, NEQ,
    /* Unknown token */
    UNKNOWN
};

// Declare extern variables that Flex defines
extern char* yytext;    // yytext contains the matched text of the current token
extern int yylineno;    // yylineno contains the current line number

/* 
   addToken: the scanner calls this function each time it recognizes a token.
   Parameters:
       type  - an integer representing the token type (see your TokenType enum)
       value - the token lexeme (a C string)
       line  - the line number on which the token was found.
*/
// void addToken(int type, const char *value, int line);

#ifdef __cplusplus
}
#endif

#endif // SCANNER_H
