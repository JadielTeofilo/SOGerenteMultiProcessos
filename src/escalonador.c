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
//realiza broadcast com informcoes do novo job para os gerenciadores
void informar_ger_exec_job(tipoTabela*dados_job){
    InfoMsgTorus * mensagem;

}

int calcular_idfila_envio(int lado, int meu_id, int * id_torus){
    switch (lado){
        case 0: 
            return(id_torus[meu_id*4]);
        //caso de fila para a direita  
        case 1:
            return(id_torus[(meu_id*4)+1]);
        //caso de fila para baixo
        case 2:
            return(id_torus[(meu_id*4)+2]);
        //caso de fila para a esquerda
        case 3:
            return(id_torus[(meu_id*4)+3]);
    }

}

void calcular_idfila_receber(int lado, int meu_id, int *id_torus){
    int col = meu_id%4;
    int lin = meu_id/4;
    switch(lado){
        case 0:
            lin = (lin-1)%4;
        case 1:
            col = (col+1)%4;
        case 2:
            lin = (lin+1)%4;
        case 3:
            col = (col-1)%4;
    }
}

//codigo do filho/gerenciador de execucao
void gerenciar_execucao(int meu_id, int * id_torus){
    InfoMsgTorus * mensagem;
    //no gerenciador 0    
    if(meu_id==0){
        int id_ida_escal=-1;
        int id_volta_escal=-1;

        //pegar id para comunicacao com escalonador no gerenciador 0
        if((id_ida_escal = msgget(0x1226, 0x1B6)) < 0){
            printf("Nenhuma fila encontrada no gerenciador 0 para comunicacao com escalonador\n");
            exit(1);
        }
        if((id_volta_escal = msgget(0x1227, 0x1B6)) < 0){
            printf("Nenhuma fila encontrada no gerenciador 0 para comunicacao com escalonador \n");
            exit(1);

        }

        msgrcv(id_ida_escal, &mensagem , sizeof(mensagem), 0, 0);
    }
    else{
        while(1){


        }
    }
}

void criar_filas_torus(int * id_torus){
    int key = 0x1228;
    int i;
    int idfila;
    for(i=0; i<64; i++){
        idfila = criar_fila(key);
        id_torus[i]=idfila;
        key++;
    }
}

void montar_torus(){
    int pid, i;
    int status;
    int id_torus[64];
    int meu_id = 0;

    //Criar as filas para comunicacao entre os gerentes
    criar_filas_torus(id_torus);

    //monta os 16 filhos que irao se comunicar entre eles
    for(i=0; i<16; i++){
        meu_id = i;
        if((pid = fork())<0){
            printf("erro no fork\n");
            exit(1);
        }
        //codigo do filho
        if(pid == 0){
            gerenciar_execucao(meu_id, id_torus);
            break;
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
    int idfila_escal_gerente0_ida = -1;
    int idfila_escal_gerente0_volta = -1;
    
    //Cria filas para comunicacao

    idfila_num_job = criar_fila(0x1223);
    idfila_estrutura = criar_fila(0x1224);
    idfila_shutdown = criar_fila(0x1225);
    idfila_escal_gerente0_ida = criar_fila(0x1226);
    idfila_escal_gerente0_volta = criar_fila(0x1227);

    //envia o proximo job para a fila de mensagens
    enviar_num_job(job_anterior, idfila_num_job);

    //Iniciar tabela de jobs
    tipoTabela * tabela_jobs;
    tipoTabela * dados_job;
    tabela_jobs = init_job_table();

    //cria os gerentes de execução segundo a topologia torus
    montar_torus();

    while(1){
        //Le um novo job inserido 
        tabela_jobs = atualiza_info_job(idfila_estrutura, tabela_jobs, idfila_num_job);

        //funcao que dorme ate chegar o momento de executar um job
        if(checar_horario_execucao_job(tabela_jobs)){

            //funcao chama os gerenciadores de execucao para executar o job
            dados_job = pop_job(tabela_jobs);
            informar_ger_exec_job(dados_job);
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