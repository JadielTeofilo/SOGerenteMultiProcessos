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

//Recebe mensagem do escalonador com o identificador do ultimo job
int pegar_ultimo_job(){
	
	int job_anterior = 1;
	int idfila;

	if(msgget(0x1223, 0x1B6) < 0){
		printf("Nenhuma fila encontrada \n");
		exit(1);
	}

	idfila = msgget(0x1223, 0x1B6);
	// printf("%d\n", idfila);

	//Recebe o ultimo job
	if (msgrcv(idfila, &job_anterior, sizeof(job_anterior), 0, 0) < 0){
	//if (msgrcv(idfila, &job_anterior, sizeof(job_anterior)-sizeof(long), 0, 0) < 0){
		printf("Nenhum numero de job encontrado na fila\n");
		exit(1);
	}
   	printf("mensagem recebida = %d\n", job_anterior);


   	return job_anterior;
}


//Retorna ponteiro para a estrutura
jobTableType * criar_estrutura_mensagem(int job_anterior, const char * arq_exec, const char * segundos){


	jobTableType * mensagem_ptr = (jobTableType*) malloc(sizeof(jobTableType));


	//define o identificador unico do job como o anterior + 1
	mensagem_ptr->job = job_anterior + 1;
	
	//aloca e passa o nome do executavel para a estrutura
	mensagem_ptr->arq_exec = (char*) malloc(strlen(arq_exec)*sizeof(char));
	strcpy(mensagem_ptr->arq_exec, arq_exec);

    /* Obter horario atual e somar com os segundos requisitados*/
	mensagem_ptr->data = time(NULL) + atoi(segundos);

	return mensagem_ptr;

}

int main(int argc, char *argv[])
{
	time_t current_time;
	char* c_time_string;
	int job_anterior = 1;
	jobTableType * mensagem_ptr;

	//Verifica o numero de parametros
	if (argc != 3){
		printf("Num de parametros invalido\n");
		exit(0);
	}


	const char * seg = argv[1];
	
	//Verifica se o numero de segundos eh um numero
	is_num(seg);

	//Procura pela fila
	job_anterior = pegar_ultimo_job();


	//Criar estrutura para colocar arq_exec novo job e data
	mensagem_ptr = criar_estrutura_mensagem(job_anterior, argv[2], argv[1]);

	 
    // Conveter para string a data
    c_time_string = ctime(&mensagem_ptr->data);
    printf("job = %d\n", mensagem_ptr->job);
    printf("arquivo = %s\n", mensagem_ptr->arq_exec);
    printf("%s\n", c_time_string);

	return 0;
}

