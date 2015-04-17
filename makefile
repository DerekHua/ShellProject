shell: lex.yy.c y.tab.c
	gcc -g main.c lex.yy.c y.tab.c -o shell
lex.yy.c: y.tab.c shell.lex
	flex shell.lex
y.tab.c: shell.y
	bison -dy shell.y
clean: 
	rm -f lex.yy.c y.tab.c y.tab.h shell
