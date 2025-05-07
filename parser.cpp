#include <iostream>
#include <bits/stdc++.h>
#include "scanner.h"

using namespace std; 

extern "C" int yylex();
extern char* yytext;      // yytext contains the current token's string value
extern int yylineno;      // yylineno contains the line number of the current token


const char* tokenTypeNames[] = {
    "PROGRAM", "INT", "FLOAT", "IF", "ELSE", "WHILE", "VOID", 
    "ID", "NUM", 
    "LBRACE", "RBRACE", "LBRACKET", "RBRACKET", "LPAREN", "RPAREN",
    "SEMICOLON", "COMMA", "DOT", "ASSIGN", 
    "PLUS", "MINUS", "MUL", "DIV", 
    "LT", "LTE", "GT", "GTE", "EQ", "NEQ",
    "UNKNOWN"
};

struct Token {
    TokenType type;
    string value;
    int line;

    Token(TokenType t, string v, int l) : type(t), value(v), line(l) {}
    Token() {}
};

Token currentToken; // Current token being processed

// -------------------------------------- SYMBOL TABLE -----------------------------------
enum enumType { 
    typeInt, typeFloat
};

struct Symbol {
    string name;
    enumType type;
    string value;
    int lineDeclared;
};

unordered_map<string, Symbol> symbolTable;

void printSymbolTable() {
    cout << "\nSymbol Table:\n";
    for (const auto& [name, sym] : symbolTable) {
        cout << "Name: " << name << ", Type: " << sym.type << ", Value: " << sym.value << ", Declared at line: " << sym.lineDeclared << endl;
    }
}

void declareVariable(const string& name, enumType type, const string& value, int line) {
    if (symbolTable.count(name)) {
        cerr << "Semantic Error: Variable '" << name << "' already declared (line " << line << ")." << endl;
        exit(1);
    }
    Symbol sym = { name, type, value, line };
    symbolTable[name] = sym;
}

Symbol& getVariable(const string& name, int line) {
    if (!symbolTable.count(name)) {
        cerr << "Semantic Error: Variable '" << name << "' used before declaration (line " << line << ")." << endl;
        exit(1);
    }
    return symbolTable[name];
}
// -------------------------------------- ^^^ SYMBOL TABLE ^^^ -----------------------------------


// this returns the next token from the tokens vector.
Token getToken() {
    int tokenType = yylex(); // Get next token from lexer
    return Token((TokenType)tokenType, yytext, yylineno); // Set the value and line properly if needed
}

void error(const char* message) {
    cerr << "Syntax error at line " << currentToken.line
        << ": found '" << currentToken.value
        << "'. " << message << std::endl;
    exit(EXIT_FAILURE);
}

void match(TokenType expected) {
    if (currentToken.type == expected) {
        currentToken = getToken();
    } 
    else {
        char msg[128];
        snprintf(msg, sizeof(msg), "Expected token type %s but found %s",
                 tokenTypeNames[expected], tokenTypeNames[currentToken.type]);
        error(msg);
    }
}

// ------------------------------- RULES ----------------------------------------------

void program(); 
void declaration_list();
void declaration_list_tail();
void declaration();
void var_declaration();
void var_declaration_tail();
enumType type_specifier();
void params();
void param_list();
void param_list_tail();
void param();
void param_tail();
void compound_stmt();
void statement_list();
void statement_list_tail();
void statement();
void assignment_stmt();
void selection_stmt();
void selection_stmt_tail();
void iteration_stmt();
Symbol var();
void var_tail();
Symbol expression();
Symbol expression_tail(Symbol term1);
void relop();
Symbol additive_expression();
Symbol additive_expression_tail(Symbol term1);
void addop();
Symbol term();
Symbol term_tail(Symbol term);
void mulop();
Symbol factor();


void program() // 1 - program -> Program ID {declaration-list statement-list}.
{

    match(PROGRAM);
    
    match(ID); 
    match(LBRACE);

    declaration_list();
    statement_list();

    match(RBRACE);
    match(DOT);
}

void declaration_list() // 2.1 - declaration-list -> declaration declaration-list-tail
{
    declaration();
    declaration_list_tail();
}

