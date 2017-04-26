#include "executa_postergado.h"
#include "fila_mensagem"

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>



void enviar_num_job(jobNumType job_anterior, int idfila){
    msgsnd(idfila, &job_anterior, sizeof(job_anterior), 0);
   	//msgsnd(idfila, &job_anterior, sizeof(job_anterior)-sizeof(long), 0);
}

void ler_estrutura_da_fila(int idfila){
    jobTableType * mensagem_ptr = (jobTableType*) malloc(sizeof(jobTableType));

    //Verifica a existencia de filas 
    if(msgget(0x1224, 0x1B6) < 0){
        printf("Nenhuma fila encontrada \n");
        exit(1);
    }


    //Recebe o ultimo job
    //precisa corrigir isso, de resto acho q teoricamente funciona, tah dando overflow
    if (msgrcv(idfila, &mensagem_ptr, sizeof(mensagem_ptr), 0, 0) < 0){
    //if (msgrcv(idfila, &estrutura, sizeof(estrutura)-sizeof(long), 0, IPC_NOWAIT) < 0){
        printf("Nenhum estrutura na fila\n");
        exit(1);
    }

    printf("job = %d\n\n", mensagem_ptr->job);
    printf("arquivo = %s\n", mensagem_ptr->arq_exec);

//return mensagem_ptr;
}


void escalonar(){
    struct jobTable mensagem_ptr;

	jobNumType job_anterior;
    job_anterior.job_num = 1;
	int idfila = -1;
    int idfila_estrutura = -1;

   	idfila = criar_fila(0x1223);
   	//envia de inicio para a fila de jobs utilizados o -1
   	enviar_num_job(job_anterior, idfila);

    idfila_estrutura = criar_fila(0x1224);

    ler_estrutura_da_fila(idfila_estrutura);

    //printf("job = %d\n", mensagem_ptr->job);
    //printf("arquivo = %s\n", mensagem_ptr->arq_exec);
    excluir_fila(idfila);
    excluir_fila(idfila_estrutura);

}




int main(int argc, char const *argv[])
{
	escalonar();

	return 0;
}