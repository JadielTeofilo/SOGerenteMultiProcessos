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

int verificar_num_param(int num){

	//Verifica o numero de parametros
	if (num != 3){
		printf("Num de parametros invalido\n");
		exit(0);
	}
}

//Recebe mensagem do escalonador com o identificador do ultimo job
escalonadorInfoType pegar_ultimo_jobNpid(){
	
	escalonadorInfoType job_anteriorNpid;
	escalonadorInfoType job_aux;
	int idfila;

	//Verifica a existencia de filas 
	if(msgget(0x1223, 0x1B6) < 0){
		printf("Nenhuma fila encontrada \n");
		exit(1);
	}
	//Pegar o id da fila
	idfila = msgget(0x1223, 0x1B6);
	// printf("%d\n", idfila);

	//Recebe o ultimo job
	if (msgrcv(idfila, &job_anteriorNpid, sizeof(job_anteriorNpid), 0, IPC_NOWAIT) < 0){
	//if (msgrcv(idfila, &job_anteriorNpid, sizeof(job_anteriorNpid)-sizeof(long), 0, 0) < 0){
		printf("Nenhum numero de job encontrado na fila\n");
		exit(1);
	}
   	printf("mensagem recebida = %d\n", job_anteriorNpid.job_num);
   	printf("mensagem recebida = %d\n", job_anteriorNpid.pid_escalonador);

   	job_aux = job_anteriorNpid;

   	return job_aux;
}


void criar_enviar_novo_job(int idfila, int job_anterior, const char* arq_exec, const char* segundos){
	
	jobTableType mensagem;
	//define o identificador unico do job como o anterior + 1
	mensagem.job = job_anterior + 1;
	
	//aloca e passa o nome do executavel para a estrutura
	strcpy(mensagem.arq_exec, arq_exec);

    /* Obter horario atual e somar com os segundos requisitados*/
	mensagem.data = time(NULL) + atoi(segundos);


	//enviar a mensagem p a fila
    if(msgsnd(idfila, &mensagem, sizeof(mensagem), 0) < 0){
    //if(msgsnd(idfila, &mensagem, sizeof(mensagem)-sizeof(long), 0) >= 0){
		printf("Erro no envio da mensagem p/ a fila de mensagem\n");
	}
	


}

int main(int argc, char *argv[])
{
	time_t current_time;
	char* c_time_string;
	escalonadorInfoType info_escalonador;
	int job_anterior;
	int pid_escalonador;
	int idfila = -1;
	const char * seg = argv[1];

	//Verificar numero de parametros
	verificar_num_param(argc);

	//Verifica se o numero de segundos eh um numero
	is_num(seg);

	//Pega info do escalonador (da fila de mesnsagem)
	info_escalonador = pegar_ultimo_jobNpid();

	// Pega o pid do escalonador
	pid_escalonador = info_escalonador.pid_escalonador;

	//Pega o job anterior atraves de uma mensagem do escalonador
	job_anterior = info_escalonador.job_num;


	//cria a fila de mensagem para enviar
    idfila = criar_fila(0x1224);
	//Cria e evia estrutura com arq_exec novo job e data
	criar_enviar_novo_job(idfila, job_anterior, argv[2], argv[1]);

	//Avisa o escalonador que foi colocado uma nova mensagem na fila
	kill(pid_escalonador, SIGCONT);

	//vai ser alterado, eh um timeout para excluir a fila
	sleep(1);
	excluir_fila(idfila);


	 
    // // Conveter para string a data
    // c_time_string = ctime(&mensagem.data);
    // printf("::job = %d\n", mensagem.job);
    // printf("::arquivo = %s\n", mensagem.arq_exec);
    // printf("::%s\n", c_time_string);

	return 0;
}

