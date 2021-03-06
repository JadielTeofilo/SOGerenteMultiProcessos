/***

Programa serve para a comunicacao com o escalonador,
ele avisa ao escalonador a solicitacao de execucao 
de um novo job apos determinada quantidade de segun-
dos



Input 

<seg>: delay de execucao em relacao a hora corrente
<arq_exec>: Nome do arquivo a ser executado 

Output

Atribuir um numero de job unico a tupla <arq_exec> 
Colocar na estrutura de dados compartilhado e enviar 
para a fila de mensagens

**/

#include "executa_postergado.h"
#include <unistd.h>



//Verifica se o numero de segundos entrado eh um numero
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
//verifica se o numero de parametros eh o correto
int verificar_num_param(int num){

	//Verifica o numero de parametros
	if (num != 3){
		printf("Num de parametros invalido\n");
		exit(0);
	}
}
//verifica se existe arquivo executavel
int existe_arquivo(char * arqin){
	if(access(arqin, F_OK)==-1){
		printf("arquivo invalido\n");
		exit(0);
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

	//Recebe o ultimo job
	if (msgrcv(idfila, &job_anterior, sizeof(job_anterior), 0, IPC_NOWAIT) < 0){
	//if (msgrcv(idfila, &job_anterior, sizeof(job_anterior)-sizeof(long), 0, 0) < 0){
		printf("Nenhum numero de job encontrado na fila\n");
		exit(1);
	}
   	num_aux = job_anterior.job_num;

   	return num_aux;
}


void criar_enviar_novo_job(int job_anterior, const char* arq_exec, const char* segundos){
	
	int idfila;
	jobInfoType mensagem;

	//Define o tipo
	mensagem.type = 1;
	//define o identificador unico do job
	mensagem.job = job_anterior;

	//aloca e passa o nome do executavel para a estrutura
	strcpy(mensagem.arq_exec, arq_exec);

    /* Obter horario atual e somar com os segundos requisitados*/
	mensagem.data = time(NULL) + atoi(segundos);
	mensagem.submissao = time(NULL);

    //Soh pode escrever na fila caso ela jah exista, utilizando o seu id
    if(msgget(0x1224, 0x1B6) < 0){
        printf("Nenhuma fila encontrada \n");
        exit(1);
    }
	idfila = msgget(0x1224, 0x1B6);

	//enviar o novo job inserido pelo usuário para tratamento
    if(msgsnd(idfila, &mensagem, sizeof(mensagem), 0) < 0){
    //if(msgsnd(idfila, &mensagem, sizeof(mensagem)-sizeof(long), 0) < 0){
		printf("Problema ao enviar as info do novo job\n");
	}
}

int main(int argc, char *argv[])
{
	time_t current_time;
	char* c_time_string;
	int job_anterior = -1;
	const char * seg = argv[1];

	//Verifica se o numero de parametros esta certo
	verificar_num_param(argc);

	
	//Verifica se o numero de segundos eh um numero
	is_num(seg);

	//verifica se o arquivo existe
	existe_arquivo(argv[2]);

	//Pega o job anterior atraves de uma mensagem do escalonador
	job_anterior = pegar_ultimo_job();

    

	//Cria e evia estrutura com arq_exec, novo job e data
	criar_enviar_novo_job(job_anterior, argv[2], argv[1]);

	//pega o pid do escalonador
	int id_shm = shmget(0x1323,sizeof(int), 0x1B6);
    int *esc_pid = shmat(id_shm, 0, 0x1B6);

    //Avisa que chegou um job
    kill(*esc_pid, SIGUSR2);

	return 0;
}

