all: build1 build2 clean

build1: executa_postergado.o fila_mensagem.o
	gcc executa_postergado.o fila_mensagem.o -o ../bin/in

executa_postergado.o: executa_postergado.c
	gcc -c executa_postergado.c executa_postergado.h

fila_mensagem.o: fila_mensagem.c
	gcc -c fila_mensagem.c fila_mensagem.h

build2: escalonador.o fila_mensagem.o
	gcc escalonador.o fila_mensagem.o -o ../bin/escalonador

escalonador.o: escalonador.c
	gcc -c escalonador.c escalonador.h


clean: 
	rm *.o *.gch