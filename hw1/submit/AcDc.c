#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "header.h"


int main( int argc, char *argv[] )
{
    FILE *source, *target;
    Program program;
    SymbolTable symtab;

    if( argc == 3){
        source = fopen(argv[1], "r");
        
//        test_parser(source);
//        printf("\n");
//        return 0;

        target = fopen(argv[2], "w");
        if( !source ){
            printf("can't open the source file\n");
            exit(2);
        }
        else if( !target ){
            printf("can't open the target file\n");
            exit(2);
        }
        else{
            program = parser(source);
            fclose(source);
            symtab = build(program);
            check(&program, &symtab);
            gencode(program, target);
        }
    }
    else{
        printf("Usage: %s source_file target_file\n", argv[0]);
    }


    return 0;
}


/********************************************* 
  Scanning 
 *********************************************/
Token tokenBuffer;
int tokenBufferEmpty = 1;

void ungetToken(Token token) {
    tokenBuffer = token;
    tokenBufferEmpty = 0;
}

Token getNumericToken( FILE *source, char c )
{
    Token token;
    int i = 0;

    while( isdigit(c) ) {
        token.tok[i++] = c;
        c = fgetc(source);
    }

    if( c != '.' ){
        ungetc(c, source);
        token.tok[i] = '\0';
        token.type = IntValue;
        return token;
    }

    token.tok[i++] = '.';

    c = fgetc(source);
    if( !isdigit(c) ){
        ungetc(c, source);
        printf("Expect a digit : %c\n", c);
        exit(1);
    }

    while( isdigit(c) ){
        token.tok[i++] = c;
        c = fgetc(source);
    }

    ungetc(c, source);
    token.tok[i] = '\0';
    token.type = FloatValue;
    return token;
}

Token getWordToken( FILE *source, char c)
{
    Token token;
    int i = 0;

    while( isalpha(c) ) {
        token.tok[i++] = c;
        c = fgetc(source);
    }

    ungetc(c, source);
    token.tok[i] = '\0';
    token.type = Alphabet;

    return token;
}

Token getOperatorToken(FILE* source, char c) {
    Token token;
    token.tok[0] = c;
    token.tok[1] = '\0';
    switch(c){
        case '=':
            token.type = AssignmentOp;
            return token;
        case '+':
            token.type = PlusOp;
            return token;
        case '-':
            token.type = MinusOp;
            return token;
        case '*':
            token.type = MulOp;
            return token;
        case '/':
            token.type = DivOp;
            return token;
        case EOF:
            token.type = EOFsymbol;
            token.tok[0] = '\0';
            return token;
        default:
            printf("Invalid character : %c\n", c);
            exit(1);
    }
}

Token checkKeyword( Token token )
{
    if(strcmp(token.tok, "i") == 0)
        token.type = IntegerDeclaration;
    else if(strcmp(token.tok, "f") == 0)
        token.type = FloatDeclaration;
    else if(strcmp(token.tok, "p") == 0)
        token.type = PrintOp;
    return token;
}

Token scanner( FILE *source )
{
    char c;
    Token token;

    if(!tokenBufferEmpty) {
        tokenBufferEmpty = 1;
        return tokenBuffer;
    }

    while( !feof(source) ){
        c = fgetc(source);
        while( isspace(c) )
            c = fgetc(source);

        if( isdigit(c) )
            return getNumericToken(source, c);
        else if( isalpha(c) ){
            token = getWordToken(source, c);
            return checkKeyword(token);
        }
        else
            return getOperatorToken(source, c);
    }
    token.tok[0] = '\0';
    token.type = EOFsymbol;
    return token;
}


/********************************************************
  Parsing
 *********************************************************/
Declaration parseDeclaration( FILE *source, Token token )
{
    Token token2;
    switch(token.type){
        case FloatDeclaration:
        case IntegerDeclaration:
            token2 = scanner(source);
            if (strcmp(token2.tok, "f") == 0 ||
                    strcmp(token2.tok, "i") == 0 ||
                    strcmp(token2.tok, "p") == 0) {
                printf("Syntax Error: %s cannot be used as id\n", token2.tok);
                exit(1);
            }
            return makeDeclarationNode( token, token2 );
        default:
            printf("Syntax Error: Expect Declaration %s\n", token.tok);
            exit(1);
    }
}

