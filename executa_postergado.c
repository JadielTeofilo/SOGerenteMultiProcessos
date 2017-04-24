/***

Input 

<seg>: delay de execucao em relacao a hora corrente
<arq_exec>: Nome do arquivo a ser executado 

Output

Atribuir um numero de job unico a tupla <arq_exec> 
Colocar na estrutura de dados compartilhado 
**/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct jobTable
{
	int job;
	char * arq_exec;
	char * data;

}typedef jobTableType;



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


int main(int argc, char *argv[])
{
	int job_anterior = -1;

	//Verifica o numero de parametros
	if (argc != 3){
		printf("Num de parametros invalido\n");
		exit(0);
	}


	const char * seg = argv[1];
	
	//Verifica se o numero de segundos eh um numero
	is_num(seg);



	int idfila = msgget(0x1223, 0x1B6);
	printf("%d\n", idfila);

	msgrcv(idfila, &job_anterior, sizeof(job_anterior)-sizeof(long), 0, 0);
   	printf("mensagem recebida = %d\n", job_anterior);





	return 0;
}

