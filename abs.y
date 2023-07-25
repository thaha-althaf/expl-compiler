%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>

	#include "abs.h"
	#include "abs.c"

	extern FILE* yyin;
	extern char* yytext;
	extern int yylineno;
	struct lsttable *lst;
	struct paramnode *phead;
	int yylex(void);
	void yyerror(char const *s);
%}

%union
{
	struct tnode *no;
	struct paramnode *param	;
	int type;
}

%type <no> prg mainblock body 
%type <no> gdeclblock gdeclist gdecl gidlist gid
%type <no> fnctblock fndef
%type <no> ldeclblock ldeclist ldecl lidlist arglist
%type <no> Slist stmt ip op exp asgn ifstmt whilestmt repeatstmt dowhilestmt 
%type <no> BREAK_ CONTINUE_ ID_ NUM_ TEXT_ id INT_ STR_ strexp
%type <param> paramlist param
%type <type> Type
%token 	PLUS MINUS MUL DIV EQ LE GE LT GT NE
%token  BEGIN_ END_ READ_ WRITE_ IF_ THEN_ ELSE_ ENDIF_ WHILE_ DO_ ENDWHILE_ 
%token  BREAK_ CONTINUE_ REPEAT_ UNTIL_
%token NUM_ ID_ TEXT_ INT_ STR_ DECL_ ENDDECL_ MAIN_ RETURN_
%left LT GT LE GE NE EQ AND OR
%left PLUS MINUS
%left MUL DIV MOD

%start prg

%%

/////////////////////////////////////////////////////////////////////////////////////////////

prg : gdeclblock fnctblock mainblock {
			$$=NULL;
			FILE *ptr = fopen("target.xsm","w");
			fprintf(ptr,"0\n2056\n0\n0\n0\n0\n0\n0\nBRKP\n");
			fprintf(ptr,"MOV SP, %d\n",sp);
			fprintf(ptr,"MOV BP, 4096\n");
			fprintf(ptr,"PUSH R0\n");
			fprintf(ptr,"CALL F0\n");
			fprintf(ptr,"INT 10\n");
			struct tnode* root=(struct tnode *)malloc(sizeof(struct tnode *)); 
			root=$2;
			
			while(strcmp($2->s,"FNCTDEFBLOCK")==0)
			{
			   codegen($2->right,$2->right->gstentry->lst,ptr);
			   $2=$2->left;
			}
			codegen($2,$2->gstentry->lst,ptr);
			codegen($3,$3->gstentry->lst,ptr);
			
			fclose(ptr);
			exit(1);
	}
    | gdeclblock  mainblock {
			$$=NULL;
			FILE *ptr = fopen("target.xsm","w");
			fprintf(ptr,"0\n2056\n0\n0\n0\n0\n0\n0\nBRKP\n");
			fprintf(ptr,"MOV SP, %d\n",sp);
			fprintf(ptr,"MOV BP, 4096\n");
			fprintf(ptr,"PUSH R0\n");
			fprintf(ptr,"CALL F0\n");
			fprintf(ptr,"INT 10\n");
			fprintf(ptr,"F%d:\n",$2->gstentry->flabel);
			codegen($2,$2->gstentry->lst,ptr);
			fclose(ptr);
			exit(1);
	}
    |mainblock {
			$$=NULL;
			FILE *ptr = fopen("target.xsm","w");
			fprintf(ptr,"0\n2056\n0\n0\n0\n0\n0\n0\nBRKP\n");
			fprintf(ptr,"MOV SP, %d\n",sp);
			fprintf(ptr,"MOV BP, 4096\n");
			fprintf(ptr,"PUSH R0\n");
			fprintf(ptr,"CALL F0\n");
			fprintf(ptr,"INT 10\n");
			fprintf(ptr,"F%d:\n",$1->gstentry->flabel);
			codegen($1,$1->gstentry->lst,ptr);
			fclose(ptr);
			exit(1);
	}
    ;
    
/////////////////////////////////////////////////////////////////////////////////////////////

