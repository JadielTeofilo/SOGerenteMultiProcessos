#include "shutdown.h"



int main(){
	
	//Criar espaco em memoria com o pid do escalonador p/ shutdown
    int id_shm = shmget(0x1323,sizeof(int), 0x1B6);
    int *shm_pid = shmat(id_shm, 0, 0x1B6);
    kill(*shm_pid, SIGTERM);
	
	exit(0);

}