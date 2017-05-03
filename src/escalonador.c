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
    jobNumType job_anterior;

    //Recebe o ultimo job
    if (msgrcv(idfila, &mensagem, sizeof(mensagem), 0, IPC_NOWAIT) < 0){
    //if (msgrcv(idfila, &mensagem, sizeof(mensagem)-sizeof(long), 0, 0) < 0){
        return tabela_jobs;
    }
    info_job = mensagem;


    tabela_jobs = append_job_ordenado(info_job.job, info_job.data, 
                                        info_job.arq_exec, tabela_jobs);

    //envia o ultimo job atualizado
    printf("mensagem enviada: %d\n", job_anterior.job_num);
    //define o identificador unico do job como o anterior + 1
    job_anterior.job_num = info_job.job+1;
    enviar_num_job(job_anterior, idfila_num_job);

    return tabela_jobs;
}


int checar_horario_execucao_job(tipoTabela * tabela_jobs){
    time_t current_time = time(NULL);
    //se a tabela for vazia, não tem programa para executar
    if(tabela_jobs==NULL)
        return 0;
    //verifica se o horario atual eh maior que o horario que deveria ser executado
    //ou seja, se já passou do horario de execução
    if(current_time>=tabela_jobs->data){
        return 1;
    }
    return 0;
}

void imprimir_remanescentes(tipoTabela * tabela_jobs){
    tipoTabela * tabela_aux;
    char* c_time_string;

    //tem que acessar a lista de tabelas e percorrer ela
    printf("\nprogramas que não serao executados:\n");
    printf("job     arquivo executavel      data\n");
    for(;tabela_jobs!=NULL; tabela_jobs = tabela_jobs->prox){
        c_time_string = ctime(&tabela_jobs->data);
        tabela_aux = pop_job(tabela_jobs);
        printf("%d          %s          %s", tabela_aux->job_num, tabela_aux->arq_exec, c_time_string);
    }
}

void executar_job(){}

void montar_torus(){
    int pid;
    int status;
    //monta os 16 filhos que irao se comunicar entre eles
    for(i=0; i<16; i++){
        if((pid = fork())<0){
            printf("erro no fork\n");
            exit(1);
        }
        //codigo do filho
        if(pid == 0){
            printf("sou o processo filho\n");
            if(execl("gerente_execucao", "gerente_execucao",..., (char *) 0)<0){
                printf("erro no execl\n");
            }
        }
    }
}

void escalonar(){

    shutInfo shutdown;
    shutdown.desliga = 0;

	jobNumType job_anterior;
    job_anterior.job_num = 1;
	int idfila_num_job = -1;
    int idfila_estrutura = -1;
    int idfila_shutdown = -1;
    
    //Cria filas para comunicacao
    idfila_num_job = criar_fila(0x1223);
    idfila_estrutura = criar_fila(0x1224);
    idfila_shutdown = criar_fila(0x1225);

    //envia o proximo job para a fila de mensagens
    enviar_num_job(job_anterior, idfila_num_job);

    //Iniciar tabela de jobs
    tipoTabela * tabela_jobs;
    tabela_jobs = init_job_table();

    //cria os gerentes de execução segundo a topologia torus
    montar_torus();

    while(1){
        //Le um novo job inserido 
        tabela_jobs = atualiza_info_job(idfila_estrutura, tabela_jobs, idfila_num_job);

        //funcao que dorme ate chegar o momento de executar um job
        if(checar_horario_execucao_job(tabela_jobs)){

            //funcao chama os gerenciadores de execucao para executar o job
            executar_job();
        }

        //Verifica se tem mensagem do processo shutdown mandando desligar
        msgrcv(idfila_shutdown, &shutdown, sizeof(shutdown), 0, IPC_NOWAIT);

        if(shutdown.desliga==1){
        //soh entra no if se tiver recebido a mensagem do shutdown para desligar.
            //imprime as informações dos jobs que não foram executados
            imprimir_remanescentes(tabela_jobs);

            //libera todas as estruturas de dados utilizados
            free_job_table(tabela_jobs);
            excluir_fila(idfila_num_job);
            excluir_fila(idfila_estrutura);
            excluir_fila(idfila_shutdown);
            break;
        }
    }
}




int main(int argc, char const *argv[])
{
	escalonar();

	return 0;
}