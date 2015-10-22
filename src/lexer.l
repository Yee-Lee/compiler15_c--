%option noyywrap 
%{
#include <stdio.h>
#include "header.h" 
int linenumber;
symtab * lookup();
symtab * ptr;
void     insertID();
void 	 printSymTab();
char *filename;
/* You need to define for all tokens in C--, here are some examples */
// Reserved Words
#define RW_RETURN       10
#define RW_TYPEDEF      11
#define RW_IF           12
#define RW_ELSE         13
#define RW_INT          14
#define RW_FLOAT        15
#define RW_FOR          16
#define RW_VOID         17
#define RW_WHILE        18
// Constants
#define CONST_INT       19
#define CONST_FLOAT     20
#define CONST_STRING    21
// Identifier
#define IDENTIFIER      22
// Symbols and operators
    // Arithmetic operators
#define OP_ADD          23
#define OP_MINUS        24
#define OP_MUL          25
#define OP_DIV          26
    // Relational operators
#define OP_LT           27 // less than
#define OP_GT           28 // greater than
#define OP_GE           29 // greater or equal
#define OP_LE           30 // less or equal
#define OP_NE           31 // not equal
#define OP_EQ           32 // equal
    // Logical operators
#define OP_OR           33
#define OP_AND          34
#define OP_NOT          35
    // assignment operator
#define OP_ASSIGN 	    36
// Separators
#define MK_LBRACE       37
#define MK_RBRACE       38
#define MK_LBRACKET     39
#define MK_RBRACKET     40
#define MK_LPAREN 	    41
#define MK_RPAREN 	    42
#define MK_SEMICOLON    43
#define MK_COMMA        44
#define MK_DOT          45
// comments
#define COMMENT         46
// Error
#define ERROR 		    100 

%}

letter   [A-Za-z]
digit    [0-9]
ID	 {letter}({letter}|{digit}|"_")*
WS	 [ \t]+
FSuffix  [f|F|l|L]
Int_constant {digit}+

/* You need to define the following RE's 
Float_constant
String_constant
comment
*/
Exponent [eE][+-]?{digit}+
Float_constant ((({digit}{digit}*\.{digit}*)|\.{digit}+){Exponent}?{FSuffix}?)|({digit}{digit}*{Exponent}{FSuffix}?) 
String_constant \"(\\.|[^\\"])*\"
comment \/\*((\*[^/])|[^*])*\*\/

/*============================*/

OP_assign        "="
OP_or   "||"
/* Other operators appear here */
OP_and "&&"
OP_not "!"
OP_add "+"
OP_minus "-"
OP_mul "*"
OP_div "/"
OP_gt ">"
OP_lt "<"
OP_ge ">="
OP_le "<="
OP_ne "!="
OP_eq "=="

/*=============================*/

newline  "\n"

DL_lparen "("
DL_rparen ")"
DL_lbrace "{"
DL_rbrace "}"
DL_comma  ","
DL_semicol ";"
DL_dot  "."

/* Other separators appear here */
DL_lbracket "["
DL_rbracket "]"

/*==============================*/
error    .

%%

{WS}            {}
{Int_constant}  {}
{Float_constant}  printf("float:%s\n", yytext);
{String_constant} printf("str:%s\n", yytext);               

{comment}      printf("comment:%s\n", yytext);         

{ID}            { 
 if(strcmp(yytext, "return") == 0); // return RW_RETURN;
 else if(strcmp(yytext, "typedef") == 0); // return RW_TYPEDEF;
 else if(strcmp(yytext, "if") == 0); // return RW_IF;
 else if(strcmp(yytext, "else") == 0); // return RW_ELSE;
 else if(strcmp(yytext, "int") == 0); // return RW_INT;
 else if(strcmp(yytext, "float") == 0); // return RW_FLOAT;
 else if(strcmp(yytext, "for") == 0); // return RW_FOR;
 else if(strcmp(yytext, "void") == 0); // return RW_VOID;
 else if(strcmp(yytext, "while") == 0); // return RW_WHILE;
 else {
   ptr = lookup(yytext);
   if (ptr == NULL) insertID(yytext);	
   else ptr->counter++;
  }
}

{OP_add}        {}
{OP_minus}      {}
{OP_mul}        {}
{OP_div}        {}
{OP_lt}         {}
{OP_gt}         {}
{OP_ge}         {}
{OP_le}         {}
{OP_ne}         {}
{OP_eq}         {}
{OP_or}         {}
{OP_and}        {}
{OP_not}        {}
{OP_assign}     /* return OP_ASSIGN; */ 
{DL_lparen}     /* return MK_LPAREN; */
{DL_rparen}     /* return MK_RPAREN; */
{DL_lbrace}     /* return MK_LBRACE; */
{DL_rbrace}     /* return MK_RBRACE; */
{DL_comma}      {}
{DL_semicol}    {}
{newline}       linenumber += 1;
{error} {         
  printf("[ERROR] %s: %d: unrecognized token:%s \n", filename, linenumber, yytext);
  /* return ERROR; */
 }

%%

main(int argc, char **argv)
{
 argc--; ++argv;
   if (argc > 0){
      yyin = fopen(argv[0], "r");
      filename = argv[0];
   }
   else{
      yyin = stdin;
      filename = "stdin"; 
   }
   yylex();
   printSymTab();
}

