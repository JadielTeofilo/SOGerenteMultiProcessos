#include "escalonador.h"


//TODO \
colocar essas definicoes em outro arquivo e usar elas\
tbm no executa postergado

//TODO \
Criar uma definicao para lados esquerdo direito cima e baixo

#define key_fila_1 0x1223
#define key_fila_2 0x1224
#define key_fila_3 0x1225
#define key_fila_4 0x1226
#define key_fila_5 0x1226
#define key_fila_6 0x1228   
#define key_fila_7 0x1229
#define key_fila_8 0x1230
#define key_fila_9 0x1231

#define key_mem_compart_1 0x1323
#define key_mem_compart_2 0x1324
#define key_mem_compart_3 0x1325
#define key_mem_compart_4 0x1326
#define key_mem_compart_5 0x1327
#define key_mem_compart_6 0x1328
#define key_mem_compart_7 0x1329
#define key_mem_compart_8 0x1330
#define key_mem_compart_9 0x1331

#define key_sem_1 0x1423
#define key_sem_2 0x1424
#define key_sem_3 0x1425
#define key_sem_4 0x1426
#define key_sem_5 0x1427
#define key_sem_6 0x1428
#define key_sem_7 0x1429
#define key_sem_8 0x1430
#define key_sem_9 0x1431

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
int id_torus_fila[64];
int id_torus_sem[16];

//id para operacao com semáforos
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
    if (msgrcv(idfila, &mensagem, sizeof(mensagem), 1, IPC_NOWAIT) < 0){
    //if (msgrcv(idfila, &mensagem, sizeof(mensagem)-sizeof(long), 1, IPC_NOWAIT) < 0){
        return tabela_jobs;
    }
    info_job = mensagem;


    tabela_jobs = append_job_ordenado(info_job.job, info_job.data, 
                                        info_job.arq_exec, tabela_jobs);

    //envia o ultimo job atualizado
    printf("mensagem enviada: %d\n", job_anterior.job_num);
    //define o identificador unico do job como o anterior + 1
    job_anterior.type = 1;
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
    printf("enviou\n");
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

//calcula id da fila de envio para cada processo escalonador
int calcular_idfila_envio(int lado, int meu_id){
    switch (lado){
        case 0: 
            return(meu_id*4);
        //caso de fila para a direita  
        case 1:
            return((meu_id*4)+1);
        //caso de fila para baixo
        case 2:
            return((meu_id*4)+2);
        //caso de fila para a esquerda
        case 3:
            return((meu_id*4)+3);
    }
}

//Retorna o id do vizinho do 'lado' lado
int calcular_id_vizinho(int lado, int meu_id){
    //calcular a propria posicao 
    int col = meu_id%4;
    int lin = meu_id/4;
    //calcula a posicao do vizinho
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
    return col * 4 + lin;
}

//Retorna o id da fila para receber msg do 'lado' lado
int calcular_idfila_receber(int lado, int meu_id){
    //calcula a id do vizinho
    int id = calcular_id_vizinho(lado, meu_id);

    //chama funcao para pegar o id da fila de retorno
    switch(lado){
        case 0:
            return calcular_idfila_envio( 2, id);
        case 1:
            return calcular_idfila_envio( 3, id);
        case 2:
            return calcular_idfila_envio( 0, id);
        case 3:
            return calcular_idfila_envio( 1, id);
    }
}

//Envia a "mensagem" para os quatro vizinhos
void envia_msgs_vizinho(InfoMsgTorus mensagem, int meu_id, int * id_torus_fila, int * id_torus_sem){
    int i;
    for(i=0; i<4; i++){
        int idfila = id_torus_fila[calcular_idfila_envio(i, meu_id)];

        if(msgsnd(idfila, &mensagem, sizeof(mensagem), IPC_NOWAIT)<0){
        //if(msgsnd(idfila_escal_gerente0_ida, &dados_job, sizeof(tipoTabela)-sizeof(long), IPC_NOWAIT)<0){
            printf("erro na hora de enviar dados para o \n");
            kill(getpid(), SIGTERM);
        }
        printf("ak %d \n", calcular_idfila_envio(i, meu_id));
        //desbloqueia o vizinho para receber a msg
        v_sem(id_torus_sem[calcular_id_vizinho(i, meu_id)]);
    }
}

