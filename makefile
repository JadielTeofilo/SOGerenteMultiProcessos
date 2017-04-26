all: build1 build2 clean

build1: executa_postergado.o fila_mensagem.o
	gcc executa_postergado.o fila_mensagem.o -o binary/in

executa_postergado.o: src/executa_postergado.c
	gcc -c src/executa_postergado.c src/executa_postergado.h

fila_mensagem.o: src/fila_mensagem.c
	gcc -c src/fila_mensagem.c src/fila_mensagem.h

build2: escalonador.o fila_mensagem.o
	gcc escalonador.o fila_mensagem.o -o binary/escalonador

escalonador.o: src/escalonador.c
	gcc -c src/escalonador.c src/escalonador.h


clean: 
	rm *.o src/*.gch