void declaration_list_tail() // 2.2 - declaration-list-tail -> declaration declaration-list-tail | ε
{
    if (currentToken.type == INT || currentToken.type == FLOAT) {
        declaration();
        declaration_list_tail();
    }
}

void declaration() // 3 - declaration -> var-declaration
{
    var_declaration();
}

void var_declaration() // 4.1 - var-declaration -> type-specifier ID var-declaration-tail
{
    // get type of variable
    enumType varType;
    varType = type_specifier();

    string varName = currentToken.value;
    declareVariable(varName, varType, "", currentToken.line);
    match(ID);

    var_declaration_tail();
}

void var_declaration_tail() // 4.2 - var-declaration-tail -> ; | [ NUM ] ;
{
    if (currentToken.type == SEMICOLON) {
        match(SEMICOLON);
    } else if (currentToken.type == LBRACKET) { // THIS IS IF ITS AN ARRAY
        match(LBRACKET);
        match(NUM);
        match(RBRACKET);
        match(SEMICOLON);
    } else {
        error("Expected ';' or '[' after variable declaration");
    }
}

enumType type_specifier() // 5 - type-specifier -> int | float
{
    if (currentToken.type == INT) {
        match(INT);
        return typeInt;
    } else if (currentToken.type == FLOAT) {
        match(FLOAT);
        return typeFloat;
    } else {
        error("Expected 'int' or 'float' keyword");
        exit(0);
    }
}

void params() // 7 - params -> param-list | void
{
    if (currentToken.type == VOID) {
        match(VOID);
    } else {
        param_list();
    }
}

void param_list() // 8.1 - param-list -> param param-list-tail
{
    param();
    param_list_tail();
}

void param_list_tail() // 8.2 - param-list-tail -> , param param-list-tail | ε
{
    if (currentToken.type == COMMA) {
        match(COMMA);
        param();
        param_list_tail();
    }
}

void param() // 9.1 - param -> type-specifier ID param-tail
{
    // get type of variable
    enumType variabletype;
    variabletype = type_specifier();

    string variablename  = currentToken.value;
    declareVariable(variablename, variabletype, "", currentToken.line);
    match(ID);
    
    param_tail();
}

void param_tail() // 9.2 - param-tail -> ε | [ ] 
{
    if (currentToken.type == LBRACKET) { // THIS IS IF ITS AN ARRAY
        match(LBRACKET);
        match(RBRACKET);
    }
}

void compound_stmt() // 10 - compound-stmt -> {statement-list}
{
    match(LBRACE);
    statement_list();
    match(RBRACE);
}

void statement_list() // 12.1 - statement-list -> empty statement-list-tail 
{
    if (currentToken.type == ID || currentToken.type == LBRACE ||
        currentToken.type == IF || currentToken.type == WHILE) {
        statement_list_tail();
    }
}

void statement_list_tail() // 12.2 - statement-list-tail -> statement statement-list-tail | ε
{
    if (currentToken.type == ID || currentToken.type == LBRACE ||
        currentToken.type == IF || currentToken.type == WHILE) {
        statement();
        statement_list_tail();
    }
}

void statement() // 13 - statement -> assignment-stmt | compound-stmt | selection-stmt | iteration-stmt
{
    if (currentToken.type == ID) {
        assignment_stmt();
    } else if (currentToken.type == LBRACE) {
        compound_stmt();
    } else if (currentToken.type == IF) {
        selection_stmt();
    } else if (currentToken.type == WHILE) {
        iteration_stmt();
    } else {
        error("Expected statement (ID, '{', 'if', or 'while')");
    }
}

void assignment_stmt() // 18 - assignment-stmt -> var = expression
{    
    Symbol varSymbol;
    varSymbol = var();

    match(ASSIGN);

    varSymbol = expression();

    symbolTable[varSymbol.name].value = varSymbol.value;
}

void selection_stmt() // 15.1 - selection-stmt -> if ( expression ) statement selection-stmt-tail
{
    match(IF);
    match(LPAREN);
    expression();
    match(RPAREN);
    statement();
    selection_stmt_tail();
}