//Recebe atual posicao e devolve a saida para chegar no gerente 0
int roteador(int meu_id){

    //calcula a saida 
    switch(meu_id%4){
        //Caso sejam da primeira coluna o destino eh para cima
        case 0:
            return 1;
        //Caso contrario o destino eh para o lado esquerdo
        default:
            return 0;
    }
}

// Aguarda o recebimento de mensagens vindas dos 4 vizinhos e depois envia 1 mensagem
InfoMsgTorus trata_broadcast(int * id_torus_fila, int meu_id, int * id_torus_sem){
    InfoMsgTorus mensagem;
    int i;

    //aguarda receber 4 mensagens
    for(i=0; i<4; i++){
        // printf("lala %d\n", meu_id);
        // aguarda o sinal de que recebeu uma mensagem
        p_sem(id_torus_sem[meu_id]);
        printf("passou\n");
        //Pega entao a mensagem que recebeu de algum dos lados
        for (int j = 0; j < 4; ++j)
        {
            int idfila = id_torus_fila[calcular_idfila_receber(j, meu_id)];
            if(msgrcv(idfila, &mensagem , sizeof(mensagem), 0, IPC_NOWAIT) > 0){
                // ele sai caso ache uma msg
                break;
            }
        } 

    }
    // envia a mensagem para os quatro lados
    envia_msgs_vizinho(mensagem, meu_id, id_torus_fila, id_torus_sem);

    return mensagem;
}

//codigo do filho/gerenciador de execucao
void gerenciar_execucao(int meu_id, int * id_torus_fila, int * id_torus_sem){
    InfoMsgTorus mensagem;
    int status;
    int pid;

    //no gerenciador 0    
    if(meu_id==0){
        int id_ida_escal=-1;
        int id_volta_escal=-1;

        //pegar id para comunicacao com escalonador no gerenciador 0
        if((id_ida_escal = msgget(key_fila_4, 0x1B6)) < 0){
            printf("Nenhuma fila encontrada no gerenciador 0 para comunicacao com escalonador\n");
            kill(getpid(), SIGTERM);
        }
        if((id_volta_escal = msgget(key_fila_5, 0x1B6)) < 0){
            printf("Nenhuma fila encontrada no gerenciador 0 para comunicacao com escalonador \n");
            kill(getpid(), SIGTERM);

        }
        //recebe primeira msg do escalonador
        msgrcv(id_ida_escal, &mensagem , sizeof(mensagem), 0, 0);
        printf("recebeu\n");
        // Envia mensagem para todos vizinhos 
        envia_msgs_vizinho(mensagem, meu_id, id_torus_fila, id_torus_sem);
        printf("oi\n");

        //Roda o programa solicitado
        if((pid = fork())<0){
            printf("erro na criacao de fork\n");
            kill(getpid(), SIGTERM);
        }
        if(pid == 0){
            if(execl(mensagem.arq_exec, mensagem.arq_exec, NULL)<0){
                printf("erro na execução do arquivo");
            }
        }

        //Aguarda o programa encerrar
        wait(&status);

        //  TODO Avisa o escalonador que acabou quando recebe \
        mensagem de todos os outros dizendo que acabaram
        int num_finalizados = 1;
        while(1){

            // mensagem.type = 3;
            // if(msgsnd(id_volta_escal, &mensagem, sizeof(mensagem), IPC_NOWAIT)<0){
            // //if(msgsnd(idfila_escal_gerente0_ida, &dados_job, sizeof(tipoTabela)-sizeof(long), IPC_NOWAIT)<0){
            //     printf("erro na hora de enviar dados para o \n");
            //     kill(getpid(), SIGTERM);
            // }
        }
    }
    else{
        while(1){
            //Aguarda o recebimento de mensagens vindas dos 4 vizinhos e depois envia 1 mensagem
            mensagem = trata_broadcast(id_torus_fila, meu_id, id_torus_sem);
            if((pid = fork())<0){
                printf("erro na criacao de fork\n");
                kill(getpid(), SIGTERM);
            }
            printf("::%s\n",mensagem.arq_exec );
            if(pid == 0){
                if(execl(mensagem.arq_exec, mensagem.arq_exec, NULL)<0){
                    printf("erro na execução do arquivo");
                }
            }
            //Aguarda o programa encerrar
            wait(&status);

            // /TODO 
            // Fazer a parte que envia mensagem de que terminou para o gerente 0
            // //Recebe atual posicao e devolve a saida para chegar no gerente 0
            // int id_envio = calcular_idfila_envio(roteador(meu_id),meu_id);
            //Enviar mensagem termino para o proximo
            

        }
    }
}

