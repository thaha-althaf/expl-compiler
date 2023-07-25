%{
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	#include "y.tab.h"
	#include "abs.h"

	int number;
%}

%option yylineno

alpha [a-zA-Z_]+

%%

"decl"       {return DECL_;}
"enddecl"    {return ENDDECL_;}
"int"        {return INT_;}
"str"        {return STR_;}
"read"       {return READ_;}
"write"      {return WRITE_;}
"begin"      {return BEGIN_;}
"end"        {return END_;}
"if"         {return IF_;}
"then"       {return THEN_;}
"else"       {return ELSE_;}
"endif"      {return ENDIF_;}
"while"      {return WHILE_;}
"do"         {return DO_;}
"endwhile"   {return ENDWHILE_;}
"repeat"     {return REPEAT_;}
"main"       {return MAIN_;}
"return"     {return RETURN_;}
"until"      {return UNTIL_;}
"break"      { 
	            yylval.no=makenode(BREAK,VOID,NULL,NULL,NULL,NULL,"BREAK");
	            return BREAK_;
			 }
"continue"   {  
	            yylval.no=makenode(CONTINUE,VOID,NULL,NULL,NULL,NULL,"CONTINUE");
	            return CONTINUE_;
		     }
[\"](.)*[\"] {
			   yylval.no=leafnum(STRING,0,yytext);
			   return TEXT_;
			 } 

({alpha}({alpha}|[0-9]+)*)	{
	                           yylval.no = leafvar(yytext,yytext); 
							   return ID_;
							}

[0-9]+	{
	       number = atoi(yytext); 
		   yylval.no = leafnum(INTEGER,number,yytext); 
           return NUM_;
		}
         
"+"	    {return PLUS;}
"-"	    {return MINUS;}
"*"	    {return MUL;}
"/"	    {return DIV;}
"=="	{return EQ;}
"||"	{return OR;}
"&&"	{return AND;}
"<="	{return LE;}
">="	{return GE;}
"<"	    {return LT;}
">"	    {return GT;}
"%"	    {return MOD;}
"!="	{return NE;}
"="	    {return *yytext;}
","	    {return *yytext;}
" "	    {}
";"     {}
[\t]	{}
[\(\)]	{return *yytext;}
[\n]	{}
"["	    {return *yytext;}
"]"	    {return *yytext;}
"{"	    {return *yytext;}
"}"	    {return *yytext;}
.	    {yyerror("unknown character\n");exit(1);}

%%

int yywrap(void) {
	return 1;
}
