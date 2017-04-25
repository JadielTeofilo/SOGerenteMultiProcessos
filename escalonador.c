#include "escalonador.h"




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


jobTableType * ler_estrutura(int idfila){
  
  jobTableType * estrutura;
  printf("%d\n", idfila);

  //Recebe o ultimo job
  if (msgrcv(idfila, &estrutura, sizeof(estrutura), 0, 0) < 0){
  //if (msgrcv(idfila, &job_anterior, sizeof(job_anterior)-sizeof(long), 0, 0) < 0){

    printf("Nenhum estrutura na fila\n");
    exit(1);
  }
    printf("job = %d\n", estrutura->job);
    printf("arquivo = %s\n", estrutura->arq_exec);
   // printf("%s\n", estrutura->data);
    //printf("mensagem recebida = %d\n", job_anterior.job_num);



    return estrutura;
}


void escalonar(){
	jobNumType job_anterior;
    job_anterior.job_num = -1;
	int idfila_job = -1;
  int idfila_estrutura = -1;

   	idfila_job = criar_fila(0x1223);
   	//envia de inicio para a fila de jobs utilizados o -1
   	enviar_num_job(job_anterior, idfila_job);

    //printf("oi\n");

   	sleep(10);
    idfila_estrutura = criar_fila(0x1224);

    ler_estrutura(idfila_estrutura);

    excluir_fila(idfila_estrutura);
   	excluir_fila(idfila_job);

}




int main(int argc, char const *argv[])
{
	escalonar();

	return 0;
}