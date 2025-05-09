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
    bool isArray = false;
    int arraySize = 0;
    vector<string> values;
    int lineDeclared;
    int lastIndex = -1; // this will hold the index for when var_tail sees `[expr]`
};

struct LValue {
  Symbol* sym;    // pointer into symbolTable
  int     index;  // –1 if scalar, or the array index   
};

unordered_map<string, Symbol> symbolTable;

void printSymbolTable() {
    cout << "\nSymbol Table:\n";
    for (const auto& [name, sym] : symbolTable) {
        cout << "Name: " << name << ", Type: " << sym.type << ", Value: " << sym.value << ", Declared at line: " << sym.lineDeclared << endl;
    }
}

void semantic_error(int line, const string &msg) {
    cerr << "Semantic error at line " << line << ": " << msg << "\n";
    exit(1);
}

void declareVariable(const string& name, enumType type, const string& initVal, int line, bool isArr = false, int arrSize = 0) {
    if (symbolTable.count(name)) {
        // cerr << "Semantic Error: Variable '" << name << "' already declared (line " << line << ")." << endl;
        semantic_error(line, "variable '" + name + "' already declared");
    }
    Symbol sym;
    sym.name = name;
    sym.type = type;
    sym.value = initVal; 
    sym.isArray = isArr;
    sym.arraySize = arrSize;
    sym.values = vector<string>(arrSize, initVal);
    sym.lineDeclared = line;
    symbolTable[name] = sym;
}

