#!/bin/sh
lex abs.l
yacc -d abs.y
gcc lex.yy.c y.tab.c -o a.exe
./a.exe
lex trans.l
gcc lex.yy.c -o a.exe
./a.exe

