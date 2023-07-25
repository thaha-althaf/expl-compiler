# Expl-compiler
The is a project that involves creating a compiler for a custom experimental language called "Expl". The compiler is designed using Lex, Yacc, and C programming language.<br>
Expl is a simple language with its syntax and semantics inspired by various programming paradigms, 
making it a suitable platform for experimenting with language design and compilation techniques.<br>

## Pre-requisites
1. Instal lex and yacc. Use the command below <br>
```sudo apt-get update``` <br> ```sudo apt-get install flex``` <br> ```sudo apt-get install bison```.
2. Instal XSM Machine Simulator <br>
   Download link : ```https://silcnitc.github.io/files/xsm_expl.tar.gz```<br>
   Extract this file and navigate into the folder xsm_expl through the terminal.<br>
   Type "make".<br>
   <sub>You may get some warnings and they can be ignored. If you get any fatal error, then install the following dependencies and try running "make" again :<br>```sudo apt-get install libreadline-dev sudo apt-get install libc6-dev ``` </sub><br><br>
   Type ```Type "cd ../xfs-interface/" and type "./init".```.


## How to run
Step 1 : Download ```abs.c``` , ```abs.h``` , ```abs.l``` , ```abs.y``` , ```trans.l``` , ```trans.c``` and ```magic.sh ```.<br>
Step 2 : Create a text (```.txt```) file with the new language.<br>
Step 3 : Run ```./magic.sh``` to generates executable files.<br>
Step 4 : Go to xsm_expl folder and run  ```./xsm -e <relative_path_to_output.xsm>```.<br>
