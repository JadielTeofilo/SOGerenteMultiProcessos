/***

Input 

<seg>: delay de execucao em relacao a hora corrente
<arq_exec>: Nome do arquivo a ser executado 

Output

Atribuir um numero de job unico a tupla <arq_exec> 
Colocar na estrutura de dados compartilhado e enviar para a fila de
mensagens
**/

#include "executa_postergado.h"



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
	
	jobNumType job_anterior;
	job_anterior.job_num = 1;
	int num_aux = 0;
	int idfila;

	//Verifica a existencia de filas 
	if(msgget(0x1223, 0x1B6) < 0){
		printf("Nenhuma fila encontrada \n");
		exit(1);
	}

	idfila = msgget(0x1223, 0x1B6);
	// printf("%d\n", idfila);

	//Recebe o ultimo job
	if (msgrcv(idfila, &job_anterior, sizeof(job_anterior), 0, IPC_NOWAIT) < 0){
	//if (msgrcv(idfila, &job_anterior, sizeof(job_anterior)-sizeof(long), 0, 0) < 0){
		printf("Nenhum numero de job encontrado na fila\n");
		exit(1);
	}
   	printf("mensagem recebida = %d\n", job_anterior.job_num);

   	num_aux = job_anterior.job_num;

   	return num_aux;
}


void criar_enviar_estrutura(int job_anterior, const char* arq_exec, const char* segundos){
	
	int idfila;
	jobTableType mensagem;
	//define o identificador unico do job como o anterior + 1
	mensagem.job = job_anterior + 1;
	
	//aloca e passa o nome do executavel para a estrutura
	strcpy(mensagem.arq_exec, arq_exec);

    /* Obter horario atual e somar com os segundos requisitados*/
	mensagem.data = time(NULL) + atoi(segundos);

    //Verifica a existencia de filas 
    if(msgget(0x1224, 0x1B6) < 0){
        printf("Nenhuma fila encontrada \n");
        exit(1);
    }
    //acha id da fila
	idfila = msgget(0x1224, 0x1B6);
	//enviar a fila
    if(msgsnd(idfila, &mensagem, sizeof(mensagem), 0) >= 0){
    //if(msgsnd(idfila, &mensagem, sizeof(mensagem)-sizeof(long), 0) >= 0){
		printf("mensagem enviada\n");
	}

}

int main(int argc, char *argv[])
{
	time_t current_time;
	char* c_time_string;
	int job_anterior = -1;
	const char * seg = argv[1];

	//Verifica o numero de parametros
	if (argc != 3){
		printf("Num de parametros invalido\n");
		exit(0);
	}


	
	//Verifica se o numero de segundos eh um numero
	is_num(seg);

	//Pega o job anterior atraves de uma mensagem do escalonador
	job_anterior = pegar_ultimo_job();


	//Cria e evia estrutura com arq_exec novo job e data
	criar_enviar_estrutura(job_anterior, argv[2], argv[1]);



	 
    // // Conveter para string a data
    // c_time_string = ctime(&mensagem.data);
    // printf("::job = %d\n", mensagem.job);
    // printf("::arquivo = %s\n", mensagem.arq_exec);
    // printf("::%s\n", c_time_string);

	return 0;
}
