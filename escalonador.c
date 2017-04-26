#include "escalonador.h"



void enviar_num_job(jobNumType job_anterior, int idfila){
    //msgsnd(idfila, &job_anterior, sizeof(job_anterior), 0);
   	msgsnd(idfila, &job_anterior, sizeof(job_anterior)-sizeof(long), 0);
}

void ler_estrutura_da_fila(int idfila){
    jobTableType mensagem;
    char* c_time_string;

    //Recebe o ultimo job
    //precisa corrigir isso, de resto acho q teoricamente funciona, tah dando overflow
    //if (msgrcv(idfila, &mensagem, sizeof(mensagem), 0, 0) < 0){
    if (msgrcv(idfila, &mensagem, sizeof(mensagem)-sizeof(long), 0, 0) < 0){
        printf("Nenhuma mensagem na fila\n");
        exit(1);
    }

    // printf("job = %d\n\n", mensagem.job);
    //printf("arquivo = %s\n", mensagem.arq_exec);
    // Conveter para string a data
    c_time_string = ctime(&mensagem.data);
    printf("::job = %d\n", mensagem.job);
    // printf("::arquivo = %s\n", mensagem.arq_exec);
    printf("::%s\n", c_time_string);

//return mensagem_ptr;
}


void escalonar(){
    struct jobTable mensagem_ptr;

	jobNumType job_anterior;
    job_anterior.job_num = 1;
	int idfila = -1;
    int idfila_estrutura = -1;

   	idfila = criar_fila(0x1223);
   	//envia de inicio para a fila de jobs utilizados o -1
   	enviar_num_job(job_anterior, idfila);

    idfila_estrutura = criar_fila(0x1224);

    ler_estrutura_da_fila(idfila_estrutura);

    //printf("job = %d\n", mensagem_ptr->job);
    //printf("arquivo = %s\n", mensagem_ptr->arq_exec);
    excluir_fila(idfila);
    excluir_fila(idfila_estrutura);

}




int main(int argc, char const *argv[])
{
	escalonar();

	return 0;
}