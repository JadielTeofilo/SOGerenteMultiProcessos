#include "escalonador.h"



void enviar_num_jobNpid(escalonadorInfoType jobNpid, int idfila){
    if(msgsnd(idfila, &jobNpid, sizeof(jobNpid), 0) < 0){
    //if(msgsnd(idfila, &jobNpid, sizeof(jobNpid), 0), 0) >= 0){
        printf("Erro no envio da mensagem p/ a fila de mensagem\n");
    }
}

jobTableType receber_info_job(){
    jobTableType mensagem;
    jobTableType mensagem_aux;
    char* c_time_string;
    int idfila = -1;


    //Verifica a existencia de filas 
    while(msgget(0x1224, 0x1B6) < 0){
        printf("Nenhuma fila encontrada \n");
        exit(1);
    }

    idfila = msgget(0x1224,0x1B6);

    //Recebe o ultimo job
    //precisa corrigir isso, de resto acho q teoricamente funciona, tah dando overflow
    if (msgrcv(idfila, &mensagem, sizeof(mensagem), 0, 0) < 0){
    //if (msgrcv(idfila, &mensagem, sizeof(mensagem)-sizeof(long), 0, 0) < 0){
        printf("Nenhuma mensagem na fila\n");
        exit(1);
    }

    // Conveter para string a data
    // Debug 
    c_time_string = ctime(&mensagem.data);
    printf("::job = %d\n", mensagem.job);
    printf("::arquivo = %s\n", mensagem.arq_exec);
    printf("::%s\n", c_time_string);
    mensagem_aux = mensagem;
    
    return mensagem_aux;
}

void dummie(){}

void escalonar(){

	escalonadorInfoType jobNpid;
    jobNpid.job_num = -1;
    jobNpid.pid_escalonador = getpid();
	int idfila = -1;

    signal(SIGALRM, dummie);

   	idfila = criar_fila(0x1223);
   	
    //envia de inicio para a fila de jobs utilizados o -1
   	enviar_num_jobNpid(jobNpid, idfila);

    pause();
    //pega da fila de mensagens info sobre novo job
    receber_info_job();

    //printf("job = %d\n", mensagem_ptr->job);
    //printf("arquivo = %s\n", mensagem_ptr->arq_exec);
    excluir_fila(idfila);

}




int main(int argc, char const *argv[])
{
	escalonar();

	return 0;
}