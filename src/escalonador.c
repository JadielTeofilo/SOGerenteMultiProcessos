#include "escalonador.h"



void enviar_num_job(jobNumType job_anterior, int idfila){
    if(msgsnd(idfila, &jobNpid, sizeof(jobNpid), 0) < 0){
    //if(msgsnd(idfila, &jobNpid, sizeof(jobNpid), 0), 0) >= 0){
        printf("Erro no envio da mensagem p/ a fila de mensagem\n");
    }
}

jobTableType receber_info_job(int idfila){
    jobTableType mensagem;
    jobTableType mensagem_aux;
    char* c_time_string;

    //Recebe o ultimo job
    //precisa corrigir isso, de resto acho q teoricamente funciona, tah dando overflow
    if (msgrcv(idfila, &mensagem, sizeof(mensagem), 0, 0) < 0){
    //if (msgrcv(idfila, &mensagem, sizeof(mensagem)-sizeof(long), 0, 0) < 0){
        printf("Nenhuma mensagem na fila\n");
        exit(1);
    }

    // printf("job = %d\n\n", mensagem.job);
    //printf("arquivo = %s\n", mensagem.arq_exec);
    // Conveter para string a data
    c_time_string = ctime(&mensagem.data);
    printf("::job = %d\n", mensagem.job);
    printf("::arquivo = %s\n", mensagem.arq_exec);
    printf("::%s\n", c_time_string);
    mensagem_aux = mensagem;
    return mensagem_aux;
}


void escalonar(){

	jobNumType job_anterior;
    job_anterior.job_num = 1;
	int idfila_job_index = -1;
    int idfila_estrutura = -1;

    //Cria filas para comunicacao
   	idfila_job_index = criar_fila(0x1223);
    idfila_estrutura = criar_fila(0x1224);

    //envia de inicio para a fila de jobs utilizados o -1
    enviar_num_job(job_anterior, idfila_job_index);


    receber_info_job(idfila_estrutura);

    //printf("job = %d\n", mensagem_ptr->job);
    //printf("arquivo = %s\n", mensagem_ptr->arq_exec);
    excluir_fila(idfila_job_index);
    excluir_fila(idfila_estrutura);

}




int main(int argc, char const *argv[])
{
	escalonar();

	return 0;
}