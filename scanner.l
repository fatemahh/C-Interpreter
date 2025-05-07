%option yylineno
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"

int col = 1;

void updateColumn() {
    col += yyleng;
}
%}

%x COMMENT

LETTER      [A-Za-z]
DIGIT       [0-9]
ID          {LETTER}({LETTER}|{DIGIT})*([@$_]?{DIGIT}+)?
NUM         {DIGIT}(\.)?{DIGIT}+([Ee][+\-]?{DIGIT}+)?
KEYWORD     ("Program"|"else"|"if"|"int"|"float"|"return"|"void"|"while")
OP_MULTI    (">="|"<="|"=="|"!=")
OP_SINGLE   [><=,;\(\)\{\}\[\]\+\-\*\/]
NUMERR      [^0-9 \t\n><=,;\(\)\{\}\[\]\+\-\*\/]

%%
"/*"                   { BEGIN(COMMENT); updateColumn(); }
<COMMENT>"*/"          { BEGIN(INITIAL); updateColumn(); }
<COMMENT>\n            { col = 1; }
<COMMENT>.             { updateColumn(); }

<<EOF>> {
    if (YY_START == COMMENT) {
        fprintf(stderr, "Lexical Error: EOF reached while comment not closed at line %d, column %d\n", yylineno, col);
        exit(1);
    }
    else {
        return 0; 
    }
}

[ \t]+                 { updateColumn(); }
\n                     { col = 1; }
"\r"                   { yylineno++; col = 1; }

{KEYWORD}              { 
    if (strcmp(yytext, "Program") == 0) {
        return PROGRAM;
    } else if (strcmp(yytext, "if") == 0) {
        return IF;
    } else if (strcmp(yytext, "else") == 0) {
        return ELSE;
    } else if (strcmp(yytext, "int") == 0) {
        return INT;
    } else if (strcmp(yytext, "void") == 0) {
        return VOID;
    } else if (strcmp(yytext, "while") == 0) {
        return WHILE;
    } else if (strcmp(yytext, "float") == 0) {
        return FLOAT;
    }
    updateColumn();
 }

{ID}                   { updateColumn(); return ID;  }
{NUM}                  { updateColumn(); return NUM; }

"{"                    { updateColumn(); return LBRACE; }
"}"                    { updateColumn(); return RBRACE; }
"("                    { updateColumn(); return LPAREN; }
")"                    { updateColumn(); return RPAREN; }
"["                    { updateColumn(); return LBRACKET; }
"]"                    { updateColumn(); return RBRACKET; }
";"                    { updateColumn(); return SEMICOLON; }
","                    { updateColumn(); return COMMA; }
"."                    { updateColumn(); return DOT; }
"="                    { updateColumn(); return ASSIGN; }
"+"                    { updateColumn(); return PLUS; }
"-"                    { updateColumn(); return MINUS; }
"*"                    { updateColumn(); return MUL; }
"/"                    { updateColumn(); return DIV; }
"<="                   { updateColumn(); return LTE; }
"<"                    { updateColumn(); return LT; }
">="                   { updateColumn(); return GTE; }
">"                    { updateColumn(); return GT; }
"=="                   { updateColumn(); return EQ; }
"!="                   { updateColumn(); return NEQ; }

{DIGIT}                { updateColumn(); return NUM; }

{LETTER}({LETTER}|{DIGIT})*"#"({LETTER}|{DIGIT})*([@$_]?{DIGIT}+)? {
    fprintf(stderr, "Lexical Error: Wrong identifier at line %d, column %d: %s\n", yylineno, col, yytext);
    updateColumn();
    exit(1);
}
{LETTER}({LETTER}|{DIGIT})*([@$_]?{DIGIT}*)"#"{DIGIT}* {
    fprintf(stderr, "Lexical Error: Wrong identifier at line %d, column %d: %s\n", yylineno, col, yytext);
    updateColumn();
    exit(1);
}
{DIGIT}(\.)?{DIGIT}+((E|e)("+"|"-")?{NUMERR}*) {
    fprintf(stderr, "Lexical Error: Wrong number format at line %d, column %d: %s\n", yylineno, col, yytext);
    updateColumn();
    exit(1);
}
. {
    fprintf(stderr, "Lexical Error: Unknown character '%s' at line %d, column %d\n", yytext, yylineno, col);
    updateColumn();
    exit(1);
}

%%

int yywrap() {
    return 1;
}