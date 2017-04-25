all: build1 build2 clean

build1: executa_postergado.o
	gcc executa_postergado.o -o at

executa_postergado.o: executa_postergado.c
	gcc -c executa_postergado.c executa_postergado.h

build2: escalonador.o
	gcc escalonador.o -o escalonador

escalonador.o: escalonador.c
	gcc -c escalonador.c escalonador.h


clean: 
	rm *.o