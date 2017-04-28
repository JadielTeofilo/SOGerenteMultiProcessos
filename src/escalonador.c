#include "escalonador.h"



void enviar_num_job(jobNumType job_anterior, int idfila){
    if(msgsnd(idfila, &job_anterior, sizeof(job_anterior), 0) < 0){
    //if(msgsnd(idfila, &job_anterior, sizeof(job_anterior), 0), 0) >= 0){
        printf("Erro no envio da mensagem p/ a fila de mensagem\n");
    }
}

jobInfoType receber_info_job(int idfila){
    jobInfoType mensagem;
    jobInfoType mensagem_aux;
    char* c_time_string;

    //Recebe o ultimo job
    if (msgrcv(idfila, &mensagem, sizeof(mensagem), 0, 0) < 0){
    //if (msgrcv(idfila, &mensagem, sizeof(mensagem)-sizeof(long), 0, 0) < 0){
        printf("Nenhuma mensagem na fila\n");
        exit(1);
    }

    // Debug
    
    // Conveter para string a data
    // c_time_string = ctime(&mensagem.data);
    // printf("::job = %d\n", mensagem.job);
    // printf("::arquivo = %s\n", mensagem.arq_exec);
    // printf("::%s\n", c_time_string);

    mensagem_aux = mensagem;
    return mensagem_aux;
}


void checar_horario_execucao_job(){}

void executar_job(){}

void escalonar(){

	jobNumType job_anterior;
    jobInfoType info_job;
    job_anterior.job_num = 1;
	int idfila_num_job = -1;
    int idfila_estrutura = -1;


    //Iniciar tabela de jobs
    tipoTabela * tabela_jobs = init_job_table();

    //Cria filas para comunicacao
   	idfila_num_job = criar_fila(0x1223);
    idfila_estrutura = criar_fila(0x1224);

    //envia de inicio para a fila de jobs utilizados o -1
    enviar_num_job(job_anterior, idfila_num_job);

    /** TODO provavelmente vai ter que ser colocado uma 
        thread essa parte de esperar por novo job e colocar na tabela
    */
    // Pega um novo job do programa executa postergado 
    info_job = receber_info_job(idfila_estrutura);

    //Coloca o novo job na tabela de forma ordenada por data
    tabela_jobs = append_job_ordenado(info_job.job, info_job.data, 
                                        info_job.arq_exec, tabela_jobs);

    //funcao que dorme ateh chegar o momento de executar um job
    checar_horario_execucao_job();

    //funcao chama os gerenciadores de execucao para executar o job
    executar_job();



    //printf("job = %d\n", mensagem_ptr->job);
    //printf("arquivo = %s\n", mensagem_ptr->arq_exec);
    tabela_jobs = delete_job_table(tabela_jobs);
    excluir_fila(idfila_num_job);
    excluir_fila(idfila_estrutura);

}




int main(int argc, char const *argv[])
{
	escalonar();

	return 0;
}