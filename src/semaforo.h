#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <signal.h>
#include <sys/sem.h>



//Cria uma semaforo com a chave key
int criar_semaforo(int key);

//Pegar id de um semaforo
int criar_semaforo(int key);

//Exclui a semaforo de id idfila
void excluir_semaforo(int idfila);

//Realiza a operacao up de dijkstra, aumenta um no semaforo
int v_sem(int sem);

//Realiza a operacao down de dijkstra, bloqueando caso zero \
e subtraindo caso um no semaforo contrario 
int p_sem(int sem);

