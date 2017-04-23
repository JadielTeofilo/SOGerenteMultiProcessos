/***

Input 

<seg>: delay de execucao em relacao a hora corrente
<arq_exec>: Nome do arquivo a ser executado 

Output

Atribuir um numero de job unico a tupla <arq_exec> 
Colocar na estrutura de dados compartilhado 
**/

#include <executa_postergado>

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
	//Verifica o numero de parametros
	if (argc != 3){
		printf("Num de parametros invalido\n");
		exit(0);
	}


	const char * seg = argv[1];
	
	//Verifica se o numero de segundos eh um numero
	is_num(seg);

	





	return 0;
}