Declarations *parseDeclarations( FILE *source )
{
    Token token = scanner(source);
    Declaration decl;
    Declarations *decls;
    switch(token.type){
        case FloatDeclaration:
        case IntegerDeclaration:
            decl = parseDeclaration(source, token);
            decls = parseDeclarations(source);
            return makeDeclarationTree( decl, decls );
        case PrintOp:
        case Alphabet:
            ungetToken(token);
            return NULL;
        case EOFsymbol:
            return NULL;
        default:
            printf("Syntax Error: Expect declarations %s\n", token.tok);
            exit(1);
    }
}

Expression *parseValue( FILE *source )
{
    Token token = scanner(source);
    Expression *value = (Expression *)malloc( sizeof(Expression) );
    value->leftOperand = value->rightOperand = NULL;

    switch(token.type){
        case Alphabet:
            (value->v).type = Identifier;
            strcpy((value->v).val.id, token.tok);
            break;
        case IntValue:
            (value->v).type = IntConst;
            (value->v).val.ivalue = atoi(token.tok);
            break;
        case FloatValue:
            (value->v).type = FloatConst;
            (value->v).val.fvalue = atof(token.tok);
            break;
        default:
            printf("Syntax Error: (parseValue)Expect Identifier or a Number %s\n", token.tok);
            exit(1);
    }

    return value;
}
Expression *parseTermTail(FILE *source, Expression *lvalue)
{
    Token token = scanner(source);
    Expression *expr;

    switch(token.type){
        case MulOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = MulNode;
            (expr->v).val.op = Mul;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseValue(source);
            return parseTermTail(source, expr);
        case DivOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = DivNode;
            (expr->v).val.op = Div;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseValue(source);
            return parseTermTail(source, expr);
        case Alphabet:
        case PrintOp:
        case PlusOp:
        case MinusOp:
            ungetToken(token);
            return lvalue;
        case EOFsymbol:
            return lvalue;
        default:
            printf("Syntax Error:(parseTermTail) Expect a numeric value or an identifier %s\n", token.tok);
            exit(1);
    }

}
Expression *parsePreTerm(FILE *source)
{
    Expression *v = parseValue(source);
    Expression *term = parseTerm(source, v);
    return term;

}

Expression *parseTerm(FILE *source, Expression *lvalue)
{
    Token token = scanner(source);
    Expression *expr;

    switch(token.type){
        case MulOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = MulNode;
            (expr->v).val.op = Mul;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseValue(source);
            return parseTermTail(source, expr);
        case DivOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = DivNode;
            (expr->v).val.op = Div;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseValue(source);
            return parseTermTail(source, expr);
        case Alphabet:
        case PrintOp:
        case PlusOp:
        case MinusOp:
            ungetToken(token);
            return lvalue;
        case EOFsymbol:
            return lvalue;
        default:
            printf("Syntax Error:(parseTerm) Expect a numeric value or an identifier %s\n", token.tok);
            exit(1);
    }

}

