#include "executa_postergado.h"


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

int criar_fila(int key){

   	int idfila = 0; 
	/* cria a fila*/
   	if ((idfila = msgget(key, IPC_CREAT|0x1B6)) < 0)
   	{
     	printf("erro na criacao da fila\n");
     	exit(1);
   	}
   	// printf("id escalonar = %d\n", idfila);
   	return idfila;
}

void excluir_fila(int idfila){
   	msgctl(idfila, IPC_RMID, 0);
}


void ler_estrutura(int idfila){
  jobTableType * mensagem_ptr = (jobTableType*) malloc(sizeof(jobTableType));

  //Recebe o ultimo job
  precisa corrigir isso, de resto acho q teoricamente funciona, tah dando overflow
  if (msgrcv(idfila, mensagem_ptr, sizeof(mensagem_ptr), 0, 0) < 0){
  //if (msgrcv(idfila, &estrutura, sizeof(estrutura)-sizeof(long), 0, IPC_NOWAIT) < 0){
    printf("Nenhum estrutura na fila\n");
    exit(1);
  }

    printf("job = %d\n\n", mensagem_ptr->job);
    //printf("arquivo = %s\n", mensagem_ptr->arq_exec);

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

    ler_estrutura(idfila_estrutura);

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