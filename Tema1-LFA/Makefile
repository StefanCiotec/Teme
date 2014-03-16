build:
	bison -d reg.y
	flex reg.l
	g++ reg.tab.c lex.yy.c -lfl -o Main
			
clean:
	rm -f Main
