/***
Esse arquivo contem as definicoes de dados e inclusoes para 
o programa executa postergado 


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
#include <time.h>
#include <unistd.h>

struct jobTable
{
	int job;
	char * arq_exec;
	time_t data;

}typedef jobTableType;

struct jobNum
{
	int job_num;

}typedef jobNumType;