mainblock : INT_ MAIN_ '('')''{'ldeclblock body'}' {
	   	        if($7->right->type!=INTEGER)
	   	        {
	   	            printf("line %d : Return type mismatch for main \n",yylineno);
	   	            exit(1);
	            }
	   	        gstinstall("main",INTEGER,1,FUNCTION,NULL);
	   	        struct gstnode* main=gstlookup("main");
	   	        main->lst=lstcopy(lst);
	   	        lst=lstdelete(lst);
	   	        $$=makenode(FUNCTION,INTEGER,$7,NULL,NULL,main,"main");
			  
	  }
	  ;
	  
/////////////////////////////////////////////////////////////////////////////////////////////

body : BEGIN_ Slist RETURN_ strexp END_ {
			struct tnode *temp=makenode(RET,$4->type,$4,NULL,NULL,NULL,"RETURN");
			$$=makenode(CONNECTOR,VOID,$2,NULL,temp,NULL,"BODY");
     }
     | BEGIN_ RETURN_ strexp END_ {
			struct tnode *temp=makenode(RET,$3->type,$3,NULL,NULL,NULL,"RETURN");
			$$=makenode(CONNECTOR,VOID,NULL,NULL,temp,NULL,"BODY");
     }
     ;
     
/////////////////////////////////////////////////////////////////////////////////////////////                          
    
gdeclblock : DECL_ gdeclist ENDDECL_ {$$=$2;}
           | DECL_  ENDDECL_ {$$=NULL;}
		   ;

gdeclist : gdeclist gdecl
         | gdecl
		 ;

gdecl : Type gidlist {tnodechangetypegst($2,$1);}
      ;

gidlist : gidlist ',' gid {$$=makenode(CONNECTOR,VOID,$1,NULL,$3,NULL,"GIDLIST");}
        | gid 
        ;

gid : ID_                   {gstinstall($1->name,VOID,1,VARIABLE,NULL); $$=$1;}
    | ID_ '[' NUM_ ']'      {gstinstall($1->name,VOID,$3->val,ARRAY,NULL); $$=$1;}
	| ID_ '(' paramlist ')' {
								gstinstall($1->name,VOID,1,FUNCTION,paramcopy(phead)); 
								phead=paramdelete(phead);
							}
	| ID_ '(' ')'           {gstinstall($1->name,VOID,1,FUNCTION,NULL); $$=$1;}
	;

/////////////////////////////////////////////////////////////////////////////////////////////                          

fnctblock : fnctblock fndef {$$ = makenode(CONNECTOR, VOID, $1, NULL, $2, NULL, "FNCTDEFBLOCK");}
          | fndef
		  ;

fndef : Type ID_ '(' paramlist ')' '{' ldeclblock body '}' {
            struct gstnode *f = gstlookup($2->name);
            if (f == NULL) {
                printf("Line %d: Function \"%s\" not declared\n", yylineno, $2->name);
                exit(1);
            }
            if (f->type != $1) {
                printf("Line %d: Invalid return type of function \"%s\"\n", yylineno, $2->name);
                exit(1);
            }
            if (paramcheck(f->phead, phead) == 0) {
                printf("Line %d: Parameter list of function \"%s\" doesn't match\n", yylineno, $2->name);
                exit(1);
            }
            if ($8->right->type != $1) {
                printf("Line %d: Return type of function \"%s\" doesn't match\n", yylineno, $2->name);
                exit(1);
            }
            f->lst = lstcopy(lst);
            lst = lstdelete(lst);
            $$ = makenode(FUNCTION, $1, $8, NULL, paramtoarg(phead), f, f->name);
            phead = paramdelete(phead);
            
		
      }
      | Type ID_ '(' ')' '{' ldeclblock body '}' {
            struct gstnode *f = gstlookup($2->name);
            if (f == NULL) {
                printf("Line %d: Function \"%s\" not declared\n", yylineno, $2->name);
                exit(1);
            }
            if (f->type != $1) {
                printf("Line %d: Invalid return type of function \"%s\"\n", yylineno, $2->name);
                exit(1);
            }
            if (paramcheck(f->phead, phead) == 0) {
                printf("Line %d: Parameter list of function \"%s\" doesn't match\n", yylineno, $2->name);
                exit(1);
            }
            if ($7->right->type != $1) {
                printf("Line %d: Return type of function \"%s\" doesn't match\n", yylineno, $2->name);
                exit(1);
            }
            f->lst = lstcopy(lst);
            lst = lstdelete(lst);
            $$ = makenode(FUNCTION, $1, $7, NULL, NULL, f, f->name);
            
	
      }
	  ;

