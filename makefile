all: build

build: executa_postergado.o
	gcc executa_postergado.o -o exe

executa_postergado.o: executa_postergado.c
	gcc -c executa_postergado.c executa_postergado.h


clean: 
	rm *.o