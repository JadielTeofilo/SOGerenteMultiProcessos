#include "escalonador.h"



void enviar_num_job(jobNumType job_anterior, int idfila){
    if(msgsnd(idfila, &job_anterior, sizeof(job_anterior), IPC_NOWAIT) < 0){
    //if(msgsnd(idfila, &job_anterior, sizeof(job_anterior), 0), 0) >= 0){
        printf("Erro no envio da mensagem p/ a fila de mensagem\n");
    }
}

tipoTabela * atualiza_info_job(int idfila, tipoTabela *tabela_jobs, int idfila_num_job){
    jobInfoType mensagem;
    jobInfoType info_job;
    char* c_time_string;
    jobNumType job_anterior;

    //Recebe o ultimo job
    if (msgrcv(idfila, &mensagem, sizeof(mensagem), 0, IPC_NOWAIT) < 0){
    //if (msgrcv(idfila, &mensagem, sizeof(mensagem)-sizeof(long), 0, 0) < 0){
        return tabela_jobs;
    }

    // Debug
    
    // Conveter para string a data
    // c_time_string = ctime(&mensagem.data);
    // printf("::job = %d\n", mensagem.job);
    // printf("::arquivo = %s\n", mensagem.arq_exec);
    // printf("::%s\n", c_time_string);

    info_job = mensagem;


    tabela_jobs = append_job_ordenado(info_job.job, info_job.data, 
                                        info_job.arq_exec, tabela_jobs);
    job_anterior.job_num = info_job.job+1;
    printf("mensagem enviada: %d\n", job_anterior.job_num);
    enviar_num_job(job_anterior, idfila_num_job);

    return tabela_jobs;
}


int checar_horario_execucao_job(tipoTabela * tabela_jobs){
    time_t current_time = time(NULL);
    if(tabela_jobs==NULL)
        return 0;
    if(current_time>=tabela_jobs->data){
        return 1;
    }
    return 0;
}
void imprimir_remanescentes(tipoTabela * tabela_jobs){
    tipoTabela * tabela_aux;
    //tem que acessar a lista de tabelas e percorrer ela
    
    //printf("%d %s \n",(*tabela_jobs).job_num, (*tabela_jobs).arq_exec );
    printf("\nprogramas que não serao executados:\n");
    printf("job     arquivo executavel\n");
    for(;tabela_jobs!=NULL; tabela_jobs = tabela_jobs->prox){

        tabela_aux = pop_job(tabela_jobs);
        printf("%d          %s\n", tabela_aux->job_num, tabela_aux->arq_exec);
    }
}
void executar_job(){}

void escalonar(){

    shutInfo shutdown;
    shutdown.desliga = 0;
	jobNumType job_anterior;
    job_anterior.job_num = 1;
	int idfila_num_job = -1;
    int idfila_estrutura = -1;
    int idfila_shutdown = -1;
    int id_mem_lista = -1;
    
    //Cria filas para comunicacao
    idfila_num_job = criar_fila(0x1223);
    idfila_estrutura = criar_fila(0x1224);
    idfila_shutdown = criar_fila(0x1225);

    //envia de inicio para a fila de jobs utilizados o -1
    enviar_num_job(job_anterior, idfila_num_job);
    //criar memoria compartilhada 
    //para que outros programas possam acessar o ponteiro da tabela
    if((id_mem_lista = shmget(0x1232, sizeof(long*), IPC_CREAT|0x1ff))<0){
        printf("erro na obtencao da memoria\n");
        exit(1);
    }
    //Iniciar tabela de jobs
    tipoTabela * tabela_jobs;
    tabela_jobs = init_job_table();

    while(1){
        // Pega um novo job do programa executa postergado 
        tabela_jobs = atualiza_info_job(idfila_estrutura, tabela_jobs, idfila_num_job);

        //funcao que dorme ateh chegar o momento de executar um job
        if(checar_horario_execucao_job(tabela_jobs)){

            //funcao chama os gerenciadores de execucao para executar o job
            executar_job();
        }

        //Verifica se tem mensagem do processo shutdown mandando desligar
        msgrcv(idfila_shutdown, &shutdown, sizeof(shutdown), 0, IPC_NOWAIT);

        if(shutdown.desliga==1){
            //printf("job = %d\n", tabela_jobs->job_num);
            //printf("arquivo = %s\n", tabela_jobs->arq_exec);
            imprimir_remanescentes(tabela_jobs);
            free_job_table(tabela_jobs);
            excluir_fila(idfila_num_job);
            excluir_fila(idfila_estrutura);
            excluir_fila(idfila_shutdown);

            shmctl(id_mem_lista, IPC_RMID, 0);
            break;
        }
    }
}




int main(int argc, char const *argv[])
{
	escalonar();

	return 0;
}