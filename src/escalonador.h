/**

Escalonador importacoes e definicoes

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
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <signal.h>

struct mensagem_torus_ida
{
	int id_mensagem;
	int job;
	char arq_exec[6969];
	time_t data;


}typedef InfoMsgTorus;

