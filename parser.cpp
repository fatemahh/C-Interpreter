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

// -----------------------------     Store in Token Vector      -------------------------------

// vector<Token> tokens; // Vector to store tokens
// int tokenIndex = 0; // Index of the current token being processed

/*
extern "C" void addToken(int type, const char *value, int line) {
    // Assume tokens is your global C++ vector<Token> from your parser.
    tokens.push_back(Token((TokenType)type, std::string(value), line));
}
*/

// this returns the next token from the tokens vector.
Token getToken() {
    /*
    if (tokenIndex < tokens.size()) {
        // currentToken = tokens[tokenIndex];
        int currIdx = tokenIndex;
        tokenIndex++;
        return tokens[currIdx];
    } else {
        return Token(UNKNOWN, "", -1); // Return an empty token if out of bounds
    }
    */

    int tokenType = yylex(); // Get next token from lexer
    // if (tokenType == -1) {
    //     return Token(UNKNOWN, "", -1);
    // }
    // Assuming the scanner provides a way to get token value and line
    return Token((TokenType)tokenType, yytext, yylineno); // Set the value and line properly if needed
}

// ---------------------------------------------------------------------------------------------

void error(const char* message) {
    cerr << "Syntax error at line " << currentToken.line
        << ": found '" << currentToken.value
        << "'. " << message << std::endl;
    exit(EXIT_FAILURE);
}

void match(TokenType expected) {

    /* CHECK NEXT TOKEN IN TOKEN ARRAY
    if (currentToken.type == expected) {
        currentToken = getToken();
    } else {
        char msg[128];
        snprintf(msg, sizeof(msg), "Expected token type %s but found %s",
                 tokenTypeNames[expected], tokenTypeNames[currentToken.type]);
        error(msg);
    }
    */

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

void program(); 
void declaration_list();
void declaration_list_tail();
void declaration();
void var_declaration();
void var_declaration_tail();
void type_specifier();
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
void var();
void var_tail();
void expression();
void expression_tail();
void relop();
void additive_expression();
void additive_expression_tail();
void addop();
void term();
void term_tail();
void mulop();
void factor();


void program() {
    // if (currentToken.type == PROGRAM) {
    //     match(PROGRAM);
    // } else {
    //     error("Expected 'Program' keyword");
    // }
    match(PROGRAM);
    
    match(ID); 
    match(LBRACE);

    declaration_list();
    statement_list();

    match(RBRACE);
    match(DOT);
}

void declaration_list() {
    declaration();
    declaration_list_tail();
}

void declaration_list_tail() {
    if (currentToken.type == INT || currentToken.type == FLOAT) {
        declaration();
        declaration_list_tail();
    }
}

void declaration() {
    var_declaration();
}

void var_declaration() {
    type_specifier();
    match(ID);
    var_declaration_tail();
}

void var_declaration_tail() {
    if (currentToken.type == SEMICOLON) {
        match(SEMICOLON);
    } else if (currentToken.type == LBRACKET) {
        match(LBRACKET);
        match(NUM);
        match(RBRACKET);
        match(SEMICOLON);
    } else {
        error("Expected ';' or '[' after variable declaration");
    }
}

void type_specifier() {
    if (currentToken.type == INT) {
        match(INT);
    } else if (currentToken.type == FLOAT) {
        match(FLOAT);
    } else {
        error("Expected 'int' or 'float' keyword");
    }
}

void params() {
    if (currentToken.type == VOID) {
        match(VOID);
    } else {
        param_list();
    }
}

void param_list() {
    param();
    param_list_tail();
}

void param_list_tail() {
    if (currentToken.type == COMMA) {
        match(COMMA);
        param();
        param_list_tail();
    }
}

void param() {
    type_specifier();
    match(ID);
    param_tail();
}

void param_tail() {
    if (currentToken.type == LBRACKET) {
        match(LBRACKET);
        match(RBRACKET);
    }
}

void compound_stmt() {
    match(LBRACE);
    statement_list();
    match(RBRACE);
}

// statement-list -> empty statement-list-tail 
// statement-list-tail -> statement statement-list-tail | ε
void statement_list() {
    // while (currentToken.type == ID || currentToken.type == LBRACE ||
    //     currentToken.type == IF || currentToken.type == WHILE) {
    //     statement();
    // }
    if (currentToken.type == ID || currentToken.type == LBRACE ||
        currentToken.type == IF || currentToken.type == WHILE) {
        statement_list_tail();
    }
}

void statement_list_tail() {
    if (currentToken.type == ID || currentToken.type == LBRACE ||
        currentToken.type == IF || currentToken.type == WHILE) {
        statement();
        statement_list_tail();
    }
}

void statement() {
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

void assignment_stmt() {
    var();
    match(ASSIGN);
    expression();
}

void selection_stmt() {
    match(IF);
    match(LPAREN);
    expression();
    match(RPAREN);
    statement();
    selection_stmt_tail();
}

void selection_stmt_tail() {
    if (currentToken.type == ELSE) {
        match(ELSE);
        statement();
    }
}

void iteration_stmt() {
    match(WHILE);
    match(LPAREN);
    expression();
    match(RPAREN);
    statement();
}

// wasalt here

void var() {
    match(ID);
    var_tail();
}

void var_tail() {
    if (currentToken.type == LBRACKET) {
        match(LBRACKET);
        expression();
        match(RBRACKET);
    }
}

void expression() {
    additive_expression();
    expression_tail();
}

void expression_tail() {
    if (currentToken.type == LT || currentToken.type == LTE || currentToken.type == GT || 
        currentToken.type == GTE || currentToken.type == EQ || currentToken.type == NEQ) {
        relop();
        additive_expression();
        expression_tail();
    }
}

void relop() {
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

void additive_expression() {
    term();
    additive_expression_tail();
}

void additive_expression_tail() {
    if (currentToken.type == PLUS || currentToken.type == MINUS) {
        addop();
        term();
        additive_expression_tail();
    }
}

void addop() {
    if (currentToken.type == PLUS) {
        match(PLUS);
    } else if (currentToken.type == MINUS) {
        match(MINUS);
    } else {
        error("Expected '+' or '-' operator");
    }
}

void term() {
    factor();
    term_tail();
}


void term_tail() {
    if (currentToken.type == MUL || currentToken.type == DIV) {
        mulop();
        factor();
        term_tail();
    }
}

void mulop() {
    if (currentToken.type == MUL) {
        match(MUL);
    } else if (currentToken.type == DIV) {
        match(DIV);
    } else {
        error("Expected '*' or '/' operator");
    }
}

void factor() {
    if (currentToken.type == LPAREN) {
        match(LPAREN);
        expression();
        match(RPAREN);
    } else if (currentToken.type == ID) {
        var();
    } else if (currentToken.type == NUM) {
        match(NUM);
    } else {
        error("Expected '(', ID, or NUM");
    }
}

int main() {
    
    // yylex();

    // debugging
    // for (size_t i = 0; i < tokens.size(); i++) {
    //     cout << "Token: " << tokenTypeNames[tokens[i].type]
    //          << ", Value: '" << tokens[i].value
    //          << "', Line: " << tokens[i].line << endl;
    // }

    
    currentToken = getToken(); // Initialize the first token
    // printf("current token: %s\n", tokenTypeNames[currentToken.type]);

    if (currentToken.type == UNKNOWN) {
        cerr << "Error: lexer couldn't initialize properly!" << endl;
        return 1;
    }

    program(); // Start parsing

    cout << "Parsing completed successfully!" << endl;

    return 0;
}