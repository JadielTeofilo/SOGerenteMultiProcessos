#include "fila_mensagem.h"


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

