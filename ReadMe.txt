A Recursive-Descent Parser 

The Rules: 

1- program -> Program ID {declaration-list statement-list}.
2.1 - declaration-list -> declaration declaration-list-tail
2.2 - declaration-list-tail -> declaration declaration-list-tail | ε
3 - declaration -> var-declaration
4.1 - var-declaration -> type-specifier ID var-declaration-tail
4.2 - var-declaration-tail -> ; | [ NUM ] ;
5 - type-specifier -> int | float
7 - params -> param-list | void
8.1 - param-list -> param param-list-tail
8.2 - param-list-tail -> , param param-list-tail | ε
9.1 - param -> type-specifier ID param-tail
9.2 - param-tail -> ε | [ ] 
10 - compound-stmt -> {statement-list}
12.1 - statement-list -> empty statement-list-tail 
12.2 - statement-list-tail -> statement statement-list-tail | ε
13 - statement -> assignment-stmt | compound-stmt | selection-stmt | iteration-stmt
15.1 - selection-stmt -> if ( expression ) statement selection-stmt-tail
15.2 - selection-stmt-tail -> else statement | ε
16 - iteration-stmt -> while ( expression ) statement
18 - assignment-stmt -> var = expression
19.1 - var -> ID var-tail
19.2 - var-tail -> [ expression ] | ε
20.1 - expression -> additive-expression expression-tail
20.2 - expression-tail -> relop additive-expression expression-tail | ε
21 - relop -> <= | < | > | >= | == | !=
22.1 - additive-expression -> term additive-expression-tail
22.2 - additive-expression-tail -> addop term additive-expression-tail | ε
23 - addop -> +|-
24.1 - term -> factor term-tail
24.2 - term-tail -> mulop factor term-tail | ε 
25. mulop -> * | /
26. factor -> ( expression ) | var | NUM

Run using:
flex scanner.l
gcc -c lex.yy.c -o lex.yy.o
g++ parser.cpp lex.yy.o -lfl -o parser
dos2unix test.txt
./parser < test.txt