Expression *parseExpressionTail( FILE *source, Expression *lvalue )
{
    Token token = scanner(source);
    Expression *expr;

    switch(token.type){

        case PlusOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = PlusNode;
            (expr->v).val.op = Plus;
            expr->leftOperand = lvalue;
            expr->rightOperand = parsePreTerm(source);
            return parseExpressionTail(source, expr);
        case MinusOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = MinusNode;
            (expr->v).val.op = Minus;
            expr->leftOperand = lvalue;
            expr->rightOperand = parsePreTerm(source);
            return parseExpressionTail(source, expr);
        case Alphabet:
        case PrintOp:
            ungetToken(token);
            return lvalue;
        case EOFsymbol:
            return lvalue;
        default:
            printf("Syntax Error:(parseExpressionTail) Expect a numeric value or an identifier %s\n", token.tok);
            exit(1);
    }
}
Expression *parseExpression( FILE *source, Expression *lvalue )
{
    Token token = scanner(source);
    Expression *expr;

    switch(token.type){
        case PlusOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = PlusNode;
            (expr->v).val.op = Plus;
            expr->leftOperand = lvalue;
            expr->rightOperand = parsePreTerm(source);
            return parseExpressionTail(source, expr);
        case MinusOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = MinusNode;
            (expr->v).val.op = Minus;
            expr->leftOperand = lvalue;
            expr->rightOperand = parsePreTerm(source);
            return parseExpressionTail(source, expr);
        case Alphabet:
        case PrintOp:
            ungetToken(token);
            return NULL;
        case EOFsymbol:
            return NULL;
        default:
            printf("Syntax Error:(parseExpression) Expect a numeric value or an identifier %s\n", token.tok);
            exit(1);
    }
}


Statement parseStatement( FILE *source, Token token )
{
    Token next_token;
    Expression *value, *expr, *term;
    switch(token.type){
        case Alphabet:
            next_token = scanner(source);
            if(next_token.type == AssignmentOp){
                term = parsePreTerm(source);
                expr = parseExpression(source, term);
                return makeAssignmentNode(token.tok, term, expr);
            }
            else{
                printf("Syntax Error: Expect an assignment op %s\n", next_token.tok);
                exit(1);
            }
        case PrintOp:
            next_token = scanner(source);
            if(next_token.type == Alphabet)
                return makePrintNode(next_token.tok);
            else{
                printf("Syntax Error: Expect an identifier %s\n", next_token.tok);
                exit(1);
            }
            break;
        default:
            printf("Syntax Error: Expect a statement %s\n", token.tok);
            exit(1);
    }
}

Statements *parseStatements( FILE * source )
{

    Token token = scanner(source);
    Statement stmt;
    Statements *stmts;

    switch(token.type){
        case Alphabet:
        case PrintOp:
            stmt = parseStatement(source, token);
            stmts = parseStatements(source);
            return makeStatementTree(stmt , stmts);
        case EOFsymbol:
            return NULL;
        default:
            printf("Syntax Error: Expect statements %s\n", token.tok);
            exit(1);
    }
}

Expression *constantFolding(Expression* expr)
{
    if(expr->leftOperand == NULL && expr->rightOperand == NULL)
        return expr;
    Expression *left = constantFolding(expr->leftOperand);
    Expression *right = constantFolding(expr->rightOperand);
    
    if((left->v).type == IntConst){
        if((right->v).type == IntConst){
            int ret;
            switch((expr->v).type){
                case PlusNode: ret = (left->v).val.ivalue + (right->v).val.ivalue; break;
                case MinusNode: ret = (left->v).val.ivalue - (right->v).val.ivalue; break;
                case MulNode: ret = (left->v).val.ivalue * (right->v).val.ivalue; break; 
                case DivNode: ret = (left->v).val.ivalue / (right->v).val.ivalue; break;   
                default: printf("Syntax error while constant folding: expect an operation op\n"); exit(1);
            }
            expr->type = Int;
            (expr->v).type = IntConst;
            (expr->v).val.ivalue = ret;
            expr->leftOperand = expr->rightOperand = NULL;
        }
        else if((right->v).type == FloatConst){
            float ret;
            switch((expr->v).type){
                case PlusNode: ret = (left->v).val.ivalue + (right->v).val.fvalue; break;
                case MinusNode: ret = (left->v).val.ivalue - (right->v).val.fvalue; break;
                case MulNode: ret = (left->v).val.ivalue * (right->v).val.fvalue; break; 
                case DivNode: ret = (left->v).val.ivalue / (right->v).val.fvalue; break;   
                default: printf("Syntax error while constant folding: expect an operation op\n"); exit(1);
            }
            expr->type = Float;
            (expr->v).type = FloatConst;
            (expr->v).val.fvalue = ret;
            expr->leftOperand = expr->rightOperand = NULL;
        }
    }
    else if((left->v).type == FloatConst){
        if((right->v).type == IntConst){
            float ret;
            switch((expr->v).type){
                case PlusNode: ret = (left->v).val.fvalue + (right->v).val.ivalue; break;
                case MinusNode: ret = (left->v).val.fvalue - (right->v).val.ivalue; break;
                case MulNode: ret = (left->v).val.fvalue * (right->v).val.ivalue; break; 
                case DivNode: ret = (left->v).val.fvalue / (right->v).val.ivalue; break;   
                default: printf("Syntax error while constant folding: expect an operation op\n"); exit(1);
            }
            expr->type = Float;
            (expr->v).type = FloatConst;
            (expr->v).val.fvalue = ret;
            expr->leftOperand = expr->rightOperand = NULL;
        }
        else if((right->v).type == FloatConst){
            float ret;
            switch((expr->v).type){
                case PlusNode: ret = (left->v).val.fvalue + (right->v).val.fvalue; break;
                case MinusNode: ret = (left->v).val.fvalue - (right->v).val.fvalue; break;
                case MulNode: ret = (left->v).val.fvalue * (right->v).val.fvalue; break; 
                case DivNode: ret = (left->v).val.fvalue / (right->v).val.fvalue; break;   
                default: printf("Syntax error while constant folding: expect an operation op\n"); exit(1);
            }
            expr->type = Float;
            (expr->v).type = FloatConst;
            (expr->v).val.fvalue = ret;
            expr->leftOperand = expr->rightOperand = NULL;
        }
    }
    return expr;
}