void selection_stmt_tail() // 15.2 - selection-stmt-tail -> else statement | ε
{
    if (currentToken.type == ELSE) {
        match(ELSE);
        statement();
    }
}

void iteration_stmt() // 16 - iteration-stmt -> while ( expression ) statement
{
    match(WHILE);
    match(LPAREN);
    expression();
    match(RPAREN);
    statement();
}

Symbol var() // 19.1 - var -> ID var-tail
{
    string varName = currentToken.value;
    Symbol varSymbol;
    varSymbol = getVariable(varName, currentToken.line);
    match(ID);
    var_tail();
    return varSymbol;

}

void var_tail() // 19.2 - var-tail -> [ expression ] | ε
{
    if (currentToken.type == LBRACKET) {
        match(LBRACKET);
        expression();
        match(RBRACKET);
    }
}

Symbol expression() // 20.1 - expression -> additive-expression expression-tail
{
    /*
    //Declare local variable that will be used as parameters ("type" synthesized attributes) of the term function
    string left_type, right_type;

    //Declare local variable that will be used as parameters ("val" synthesized attributes) of the term function
    int left_val, right_val;
    
    string temp; // variable to keep the name of the temporary variable

    // The BNF of expr is converted to EBNF to avoid the left recursion
    term(left_type, left_val); //term() will return type and value of term

    // This loop is the check the types of the operands and evaluate the expression
    while (token == PLUS) {
        match(PLUS);
        term(right_type, right_val);
        
        // Check left and right operands types
        if (right_type != left_type)
            semantic_err("Operand are not the same type");
        
        // Compute the left and right operands and put the results in the variable left_val used to accumulate the results
        left_val = left_val + right_val;
    };

    exp_typ = left_type;
    val = left_val;
    */
    
    Symbol term1;
    Symbol result;

    term1 = additive_expression();
    result = expression_tail(term1);
    return result;
}

Symbol expression_tail(Symbol term1) // 20.2 - expression-tail -> relop additive-expression expression-tail | ε
{
    Symbol term2;
    Symbol result;
    if (currentToken.type == LT) {
        relop();
        term2 = additive_expression(); 
        if (stoi(term1.value) < stoi(term2.value)) {
            result.value = "1";
        }
        else {
            result.value = "0";
        }    
        return expression_tail(result);
    }
    else if (currentToken.type == LTE) {
        relop();
        term2 = additive_expression();
        if (stoi(term1.value) <= stoi(term2.value)) {
            result.value = "1";
        }
        else {
            result.value = "0";
        }     
        return expression_tail(result);    
    }
    else if (currentToken.type == GT) {
        relop();
        term2 = additive_expression();
        if (stoi(term1.value) > stoi(term2.value)) {
            result.value = "1";
        }
        else {
            result.value = "0";
        }    
        return expression_tail(result);
    }
    else if (currentToken.type == GTE) {
        relop();
        term2 = additive_expression();
        if (stoi(term1.value) >= stoi(term2.value)) {
            result.value = "1";
        }
        else {
            result.value = "0";
        }     
        return expression_tail(result);
    }
    else if (currentToken.type == EQ) {
        relop();
        term2 = additive_expression();
        if (stoi(term1.value) == stoi(term2.value)) {
            result.value = "1";
        }
        else {
            result.value = "0";
        }   
        return expression_tail(result);
    }
    else if (currentToken.type == NEQ) {
        relop();
        term2 = additive_expression();
        if (stoi(term1.value) != stoi(term2.value)) {
            result.value = "1";
        }
        else {
            result.value = "0";
        }    
        return expression_tail(result);
    }
    return result;
}

void relop() // 21 - relop -> <= | < | > | >= | == | !=
{
    if (currentToken.type == LT) {
        match(LT);
    }
    else if (currentToken.type == LTE) {
        match(LTE);
    } else if (currentToken.type == GT) {
        match(GT);
    } else if (currentToken.type == GTE) {
        match(GTE);
    } else if (currentToken.type == EQ) {
        match(EQ);
    } else if (currentToken.type == NEQ) {
        match(NEQ);
    } else {
        error("Expected relational operator (<=, <, >=, >, ==, !=)");
    }
}

