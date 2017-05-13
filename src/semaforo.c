#include "semaforo.h"

struct sembuf operacao[2];



//Cria uma semaforo com a chave key
int criar_semaforo(int key){
	int idsem;
	if ((idsem = semget(key, 1, IPC_CREAT|0x1ff)) < 0)
        {
            printf("erro na criacao da fila\n");
            kill(getpid(), SIGTERM);
        }
   	return idsem;
}

//Exclui a semaforo de id idfila
void excluir_semaforo(int idfila){
	semctl(idfila, 1, IPC_RMID, 0);

}

//Realiza a operacao up de dijkstra, aumenta um no semaforo
int v_sem(int sem)
{
    operacao[0].sem_num = 0;
    operacao[0].sem_op = 1;
    operacao[0].sem_flg = 0;
    if (semop(sem, operacao, 1) < 0)
    	printf("erro no p=%d\n", errno);
}

//Realiza a operacao down de dijkstra, bloqueando caso zero \
e subtraindo caso um no semaforo contrario 
int p_sem(int sem)
{
     operacao[0].sem_num = 0;
     operacao[0].sem_op = -1;
     operacao[0].sem_flg = 0;
     if ( semop(sem, operacao, 1) < 0)
       printf("erro no p=%d\n", errno);
}