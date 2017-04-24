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




void escalonar(){

    int job_anterior;

   	int idfila = 0; 
 
   	/* cria a fila*/
   	if ((idfila = msgget(0x1223, IPC_CREAT|0x1B6)) < 0)
   	{
     	printf("erro na criacao da fila\n");
     	exit(1);
   	}
   	// printf("%d\n", idfila);

   	job_anterior = 42;
   	msgsnd(idfila, &job_anterior, sizeof(job_anterior)-sizeof(long), 0);
   	sleep(60);
   	msgctl(idfila, IPC_RMID, 0);
}




int main(int argc, char const *argv[])
{
	escalonar();

	return 0;
}