void criar_filasNsem_torus(int * id_torus_fila, int *id_torus_fila_sem){
    int key_fila = key_fila_6;
    int key_sem = key_sem_1;
    int i;
    int idfila;
    for(i=0; i<64; i++){
        //Criar fila
        idfila = criar_fila(key_fila);

        if(i < 16){
            //Criar semaforo
            idsem = criar_semaforo(key_sem);
            //salvar o id sem
            id_torus_sem[i] = idsem;
            //atualizar a key sem
            key_sem++; 
        }

        //salvar o id fila
        id_torus_fila[i] = idfila;

        //atualizar a key fila
        key_fila++;
    }
}

void montar_torus(){
    int pid, i;
    int meu_id = 0;
    int *psem;

    //Criar as filas para comunicacao entre os gerentes
    criar_filasNsem_torus(id_torus_fila, id_torus_sem);

 
    /* cria semaforo*/


    //monta os 16 filhos que irao se comunicar entre eles
    for(i=0; i<16; i++){
        meu_id = i;
        if((pid = fork())<0){
            printf("erro no fork\n");
            kill(getpid(), SIGTERM);
        }
        //codigo do filho
        if(pid == 0){
            gerenciar_execucao(meu_id, id_torus_fila, id_torus_sem);
            break;
        }
        pid_filho[i]=pid;
    }
}

//Exclui as filas e os semaforos utilizados
void fechar_filasNsems_torus(){
    int i;
    for(i=0; i<64; i++){
        excluir_fila(id_torus_fila[i]);
        if(i<16){
           excluir_semaforo(id_torus_sem[i]);
        }
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



    // Matar os gerentes
    for (int i = 0; i < 16; i++)
       kill(pid_filho[i], SIGKILL);

    int status;
    //aguardar a morte dos gerentes
    while(wait(&status) != -1);

    //liberar a memoria compartilhada
    shmctl(id_shm, IPC_RMID, 0);

    printf("Desligando o Programa...\n");
    //Exclui as filas e os semaforos utilizados
    fechar_filasNsems_torus();

    exit(1);

}

void escalonar(){
    signal(SIGTERM, shutdown);


    jobNumType job_anterior;
    job_anterior.type = 1;
    job_anterior.job_num = 1;
    idfila_num_job = -1;
    idfila_estrutura = -1;
    idfila_shutdown = -1;
    idfila_escal_gerente0_ida = -1;
    idfila_escal_gerente0_volta = -1;
    
    //Cria filas para comunicacao
    idfila_num_job = criar_fila(key_fila_1);
    idfila_estrutura = criar_fila(key_fila_2);
    idfila_shutdown = criar_fila(key_fila_3);
    idfila_escal_gerente0_ida = criar_fila(key_fila_4);
    idfila_escal_gerente0_volta = criar_fila(key_fila_5);

    //Criar espaco em memoria com o pid do escalonador p/ shutdown
    id_shm = shmget(key_mem_compart_1,sizeof(int), IPC_CREAT| 0x1B6);
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