/////////////////////////////////////////////////////////////////////////////////////////////

paramlist: paramlist ',' param
         | param
         ;

param : Type ID_ {phead = paraminstall(phead, $2->name, $1, VARIABLE);}
      ;

/////////////////////////////////////////////////////////////////////////////////////////////

ldeclblock : DECL_ ldeclist ENDDECL_ {
                lst = lstparaminstall(lst, phead);
                $$ = $2;
               
          }
          | DECL_ ENDDECL_ {
                lst = lstparaminstall(lst, phead);
                $$ = NULL;
                
          }
          | %empty {
                lst = lstparaminstall(lst, phead);
                $$ = NULL;
               
          }
		  ;

ldeclist : ldeclist ldecl
         | ldecl
		 ;

ldecl : Type lidlist  {tnodechangetypelst(lst, $2, $1);}
      ;


lidlist : lidlist ',' ID_ {
            lst = lstinstall(lst, $3->name, 1);
            $$ = makenode(CONNECTOR, VOID, $1, NULL, $3, NULL, "LIDLIST");
        }
        | ID_ {
            lst = lstinstall(lst, $1->name, 1);
            $$ = $1;
        }
		;

/////////////////////////////////////////////////////////////////////////////////////////////

arglist : arglist ',' strexp {$$ = tnodeargappend($1, $3);}
        | strexp
		;

/////////////////////////////////////////////////////////////////////////////////////////////

Type : INT_ {$$ = 0;}
     | STR_ {$$ = 2;}
     ;

/////////////////////////////////////////////////////////////////////////////////////////////

Slist : Slist stmt  {$$=makenode(CONNECTOR,VOID,$1,NULL,$2,NULL,"CONNECTOR");}
      | stmt  {$$=$1;}
      ;

stmt : ip | op | asgn | ifstmt | whilestmt | BREAK_ | CONTINUE_ | dowhilestmt | repeatstmt 
     ;

ip : READ_ '(' id ')'  {$$=makenode(READ,VOID,$3,NULL,NULL,NULL,"READ");}
   ;
op : WRITE_ '(' strexp ')' {$$=makenode(WRITE,VOID,$3,NULL,NULL,NULL,"WRITE");}
   ;
   
asgn : id '=' strexp   {$$=makenode(OPERATOR,INTEGER,$1,NULL,$3,NULL,"=");}
     ;
     
ifstmt : IF_ '(' exp ')' THEN_ Slist ELSE_ Slist ENDIF_ {$$=makenode(IF,VOID,$3,$6,$8,NULL,"IF");}
       | IF_ '(' exp ')' THEN_ Slist ENDIF_ {$$=makenode(IF,VOID,$3,$6,NULL,NULL,"IF");}
       ;

whilestmt :  WHILE_ '(' exp ')' DO_ Slist ENDWHILE_ {$$=makenode(WHILE,VOID,$3,NULL,$6,NULL,"WHILE");}
	  ;

repeatstmt : REPEAT_ Slist UNTIL_ '(' exp ')' {$$=makenode(REPEATUNTIL,VOID,$2,NULL,$5,NULL,"REPEATUNTIL");}
	   ;

dowhilestmt : DO_ Slist WHILE_ '('exp')' {$$=makenode(DOWHILE,VOID,$2,NULL,$5,NULL,"DOWHILE");}
	  
/////////////////////////////////////////////////////////////////////////////////////////////