Symbol additive_expression() // 22.1 - additive-expression -> term additive-expression-tail
{
    Symbol term1;
    Symbol result;
    term1 = term();
    result = additive_expression_tail(term1);

    return result;
}

Symbol additive_expression_tail(Symbol term1) // 22.2 - additive-expression-tail -> addop term additive-expression-tail | ε
{
    Symbol term2;
    Symbol result;

    if (currentToken.type == PLUS) {
        addop();
        term2 = term();
        if ((term1.type == typeInt) && (term2.type == typeInt))
            result.value = to_string(stoi(term1.value) + stoi(term2.value)); 
        else
            result.value = to_string(stof(term1.value) + stof(term2.value)); 
        
        result = additive_expression_tail(result);
        return result;
    }
    else if (currentToken.type == MINUS) {
        addop();
        term2 = term();
        if ((term1.type == typeInt) && (term2.type == typeInt))
            result.value = to_string(stoi(term1.value) - stoi(term2.value)); 
        else
            result.value = to_string(stof(term1.value) - stof(term2.value)); 
            
        result = additive_expression_tail(result);
        return result;
    }

    result.value = "0";
    return result;
}

void addop() // 23 - addop -> +|-
{
    if (currentToken.type == PLUS) {
        match(PLUS);
    } else if (currentToken.type == MINUS) {
        match(MINUS);
    } else {
        error("Expected '+' or '-' operator");
    }
}

Symbol term() // 24.1 - term -> factor term-tail
{
    Symbol factor1;
    Symbol result;

    factor1 = factor();
    result = term_tail(factor1);

    return result;
}

Symbol term_tail(Symbol term) // 24.2 - term-tail -> mulop factor term-tail | ε 
{
    Symbol result;
    if (currentToken.type == MUL || currentToken.type == DIV) {
        if (currentToken.type == MUL) {
            mulop();
            Symbol value = factor();
            if ((term.type == typeInt) && (value.type == typeInt))
                result.value = to_string(stoi(term.value) * stoi(value.value)); 
            else
                result.value = to_string(stof(term.value) * stof(value.value)); 
        } else if (currentToken.type == DIV) {
            mulop();
            Symbol value = factor();
            if ((term.type == typeInt) && (value.type == typeInt))
                result.value = to_string(stoi(term.value) / stoi(value.value)); 
            else
                result.value = to_string(stof(term.value) / stof(value.value));
        } else {
            error("Expected '*' or '/' operator");
            exit(0);
        }

        result = term_tail(result);
    }
    return result;
}

void mulop() // 25. mulop -> * | /
{
    if (currentToken.type == MUL) {
        match(MUL);
    } else if (currentToken.type == DIV) {
        match(DIV);
    } else {
        error("Expected '*' or '/' operator");
    }
}

Symbol factor() // 26. factor -> ( expression ) | var | NUM
{
    Symbol result;
    if (currentToken.type == LPAREN) {
        match(LPAREN);
        result = expression();
        match(RPAREN);
    } else if (currentToken.type == ID) {
        result = var();
    } else if (currentToken.type == NUM) {
        result.name = "";
        result.value = currentToken.value;
        if (currentToken.type == ID)
            result.type = typeInt;
        else
            result.type = typeFloat;
        match(NUM);
    } else {
        error("Expected '(', ID, or NUM");
    }
    
    return result;
}

// ------------------------------- ^^^ RULES ^^^ ----------------------------------------


int main() {
    
    currentToken = getToken(); // Initialize the first token

    if (currentToken.type == UNKNOWN) {
        cerr << "Error: lexer couldn't initialize properly!" << endl;
        return 1;
    }

    cout << "=== Running Parser + Interpreter ===\n";
    program(); // Start parsing
    cout << "Parsing completed successfully!" << endl;

    cout << "=== Final Symbol Table ===\n";
    for (const auto& entry : symbolTable) {
        cout << entry.first << " = " << entry.second.value << " (type: " << entry.second.type << ")\n";
    }

    return 0;
}