/*********************************************************************
  Build AST
 **********************************************************************/
Declaration makeDeclarationNode( Token declare_type, Token identifier )
{
    Declaration tree_node;

    switch(declare_type.type){
        case FloatDeclaration:
            tree_node.type = Float;
            break;
        case IntegerDeclaration:
            tree_node.type = Int;
            break;
        default:
            break;
    }
    strcpy(tree_node.name, identifier.tok);

    return tree_node;
}

Declarations *makeDeclarationTree( Declaration decl, Declarations *decls )
{
    Declarations *new_tree = (Declarations *)malloc( sizeof(Declarations) );
    new_tree->first = decl;
    new_tree->rest = decls;

    return new_tree;
}


Statement makeAssignmentNode( char *name, Expression *term, Expression *expr_tail )
{
    Statement stmt;
    AssignmentStatement assign;

    stmt.type = Assignment;
    strcpy(assign.id ,name);
    if(expr_tail == NULL)
        assign.expr = term;
    else
        assign.expr = expr_tail;

//*********************************
//    add constant folding    
//*********************************
    assign.expr = constantFolding(assign.expr);
    stmt.stmt.assign = assign;

    return stmt;
}

Statement makePrintNode( char *name )
{
    Statement stmt;
    stmt.type = Print;
    strcpy(stmt.stmt.variable, name);

    return stmt;
}

Statements *makeStatementTree( Statement stmt, Statements *stmts )
{
    Statements *new_tree = (Statements *)malloc( sizeof(Statements) );
    new_tree->first = stmt;
    new_tree->rest = stmts;

    return new_tree;
}

/* parser */
Program parser( FILE *source )
{
    Program program;

    program.declarations = parseDeclarations(source);
    program.statements = parseStatements(source);

    return program;
}


/********************************************************
  Build symbol table
 *********************************************************/
void InitializeTable( SymbolTable *table )
{
    int i;
    for(i = 0 ; i < 23; i++){
        table->record[i].type = Notype;
        strcpy(table->record[i].id, "");
    }
}

void add_table( SymbolTable *table, char *name, DataType t )
{

    //hash index
    int index = 0, i=0;
    int len = strlen(name);
    
    for(i=0; i<len; i++)
        index = (name[i]+index);
    index %= 23;

    //collision, hash to next
    while(table->record[index].type != Notype) 
        index = (index + 5)%23;
    
    //add data
    table->record[index].type = t;
    strcpy(table->record[index].id, name); 
}