exp : exp PLUS exp  {$$=makenode(OPERATOR,INTEGER,$1,NULL,$3,NULL,"+");}
    | exp MINUS exp  {$$=makenode(OPERATOR,INTEGER,$1,NULL,$3,NULL,"-");} 
    | exp MUL exp  {$$=makenode(OPERATOR,INTEGER,$1,NULL,$3,NULL,"*");}
    | exp DIV exp  {$$=makenode(OPERATOR,INTEGER,$1,NULL,$3,NULL,"/");} 
    | exp MOD exp  {$$=makenode(OPERATOR,INTEGER,$1,NULL,$3,NULL,"%");} 
    | exp AND exp  {$$=makenode(OPERATOR,BOOLEAN,$1,NULL,$3,NULL,"&&");} 
    | exp OR exp  {$$=makenode(OPERATOR,BOOLEAN,$1,NULL,$3,NULL,"||");} 
    | exp LT exp  {$$=makenode(OPERATOR,BOOLEAN,$1,NULL,$3,NULL,"<");} 
    | exp GT exp  {$$=makenode(OPERATOR,BOOLEAN,$1,NULL,$3,NULL,">");} 
    | exp LE exp  {$$=makenode(OPERATOR,BOOLEAN,$1,NULL,$3,NULL,"<=");} 
    | exp GE exp  {$$=makenode(OPERATOR,BOOLEAN,$1,NULL,$3,NULL,">=");}    
    | exp NE exp  {$$=makenode(OPERATOR,BOOLEAN,$1,NULL,$3,NULL,"!=");} 
    | exp EQ exp  {$$=makenode(OPERATOR,BOOLEAN,$1,NULL,$3,NULL,"==");}   
    | '(' exp ')' {$$=$2;}
    | NUM_ {$$=$1;}
    | id  {$$=$1;}
    ;    

strexp : exp 
       | TEXT_
	   ;

id : ID_ {
	    	$$=$1;
	    	struct lstnode *curr=lstlookup(lst,$1->name);
			if(curr)
			$$->type = curr->type;
			else
	    	{
				struct gstnode *curr=gstlookup($1->name);
				if(curr==NULL)
	    		{
  					printf("Line %d: Variable \"%s\" not declared\n", yylineno, $1->name);	    	
					exit(1);
	    		}
	    		$$->type=curr->type;
			}
   }
   | ID_ '[' exp ']' {
   	          			if($3->type==BOOLEAN)
   	          			{
            				printf("Line %d: Array index cannot be boolean\n", yylineno);
	  	    				exit(1);
	         			}
	          			struct gstnode *curr=gstlookup($1->name);
	 	  				if(curr==NULL)
	  	                {
          					printf("Line %d: Array \"%s\" not declared\n", yylineno, $1->name);
	  	  				    exit(1);
	  	  				}
	  	  				if(curr->typeofvar!=ARRAY)
	  	  				{
          					printf("Line %d: \"%s\" is not an array\n", yylineno, $1->name);
	  	  				    exit(1);
	  	  				}
	          $1->type=curr->type;
	          $$=leafarray($1->name,$3,"ARRAY");
   	}
	| ID_ '(' arglist ')' {
        					struct gstnode *curr = gstlookup($1->name);
        					if (curr == NULL) 
							{
        					    printf("Line %d: Function \"%s\" not declared\n", yylineno, $1->name);
        					    exit(1);
        					}
        					if (curr->typeofvar != FUNCTION) 
							{
        					    printf("Line %d: \"%s\" is not a function\n", yylineno, $1->name);
        					    exit(1);
        					}
        					if (checktnodeparam(curr->phead, $3) == 0) 
							{
          						printf("Line %d: Wrong arguments in \"%s\", does not match with declaration\n", yylineno, $1->name);
            					exit(1);
        					}
        					$1->nodetype = FUNCTIONCALL;
        					$1->type = curr->type;
        					$1->arglist = $3;
        					$$ = $1;
    }
	;
%%

void yyerror(char const *s)
{
	printf("yyerror | Line :%d | %s : %s\n",yylineno,s,yytext);
}

int main(void)
{
        lst=lstinittable();
        phead=NULL;
	yyin=fopen("input.txt","r");
	yyparse();
	return 0;
}