Symbol& getVariable(const string& name, int line) {
    if (!symbolTable.count(name)) {
        semantic_error(line, "variable '" + name + "' used before declaration");
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
void var_declaration_tail(const string &varName);
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
void var_tail(Symbol& sym);
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
    enumType varType = type_specifier();
    string varName = currentToken.value;
    match(ID);
    
    declareVariable(varName, varType, "", currentToken.line);

    var_declaration_tail(varName);
}

void var_declaration_tail(const string &varName) // 4.2 - var-declaration-tail -> ; | [ NUM ] ;
{
    if (currentToken.type == SEMICOLON) {
        match(SEMICOLON);
    } else if (currentToken.type == LBRACKET) { // THIS IS IF ITS AN ARRAY
        match(LBRACKET);
        int size = stoi(currentToken.value);
        match(NUM);
        match(RBRACKET);
        match(SEMICOLON);

        Symbol &sym = symbolTable[varName];
        sym.isArray = true;
        sym.arraySize = size;
        sym.values = vector<string>(size, "0");
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

    string variablename = currentToken.value;
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

// CHANGED
void assignment_stmt() // 18 - assignment-stmt -> var = expression
{    
    Symbol lhs = var();
    int opLine = currentToken.line;
    match(ASSIGN);
    Symbol rhs = expression();

    if (lhs.type != rhs.type) {
        semantic_error(opLine,
            "cannot assign " +
            string(rhs.type==typeInt?"int":"float") +
            " to " +
            string(lhs.type==typeInt?"int":"float") +
            " variable '" + lhs.name + "'");
    }

    auto &entry = symbolTable[lhs.name];
    if (lhs.lastIndex < 0) {
        entry.value = rhs.value;
    } else {
        entry.values[lhs.lastIndex] = rhs.value;
    }
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
    Symbol varSymbol = getVariable(varName, currentToken.line);
    match(ID);

    var_tail(varSymbol);
    return varSymbol;

}

void var_tail(Symbol& sym) // 19.2 - var-tail -> [ expression ] | ε
{
    if (currentToken.type == LBRACKET) {
        match(LBRACKET);
        Symbol idxSym = expression(); 
        int idx = stoi(idxSym.value);
        match(RBRACKET);

        if (!sym.isArray) {
            // error("Semantic error: " + sym.name + " is not an array");
            semantic_error(sym.lineDeclared, "variable '" + sym.name + "' is not an array");
        }
        if (idx < 0 || idx >= sym.arraySize) {
            semantic_error(currentToken.line, "array index out of bounds for '" + sym.name + "'");
        }

        sym.lastIndex = idx;
        sym.value = sym.values[idx];
    }
}

Symbol expression() // 20.1 - expression -> additive-expression expression-tail
{
    Symbol term1;
    Symbol result;

    term1 = additive_expression();
    result = expression_tail(term1);
    return result;
}

Symbol expression_tail(Symbol term1) { // 20.2 - expression-tail -> relop additive-expression expression-tail | ε
    if ( currentToken.type == LT
        || currentToken.type == LTE
        || currentToken.type == GT
        || currentToken.type == GTE
        || currentToken.type == EQ
        || currentToken.type == NEQ ) 
      {
        TokenType op = currentToken.type;
        int opLine = currentToken.line;
        match(op);
        Symbol term2 = additive_expression();

        bool cond;
        // compare as ints (we only support integer relational results here)
        if (term1.type != term2.type)
            semantic_error(opLine, "mixed types in relational operator");

        int lhs = stoi(term1.value), rhs = stoi(term2.value);

        switch (op) {
            case LT:  cond = lhs <  rhs; break;
            case LTE: cond = lhs <= rhs; break;
            case GT:  cond = lhs >  rhs; break;
            case GTE: cond = lhs >= rhs; break;
            case EQ:  cond = lhs == rhs; break;
            case NEQ: cond = lhs != rhs; break;
            default:  cond = false; break;
        }
        
        Symbol result;
        result.value = cond ? "1" : "0";
        result.type  = typeInt;
        result.name  = "";
        return expression_tail(result);
    }
    // ε-case: no comparison, just propagate the original term
    return term1;
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

Symbol additive_expression_tail(Symbol term1) {
    Symbol term2, result;

    if (currentToken.type == PLUS) {
        int opLine = currentToken.line;
        addop();                // consume '+'
        term2 = term();         // parse RHS

        if (term1.type == typeInt && term2.type == typeInt) {
            result.value = to_string(stoi(term1.value) + stoi(term2.value));
            result.type  = typeInt;
        }
        else if (term1.type == typeFloat && term2.type == typeFloat) {
            result.value = to_string(stof(term1.value) + stof(term2.value));
            result.type  = typeFloat;
        }
        else {
            semantic_error(opLine,
                "mixed types in addition (" +
                string(term1.type==typeInt?"int":"float") +
                " + " +
                string(term2.type==typeInt?"int":"float") +
                ")");
        }

        result.name = "";
        return additive_expression_tail(result);
    }
    else if (currentToken.type == MINUS) {
        int opLine = currentToken.line;
        addop();                // consume '-'
        term2 = term();         // parse RHS

        if (term1.type == typeInt && term2.type == typeInt) {
            result.value = to_string(stoi(term1.value) - stoi(term2.value));
            result.type  = typeInt;
        }
        else if (term1.type == typeFloat && term2.type == typeFloat) {
            result.value = to_string(stof(term1.value) - stof(term2.value));
            result.type  = typeFloat;
        }
        else {
            semantic_error(opLine,
                "mixed types in subtraction (" +
                string(term1.type==typeInt?"int":"float") +
                " - " +
                string(term2.type==typeInt?"int":"float") +
                ")");
        }

        result.name = "";
        return additive_expression_tail(result);
    }

    // ε-case: no more '+' or '-'
    return term1;
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

Symbol term_tail(Symbol term) { // 24.2 - term-tail -> mulop factor term-tail | ε 
    if (currentToken.type == MUL || currentToken.type == DIV) {
        TokenType op = currentToken.type;
        int opLine = currentToken.line;
        // match(op);
        mulop(); // consume '*' or '/'
        Symbol rhs = factor();

        Symbol result;
        // compute result.value & result.type
        if (op == MUL) {
            if (term.type == typeInt && rhs.type == typeInt) {
                result.value = to_string(stoi(term.value) * stoi(rhs.value));
                result.type  = typeInt;
            } else if (term.type == typeFloat && rhs.type == typeFloat) {
                result.value = to_string(stof(term.value) * stof(rhs.value));
                result.type  = typeFloat;
            }
            else {
                semantic_error(opLine,
                    "mixed types in multiplication (" +
                    string(term.type==typeInt?"int":"float") +
                    " * " +
                    string(rhs.type==typeInt?"int":"float") +
                    ")");
            }
        } else { // DIV
            if (term.type == typeInt && rhs.type == typeInt) {
                if (stoi(rhs.value) == 0) {
                    semantic_error(opLine, "division by zero");
                }
                result.value = to_string(stoi(term.value) / stoi(rhs.value));
                result.type  = typeInt;
            } else if (term.type == typeFloat && rhs.type == typeFloat) {
                if (stof(rhs.value) == 0) {
                    semantic_error(opLine, "division by zero");
                }
                result.value = to_string(stof(term.value) / stof(rhs.value));
                result.type  = typeFloat;
            }
            else {
                semantic_error(opLine,
                    "mixed types in division (" +
                    string(term.type==typeInt?"int":"float") +
                    " / " +
                    string(rhs.type==typeInt?"int":"float") +
                    ")");
            }
        }
        result.name = "";  
        return term_tail(result);
    }
    // ε-case: just propagate the original term
    return term;
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
        // if (currentToken.type == ID)
        //     result.type = typeInt;
        // else
        //     result.type = typeFloat;
        if (currentToken.value.find('.') != string::npos)
            result.type = typeFloat;
        else
            result.type = typeInt;
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
    for (const auto& [name, sym] : symbolTable) {
        if (!sym.isArray) {
            cout << name
                << " = " << sym.value
                << "  (type: " 
                << (sym.type==typeInt ? "int" : "float")
                << ")\n";
        }
        else {
            cout << name
                << "[" << sym.arraySize << "] = { ";
            for (int i = 0; i < sym.arraySize; ++i) {
            cout << sym.values[i]
                << (i+1<sym.arraySize ? ", " : " ");
            }
            cout << "}  (type: " 
                << (sym.type==typeInt ? "int" : "float")
                << ")\n";
        }
    }

    return 0;
}