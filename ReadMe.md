# A Recursive-Descent Parser
This is a basic recusrive-descent parser. 
It parses the language with the following rules:

1. program -> Program ID {declaration-list statement-list}.
2. declaration-list -> declaration declaration-list-tail
3. declaration-list-tail -> declaration declaration-list-tail | ε
4. declaration -> var-declaration
5. var-declaration -> type-specifier ID var-declaration-tail
6. var-declaration-tail -> ; | [ NUM ] ;
7. type-specifier -> int | float
8. params -> param-list | void
9. param-list -> param param-list-tail
10. param-list-tail -> , param param-list-tail | ε
11. param -> type-specifier ID param-tail
12. param-tail -> ε | [ ] 
13. compound-stmt -> {statement-list}
14. statement-list -> empty statement-list-tail 
15. statement-list-tail -> statement statement-list-tail | ε
16. statement -> assignment-stmt | compound-stmt | selection-stmt | iteration-stmt
17. selection-stmt -> if ( expression ) statement selection-stmt-tail
18. selection-stmt-tail -> else statement | ε
19. iteration-stmt -> while ( expression ) statement
20. assignment-stmt -> var = expression
21. var -> ID var-tail
22. var-tail -> [ expression ] | ε
23. expression -> additive-expression expression-tail
24. expression-tail -> relop additive-expression expression-tail | ε
25. relop -> <= | < | > | >= | == | !=
26. additive-expression -> term additive-expression-tail
27. additive-expression-tail -> addop term additive-expression-tail | ε
28. addop -> +|-
29. term -> factor term-tail
30. term-tail -> mulop factor term-tail | ε 
31. mulop -> * | /
32. factor -> ( expression ) | var | NUM

## Running the Program
You can run the program via WSL using the following commands: 

```bash
flex scanner.l
gcc -c lex.yy.c -o lex.yy.o
g++ parser.cpp lex.yy.o -lfl -o parser
dos2unix test1.txt
./parser < test1.txt
```

Replace `test1.txt` with the name of the file you want to run the program on (assuming it is in the same directory).