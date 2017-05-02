/*
	
		SHUTDOWN - 	mata todos os processos e retorna as informações

*/
#include "executa_postergado.h"
#include "fila_mensagem.h"
#include "tabela_job.h"

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
