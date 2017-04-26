#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>



//Cria uma fila com a chave key
int criar_fila(int key);

//Exclui a fila de id idfila
void excluir_fila(int idfila);

