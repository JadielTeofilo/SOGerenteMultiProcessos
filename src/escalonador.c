#include "escalonador.h"

//Definicoes para os ids das filas
int idfila_num_job;
int idfila_estrutura;
int idfila_shutdown;
int idfila_escal_gerente0_ida;
int idfila_escal_gerente0_volta;

//definicao id memoria compartilhada
int id_shm;

//dados do job
tipoTabela * tabela_jobs;
tipoTabela * dados_job;

//id dos processos gerentes de execucao
int pid_filho[16];

//id das filas de comunicacao do torus
int id_torus[64];

//id para operacao com semáforos
struct sembuf operacao[2];
int idsem;


void enviar_num_job(jobNumType job_anterior, int idfila){
    if(msgsnd(idfila, &job_anterior, sizeof(job_anterior), IPC_NOWAIT) < 0){
    //if(msgsnd(idfila, &job_anterior, sizeof(job_anterior)-sizeof(long), 0, 0) >= 0){
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
    if(tabela_jobs==NULL){
        return 0;
    }
    //verifica se o horario atual eh maior que o horario que deveria ser executado
    //ou seja, se já passou do horario de execução
    if(current_time>=tabela_jobs->data){
        return 1;
    }
    return 0;
}

// imprimir dados dos jobs que nao foram executados
void imprimir_remanescentes(tipoTabela * tabela_jobs){
    tipoTabela * tabela_aux;
    char* c_time_string;

    //tem que acessar a lista de tabelas e percorrer ela
    printf("\nprogramas que não serao executados:\n");
    printf("job     arquivo executavel      data\n");
    for(;tabela_jobs!=NULL; tabela_jobs = tabela_jobs->prox){
        c_time_string = ctime(&tabela_jobs->data);
        tabela_aux = tabela_jobs;
        printf("%d          %s          %s", tabela_aux->job_num, tabela_aux->arq_exec, c_time_string);
    }
}
//realiza broadcast com informcoes do novo job para os gerenciadores
void informar_ger_exec_job(tipoTabela * dados_job){
    InfoMsgTorus mensagem;

    mensagem.type = 2;
    mensagem.id_mensagem = 1;
    mensagem.job = dados_job->job_num;
    strcpy(mensagem.arq_exec, dados_job->arq_exec);
    mensagem.data = dados_job->data;

    if(msgsnd(idfila_escal_gerente0_ida, &mensagem, sizeof(mensagem), IPC_NOWAIT)<0){
    //if(msgsnd(idfila_escal_gerente0_ida, &dados_job, sizeof(tipoTabela)-sizeof(long), IPC_NOWAIT)<0){
        printf("erro na hora de enviar dados para o \n");
        kill(getpid(), SIGTERM);
    }


}

int p_sem()
{
     operacao[0].sem_num = 0;
     operacao[0].sem_op = 0;
     operacao[0].sem_flg = 0;
     operacao[1].sem_num = 0;
     operacao[1].sem_op = 1;
     operacao[1].sem_flg = 0;
     if ( semop(idsem, operacao, 2) < 0)
       printf("erro no p=%d\n", errno);
}
int v_sem()
{
     operacao[0].sem_num = 0;
     operacao[0].sem_op = -1;
     operacao[0].sem_flg = 0;
     if ( semop(idsem, operacao, 1) < 0)
       printf("erro no p=%d\n", errno);
}

//calcula id da fila de envio para cada processo escalonador
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

//Retorna o id da fila para receber msg do 'lado' lado
int calcular_idfila_receber(int lado, int meu_id, int *id_torus){
    //calcular a propria posicao 
    int col = meu_id%4;
    int lin = meu_id/4;
    //calcula a posicao do visinho
    switch(lado){
        case 0:
            lin = (lin-1)%4;
            break;
        case 1:
            col = (col+1)%4;
            break;
        case 2:
            lin = (lin+1)%4;
            break;
        case 3:
            col = (col-1)%4;
            break;
    }
    //calcula a id do vizinho
    int id = col * 4 + lin;
    //chama funcao para pegar o id da fila de retorno
    switch(lado){
        case 0:
            return calcular_idfila_envio( 2, id, id_torus);
        case 1:
            return calcular_idfila_envio( 3, id, id_torus);
        case 2:
            return calcular_idfila_envio( 0, id, id_torus);
        case 3:
            return calcular_idfila_envio( 1, id, id_torus);
    }
}

void envia_msgs_vizinho(InfoMsgTorus mensagem, int meu_id, int * id_torus){
    int i;
    for(i=0; i<4; i++){
        int idfila = calcular_idfila_envio(i, meu_id, id_torus);

        if(msgsnd(idfila, &mensagem, sizeof(mensagem), IPC_NOWAIT)<0){
        //if(msgsnd(idfila_escal_gerente0_ida, &dados_job, sizeof(tipoTabela)-sizeof(long), IPC_NOWAIT)<0){
            printf("erro na hora de enviar dados para o \n");
            kill(getpid(), SIGTERM);
        }
    }
}


void recebe_mensagem(){}
//codigo do filho/gerenciador de execucao
void gerenciar_execucao(int meu_id, int * id_torus){
    InfoMsgTorus mensagem;
    int status;
    int pid;
    //no gerenciador 0    
    if(meu_id==0){
        int id_ida_escal=-1;
        int id_volta_escal=-1;

        //pegar id para comunicacao com escalonador no gerenciador 0
        if((id_ida_escal = msgget(0x1226, 0x1B6)) < 0){
            printf("Nenhuma fila encontrada no gerenciador 0 para comunicacao com escalonador\n");
            kill(getpid(), SIGTERM);
        }
        if((id_volta_escal = msgget(0x1227, 0x1B6)) < 0){
            printf("Nenhuma fila encontrada no gerenciador 0 para comunicacao com escalonador \n");
            kill(getpid(), SIGTERM);

        }
        //recebe primeira msg do escalonador
        msgrcv(id_ida_escal, &mensagem , sizeof(mensagem), 0, 0);

        // Envia mensagem para todos vizinhos 
        envia_msgs_vizinho(mensagem, meu_id, id_torus);
        
        if((pid = fork())<0){
            printf("erro na criacao de fork\n");
            kill(getpid(), SIGTERM);
        }
        if(pid == 0){
            execl(mensagem.arq_exec, mensagem.arq_exec, NULL);
        }

        wait(&status);


        mensagem.type = 3;
        if(msgsnd(id_volta_escal, &mensagem, sizeof(mensagem), IPC_NOWAIT)<0){
        //if(msgsnd(idfila_escal_gerente0_ida, &dados_job, sizeof(tipoTabela)-sizeof(long), IPC_NOWAIT)<0){
            printf("erro na hora de enviar dados para o \n");
            kill(getpid(), SIGTERM);
        }
    }
    else{
        while(1){

            recebe_mensagem();


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
    int meu_id = 0;
    int *psem;

    //Criar as filas para comunicacao entre os gerentes
    criar_filas_torus(id_torus);

 
    /* cria semaforo*/
    if ((idsem = semget(0x1223, 1, IPC_CREAT|0x1ff)) < 0)
    {
        printf("erro na criacao da fila\n");
        exit(1);
    }

    //monta os 16 filhos que irao se comunicar entre eles
    for(i=0; i<16; i++){
        meu_id = i;
        if((pid = fork())<0){
            printf("erro no fork\n");
            kill(getpid(), SIGTERM);
        }
        //codigo do filho
        if(pid == 0){
            gerenciar_execucao(meu_id, id_torus);
            break;
        }
        pid_filho[i]=pid;
    }
}

void fechar_filas_torus(){
    int i;
    for(i=0; i<64; i++){
        excluir_fila(id_torus[i]);
    }
}

void shutdown(){
    //soh entra no if se tiver recebido a mensagem do shutdown para desligar.
    //imprime as informações dos jobs que não foram executados
    imprimir_remanescentes(tabela_jobs);

    //libera todas as estruturas de dados utilizados
    free_job_table(tabela_jobs);
    excluir_fila(idfila_num_job);
    excluir_fila(idfila_estrutura);
    excluir_fila(idfila_shutdown);
    excluir_fila(idfila_escal_gerente0_ida);
    excluir_fila(idfila_escal_gerente0_volta);

    //espera o fechamento dos filhotes e fecha
    int status;
    int i;
    for (i = 0; i < 16; i++)
       kill(pid_filho[i], SIGKILL);

    while(wait(&status) != -1);

    shmctl(id_shm, IPC_RMID, 0);


    fechar_filas_torus();

    exit(1);

}

void escalonar(){
    signal(SIGTERM, shutdown);


    jobNumType job_anterior;
    job_anterior.job_num = 1;
    idfila_num_job = -1;
    idfila_estrutura = -1;
    idfila_shutdown = -1;
    idfila_escal_gerente0_ida = -1;
    idfila_escal_gerente0_volta = -1;
    
    //Cria filas para comunicacao
    idfila_num_job = criar_fila(0x1223);
    idfila_estrutura = criar_fila(0x1224);
    idfila_shutdown = criar_fila(0x1225);
    idfila_escal_gerente0_ida = criar_fila(0x1226);
    idfila_escal_gerente0_volta = criar_fila(0x1227);

    //Criar espaco em memoria com o pid do escalonador p/ shutdown
    id_shm = shmget(0x1323,sizeof(int), IPC_CREAT| 0x1B6);
    int *shm_pid = shmat(id_shm, 0, 0x1B6);
    *shm_pid = getpid();


    //envia o proximo job para a fila de mensagens
    enviar_num_job(job_anterior, idfila_num_job);

    //Iniciar tabela de jobs
    tabela_jobs = init_job_table();

    //cria os gerentes de execução segundo a topologia torus
    montar_torus();

    while(1){
        //Le um novo job inserido 
        tabela_jobs = atualiza_info_job(idfila_estrutura, tabela_jobs, idfila_num_job);

        //funcao que dorme ate chegar o momento de executar um job
        if(checar_horario_execucao_job(tabela_jobs)){
            //funcao chama os gerenciadores de execucao para executar o job


            informar_ger_exec_job(tabela_jobs);
            tabela_jobs = pop_job(tabela_jobs);
        }

        //Verifica se tem mensagem do processo shutdown mandando desligar
        msgrcv(idfila_shutdown, &shutdown, sizeof(shutdown), 0, IPC_NOWAIT);

    }
}




int main(int argc, char *argv[])
{

	escalonar();

	return 0;
}