all: build1 build2 build3 clean

build1: executa_postergado.o fila_mensagem.o tabela_job.o 
	@mkdir -p ../binary
	gcc executa_postergado.o fila_mensagem.o tabela_job.o -o ../binary/in

executa_postergado.o: executa_postergado.c
	gcc -c executa_postergado.c executa_postergado.h

fila_mensagem.o: fila_mensagem.c
	gcc -c fila_mensagem.c fila_mensagem.h

tabela_job.o: tabela_job.c
	gcc -c tabela_job.c tabela_job.h

build2: escalonador.o fila_mensagem.o tabela_job.o
	gcc escalonador.o fila_mensagem.o tabela_job.o -o ../binary/escalonador

escalonador.o: escalonador.c
	gcc -c escalonador.c escalonador.h

build3: shutdown.o fila_mensagem.o tabela_job.o
	gcc shutdown.o fila_mensagem.o tabela_job.o -o ../binary/shutdown

shutdown.o: shutdown.c
	gcc -c shutdown.c shutdown.h


clean: 
	rm *.o *.gch