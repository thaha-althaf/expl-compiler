%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "trans.c"
    int first = 1, addr = 2040;
    FILE *fp;
%}
%%

(L|F)[0-9]+:\n  {
	    	if (first)
	    	{
	    		char *labelname=malloc(strlen(yytext));
	    		strcpy(labelname,yytext);
	    		labelname[strlen(yytext)-2]='\0';
	    		add(labelname,addr);
	    	}
	    }

(L|F)[0-9]+ {
            if (!first) 
       			 fprintf(fp, "%d", getAddr(yytext));
            }

	   
\n {
   	addr+=2;
   	if(!first)
   	fprintf(fp,"\n");
   }
   
. {
        if(!first)
   	fprintf(fp,"%s",yytext);
  }
%%

int yywrap() {
    if (first) {
        yyin = fopen("target.xsm", "r");
        first = 0;
        return 0;
    }
    if (!first) {
        return 1;
    }
}

int main() {
    fp = fopen("output.xsm", "w");
    extern FILE *yyin;
    yyin = fopen("target.xsm", "r");
    yylex();
    return 0;
}