SymbolTable build( Program program )
{
    SymbolTable table;
    Declarations *decls = program.declarations;
    Declaration current;

    InitializeTable(&table);

    while(decls !=NULL){
        current = decls->first;
        add_table(&table, current.name, current.type);
        decls = decls->rest;
    }

    return table;
}


/********************************************************************
  Type checking
 *********************************************************************/

void convertType( Expression * old, DataType type )
{
    if(old->type == Float && type == Int){
        printf("error : can't convert float to integer\n");
        return;
    }
    if(old->type == Int && type == Float){
        Expression *tmp = (Expression *)malloc( sizeof(Expression) );
        if(old->v.type == Identifier)
            printf("convert to float %s \n",old->v.val.id);
        else
            printf("convert to float %d \n", old->v.val.ivalue);
        tmp->v = old->v;
        tmp->leftOperand = old->leftOperand;
        tmp->rightOperand = old->rightOperand;
        tmp->type = old->type;

        Value v;
        v.type = IntToFloatConvertNode;
        v.val.op = IntToFloatConvert;
        old->v = v;
        old->type = Int;
        old->leftOperand = tmp;
        old->rightOperand = NULL;
    }
}

DataType generalize( Expression *left, Expression *right )
{
    if(left->type == Float || right->type == Float){
        printf("generalize : float\n");
        return Float;
    }
    printf("generalize : int\n");
    return Int;
}

char variableToRegister( SymbolTable *table, char *name){
    int i=0, index = 0;

    // hash string to index
    int len = strlen(name);
    for(i = 0; i < len; i++)
        index = (index + name[i]);
    index %= 23;
    while(strcmp(table->record[index].id, name) != 0)
        index = (index + 5) % 23;

    // return mapped register name
    return 'a'+index;

}

DataType lookup_table( SymbolTable *table, char *name )
{

    int i=0, index = 0;

    // hash string to index
    int len = strlen(name);
    for(i = 0; i < len; i++)
        index = (index + name[i]);
    index %= 23;
    while(strcmp(table->record[index].id, name) != 0)
        index = (index + 5) % 23;
    // return data type
    return table->record[index].type;
}

void checkexpression( Expression * expr, SymbolTable * table )
{
    char name[256];
    if(expr->leftOperand == NULL && expr->rightOperand == NULL){
        switch(expr->v.type){
            case Identifier:
                strcpy(name, expr->v.val.id);
                expr->v.val.regId = variableToRegister(table, name); 
                printf("identifier : %s(%c)\n", name, expr->v.val.regId);
                expr->type = lookup_table(table, name);
                break;
            case IntConst:
                printf("constant : int\n");
                expr->type = Int;
                break;
            case FloatConst:
                printf("constant : float\n");
                expr->type = Float;
                break;
                //case PlusNode: case MinusNode: case MulNode: case DivNode:
            default:
                break;
        }
    }
    else{
        Expression *left = expr->leftOperand;
        Expression *right = expr->rightOperand;

        checkexpression(left, table);
        checkexpression(right, table);

        DataType type = generalize(left, right);
        convertType(left, type);//left->type = type;//converto
        convertType(right, type);//right->type = type;//converto
        expr->type = type;
    }
}

void checkstmt( Statement *stmt, SymbolTable * table )
{
    if(stmt->type == Assignment){
        AssignmentStatement assign = stmt->stmt.assign;
        printf("assignment : %s \n",assign.id);
        checkexpression(assign.expr, table);

        stmt->stmt.assign.type = lookup_table(table, assign.id);
        stmt->stmt.assign.regId = variableToRegister(table, assign.id);

        if (assign.expr->type == Float && stmt->stmt.assign.type == Int) {
            printf("error : can't convert float to integer\n");
        } else {
            convertType(assign.expr, stmt->stmt.assign.type);
        }
    }
    else if (stmt->type == Print){
        printf("print : %s \n",stmt->stmt.variable);
        stmt->stmt.regId = variableToRegister(table, stmt->stmt.variable);
    }
    else printf("error : statement error\n");//error
}

