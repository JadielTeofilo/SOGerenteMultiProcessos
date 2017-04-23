/***

Input 

<seg>: delay de execucao em relacao a hora corrente
<arq_exec>: Nome do arquivo a ser executado 

Output

Atribuir um numero de job unico a tupla <arq_exec> 
Colocar na estrutura de dados compartilhado 
**/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

struct jobTable
{
	int job;
	char * arq_exec;
	time_t data;

}typedef jobTableType;



//Verifica se o numero de segundos eh um numero
int is_num(const char * argv){
	int tam = strlen(argv);

	for (int i = 0; i < tam; ++i)
	{
		if(!isdigit(argv[i])){
			printf("Parametro invalido\n");
			exit(0);
		}
	}
}


int main(int argc, char *argv[])
{
	time_t current_time;
	char* c_time_string;
	int job_anterior = -1;
	int idfila;
	jobTableType mensagem;

	//Verifica o numero de parametros
	if (argc != 3){
		printf("Num de parametros invalido\n");
		exit(0);
	}


	const char * seg = argv[1];
	
	//Verifica se o numero de segundos eh um numero
	is_num(seg);


	if(msgget(0x1223, 0x1B6) < 0){
		printf("Nenhuma fila encontrada \n");
		exit(1);
	}

	idfila = msgget(0x1223, 0x1B6);
	// printf("%d\n", idfila);

	//Recebe o ultimo job
	if (msgrcv(idfila, &job_anterior, sizeof(job_anterior)-sizeof(long), 0, IPC_NOWAIT) < 0){
		printf("Nenhum numero de job encontrado na fila\n");
		exit(1);
	}
   	// printf("mensagem recebida = %d\n", job_anterior);

	//Criar estrutura para colocar arq_exec novo job e data
	mensagem.job = job_anterior + 1;
	mensagem.arq_exec = (char*) malloc(strlen(argv[2])*sizeof(char));
    /* Obtain current time. */
	mensagem.data = time(NULL) + atoi(argv[1]);
	 
    /* Conveter para string a data*/
    c_time_string = ctime(&mensagem.data);
    printf("job = %d\n", mensagem.job);
    printf("arquivo = %s\n", mensagem.arq_exec);
    printf("%s\n", c_time_string);

	return 0;
}

