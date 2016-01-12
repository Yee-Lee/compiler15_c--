#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED

/******************************************************************************************************************************************
    All enumeration literals
       TokenType : Specify the type of the token scanner returns
	   DataType  : The data type of the declared variable
	   StmtType  : Indicate one statement in AcDc program is print or assignment statement.
	   ValueType : The node types of the expression tree that represents the expression on the right hand side of the assignment statement.
	               Identifier, IntConst, FloatConst must be the leaf nodes ex: a, b, c , 1.5 , 3.
				   PlusNode, MinusNode, MulNode, DivNode are the operations in AcDc. They must be the internal nodes.
                   Note that IntToFloatConvertNode to represent the type coercion may appear after finishing type checking. 			  
	   Operation : Specify all arithematic expression, including +, - , *, / and type coercion.
*******************************************************************************************************************************************/

typedef enum TokenType { FloatDeclaration, IntegerDeclaration, PrintOp, AssignmentOp, PlusOp, MinusOp,
             MulOp, DivOp, Alphabet, IntValue, FloatValue, EOFsymbol } TokenType;
typedef enum DataType { Int, Float, Notype }DataType;
typedef enum StmtType { Print, Assignment } StmtType;
typedef enum ValueType { Identifier, IntConst, FloatConst, PlusNode, MinusNode, MulNode, DivNode, IntToFloatConvertNode } ValueType;
typedef enum Operation { Plus, Minus, Mul, Div, Assign, IntToFloatConvert } Operation;


/**************************************************************************************** 
   All structures to facilitate the processes of 
   scanning, parsing, AST, type-checking, building the symbol table, and code generation.
*****************************************************************************************/


/* For scanner */
typedef struct Token{
    TokenType type;
    char tok[1025];
}Token;

/*** The following are nodes of the AST. ***/

/* For decl production or say one declaration statement */
typedef struct Declaration{
    DataType type;
    char name[256];
}Declaration;

/* 
    For decls production or say all declarations. (
	You can view it as the subtree for decls in AST,
	or just view it as the linked list that stores 
	all declarations. ) 
*/
typedef struct Declarations{
    Declaration first;
    struct Declarations *rest;
}Declarations;

/* For the nodes of the expression on the right hand side of one assignment statement */
typedef struct Value{
    ValueType type;
    union{
        char id[256];               /* if the node represent the access of the identifier */
        char regId;
        Operation op;              /* store +, -, *, /, =, type_convert */
        int ivalue;                /* for integer constant in the expression */
        float fvalue;              /* for float constant */
    }val;
}Value;


/* 
   The data structure of the expression tree.
   Recall how to deal with expression by tree 
   in data structure course.   
*/
typedef struct Expression{
    Value v;
    struct Expression *leftOperand;
    struct Expression *rightOperand;
    DataType type;
}Expression;


/* For one assignment statement */
typedef struct AssignmentStatement{
    char id[256];
    char regId;
    Expression *expr;
    DataType type;      /* For type checking to store the type of all expression on the right. */
}AssignmentStatement;


/* For stmt production or say one statement*/
typedef struct Statement{
    StmtType type;
    union{
        char regId;                      /* print statement(register id)*/
        char variable[256];              /* print statement */
        AssignmentStatement assign;
    }stmt;
}Statement;

/* For stmts production or say all productions */
typedef struct Statements{
    struct Statement first;
    struct Statements *rest;
}Statements;

/* For the root of the AST. */
typedef struct Program{
    Declarations *declarations;
    Statements *statements;
}Program;



/* For building the symbol table */
typedef struct TableRecord{
    DataType type;
    char id[256];
} Record;

typedef struct SymbolTable{
    Record record[23];
//    DataType table[26];
} SymbolTable;


Token getNumericToken( FILE *source, char c );
Token scanner( FILE *source );
Declaration makeDeclarationNode( Token declare_type, Token identifier );
Declarations *makeDeclarationTree( Declaration decl, Declarations *decls );
Declaration parseDeclaration( FILE *source, Token token );
Declarations *parseDeclarations( FILE *source );
Expression *parseValue( FILE *source );

//hw1 added-----------------------------------------
Expression *parseTerm(FILE *source, Expression *lvalue);
Expression *parseTermTail(FILE *source, Expression *lvalue);
Expression *constantFolding(Expression* expr);
Token getWordToken( FILE *source, char c);
Token getOperatorToken(FILE* source, char c);
Token checkKeyword( Token token);
char variableToRegister(SymbolTable *table, char *name);
//-----------------------------------------------------

Expression *parseExpressionTail( FILE *source, Expression *lvalue );
Expression *parseExpression( FILE *source, Expression *lvalue );

//modified**----------------------------
Statement makeAssignmentNode( char *name, Expression *term, Expression *expr_tail );
Statement makePrintNode( char *name );
//-----------------------------------------

Statements *makeStatementTree( Statement stmt, Statements *stmts );
Statement parseStatement( FILE *source, Token token );
Statements *parseStatements( FILE * source );
Program parser( FILE *source );
void InitializeTable( SymbolTable *table );

//modified------------------------------------------------------
void add_table( SymbolTable *table, char *name, DataType t );
DataType lookup_table( SymbolTable *table, char *name );
//---------------------------------------------------------------

SymbolTable build( Program program );
void convertType( Expression * old, DataType type );
DataType generalize( Expression *left, Expression *right );
void checkexpression( Expression * expr, SymbolTable * table );
void checkstmt( Statement *stmt, SymbolTable * table );
void check( Program *program, SymbolTable * table);
void fprint_op( FILE *target, ValueType op );
void fprint_expr( FILE *target, Expression *expr );
void gencode( Program prog, FILE * target );

void print_expr( Expression *expr );
void test_parser( FILE *source );

#endif // HEADER_H_INCLUDED