void check( Program *program, SymbolTable * table )
{
    Statements *stmts = program->statements;
    while(stmts != NULL){
        checkstmt(&stmts->first,table);
        stmts = stmts->rest;
    }
}


/***********************************************************************
  Code generation
 ************************************************************************/
void fprint_op( FILE *target, ValueType op )
{
    switch(op){
        case MinusNode:
            fprintf(target,"-\n");
            break;
        case PlusNode:
            fprintf(target,"+\n");
            break;
        case MulNode:
            fprintf(target,"*\n");
            break;
        case DivNode:
            fprintf(target,"/\n");
            break;
        default:
            fprintf(target,"Error in fprintf_op ValueType = %d\n",op);
            break;
    }
}

char nameToReg(){}

void fprint_expr( FILE *target, Expression *expr)
{

    if(expr->leftOperand == NULL){
        switch( (expr->v).type ){
            case Identifier:

                fprintf(target,"l%c\n",(expr->v).val.regId);
                break;
            case IntConst:
                fprintf(target,"%d\n",(expr->v).val.ivalue);
                break;
            case FloatConst:
                fprintf(target,"%f\n", (expr->v).val.fvalue);
                break;
            default:
                fprintf(target,"Error In fprint_left_expr. (expr->v).type=%d\n",(expr->v).type);
                break;
        }
    }
    else{
        fprint_expr(target, expr->leftOperand);
        if(expr->rightOperand == NULL){
            fprintf(target,"5k\n");
        }
        else{
            //	fprint_right_expr(expr->rightOperand);
            fprint_expr(target, expr->rightOperand);
            fprint_op(target, (expr->v).type);
        }
    }
}

void gencode(Program prog, FILE * target)
{
    Statements *stmts = prog.statements;
    Statement stmt;

    while(stmts != NULL){
        stmt = stmts->first;
        switch(stmt.type){
            case Print:
                fprintf(target,"l%c\n",stmt.stmt.regId);
                fprintf(target,"p\n");
                break;
            case Assignment:
                fprint_expr(target, stmt.stmt.assign.expr);
                /*
                   if(stmt.stmt.assign.type == Int){
                   fprintf(target,"0 k\n");
                   }
                   else if(stmt.stmt.assign.type == Float){
                   fprintf(target,"5 k\n");
                   }*/
                fprintf(target,"s%c\n",stmt.stmt.assign.regId);
                fprintf(target,"5 k\n");
                break;
        }
        stmts=stmts->rest;
    }

}


/***************************************
  For our debug,
  you can omit them.
 ****************************************/
void print_expr(Expression *expr)
{
    if(expr == NULL)
        return;
    else{
        print_expr(expr->leftOperand);
        switch((expr->v).type){
            case Identifier:
                printf("%s ", (expr->v).val.id);
                break;
            case IntConst:
                printf("%d ", (expr->v).val.ivalue);
                break;
            case FloatConst:
                printf("%f ", (expr->v).val.fvalue);
                break;
            case PlusNode:
                printf("+ ");
                break;
            case MinusNode:
                printf("- ");
                break;
            case MulNode:
                printf("* ");
                break;
            case DivNode:
                printf("/ ");
                break;
            case IntToFloatConvertNode:
                printf("(float) ");
                break;
            default:
                printf("error ");
                break;
        }
        print_expr(expr->rightOperand);
    }
}

void test_parser( FILE *source )
{
    Declarations *decls;
    Statements *stmts;
    Declaration decl;
    Statement stmt;
    Program program = parser(source);

    decls = program.declarations;

    while(decls != NULL){
        decl = decls->first;
        if(decl.type == Int)
            printf("i ");
        if(decl.type == Float)
            printf("f ");
        printf("%s ",decl.name);
        decls = decls->rest;
    }

    stmts = program.statements;

    while(stmts != NULL){
        stmt = stmts->first;
        if(stmt.type == Print){
            printf("p %s ", stmt.stmt.variable);
        }

        if(stmt.type == Assignment){
            printf("%s = ", stmt.stmt.assign.id);
            print_expr(stmt.stmt.assign.expr);
        }
        stmts = stmts->rest;
    }

}
