struct jobTable
{
	int job;
	char * arq_exec;
	char * data;

}typedef jobTableType;


#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>


void enviar_num_job(int job_anterior, int idfila){

   	msgsnd(idfila, &job_anterior, sizeof(job_anterior)-sizeof(long), 0);
}

int criar_fila(){

   	int idfila = 0; 
	/* cria a fila*/
   	if ((idfila = msgget(0x1223, IPC_CREAT|0x1B6)) < 0)
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


void escalonar(){
	int job_anterior = 1;
	int idfila = -1;

   	idfila = criar_fila();
   	//envia de inicio para a fila de jobs utilizados o -1
   	enviar_num_job(job_anterior, idfila);


   	sleep(60);


   	excluir_fila(idfila);
}




int main(int argc, char const *argv[])
{
	escalonar();

	return 0;
}