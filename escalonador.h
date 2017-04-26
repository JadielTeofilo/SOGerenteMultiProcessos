/**

Escalonador importacoes e definicoes

*/


#include "executa_postergado.h"
#include "fila_mensagem.h"

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>

