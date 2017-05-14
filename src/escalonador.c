#include "escalonador.h"

//TODO \
Tem funcoes que estao recebendo variaveis globais por parametro\
ajeita ai shindi

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

#define key_sem_volta_1 0x1523
#define key_sem_volta_2 0x1524
#define key_sem_volta_3 0x1525
#define key_sem_volta_4 0x1526
#define key_sem_volta_5 0x1527
#define key_sem_volta_6 0x1528
#define key_sem_volta_7 0x1529
#define key_sem_volta_8 0x1530
#define key_sem_volta_9 0x1531

int flag_shutdown = 0;

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
int id_torus_sem_volta[16];


//Exclui as filas e os semaforos utilizados
void fechar_filasNsems_torus(){
    int i;
    for(i=0; i<64; i++){
        excluir_fila(id_torus_fila[i]);
        if(i<16){
           excluir_semaforo(id_torus_sem[i]);
           excluir_semaforo(id_torus_sem_volta[i]);
        }
    }
}

void libera_mem(){
    //libera todas as estruturas de dados utilizados
    free_job_table(tabela_jobs);
    excluir_fila(idfila_num_job);
    excluir_fila(idfila_estrutura);
    excluir_fila(idfila_shutdown);
    excluir_fila(idfila_escal_gerente0_ida);
    excluir_fila(idfila_escal_gerente0_volta);

    fechar_filasNsems_torus();

    // Matar os gerentes
    for (int i = 0; i < 16; i++)
       kill(pid_filho[i], SIGKILL);

    int status;
    //aguardar a morte dos gerentes
    while(wait(&status) != -1);

    //liberar a memoria compartilhada
    shmctl(id_shm, IPC_RMID, 0);

    

    exit(1);
}


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

//realiza passar mensagem com dados para o gerente zero 
void informar_ger_exec_zero(tipoTabela * dados_job){
    InfoMsgTorus mensagem;
    mensagem.type = 2;
    mensagem.id_mensagem = 1;
    mensagem.job = dados_job->job_num;
    strcpy(mensagem.arq_exec, dados_job->arq_exec);
    mensagem.data = dados_job->data;

    //mandar mensagem para o zero
    if(msgsnd(idfila_escal_gerente0_ida, &mensagem, sizeof(mensagem), IPC_NOWAIT)<0){
    //if(msgsnd(idfila_escal_gerente0_ida, &dados_job, sizeof(tipoTabela)-sizeof(long), IPC_NOWAIT)<0){
        printf("erro na hora de enviar dados para o gerente 0\n");
        libera_mem();
    }
}

//calcula id da fila de envio para cada processo escalonador
int calcular_index_fila_envio(int lado, int meu_id){
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
            //caso menor que zero pegar o equivalente positivo
            (lin < 0)?lin += 4 : lin;
            break;
        case 1:
            col = (col+1)%4;
            (col < 0)?col += 4 : col;
            break;
        case 2:
            lin = (lin+1)%4;
            (lin < 0)?lin += 4 : lin;
            break;
        case 3:
            col = (col-1)%4;
            //caso menor que zero pegar o equivalente positivo
            (col < 0)?col += 4 : col;
            break;
    }
    //calcula a id do vizinho
    return lin * 4 + col;
}

//Retorna o id da fila para receber msg do 'lado' lado
int calcular_idfila_receber(int lado, int meu_id){
    //calcula a id do vizinho
    int id = calcular_id_vizinho(lado, meu_id);

    //chama funcao para pegar o id da fila de retorno
    switch(lado){
        case 0:
            return calcular_index_fila_envio( 2, id);
        case 1:
            return calcular_index_fila_envio( 3, id);
        case 2:
            return calcular_index_fila_envio( 0, id);
        case 3:
            return calcular_index_fila_envio( 1, id);
    }
}

//Envia a "mensagem" para os quatro vizinhos
void envia_msgs_vizinho(InfoMsgTorus mensagem, int meu_id, int * id_torus_fila, int * id_torus_sem){
    int i;

    for(i=0; i<4; i++){
        int idfila = id_torus_fila[calcular_index_fila_envio(i, meu_id)];

        if(msgsnd(idfila, &mensagem, sizeof(mensagem), IPC_NOWAIT)<0){
        //if(msgsnd(idfila_escal_gerente0_ida, &dados_job, sizeof(tipoTabela)-sizeof(long), IPC_NOWAIT)<0){
            printf("erro na hora de enviar dados para os vizinhos errno: %d\n",errno);
            libera_mem();
            //return;
        }
        // printf("ak %d \n", calcular_index_fila_envio(i, meu_id));
        //desbloqueia o vizinho para receber a msg
        // printf("desbloqueia o %d\n",calcular_id_vizinho(i, meu_id));
        v_sem(id_torus_sem[calcular_id_vizinho(i, meu_id)]);
    }
}

//Recebe atual posicao e devolve a saida para chegar no gerente 0
int roteador(int meu_id){

    //Caso sejam da primeira coluna o destino eh para cima
    //Caso contrario o destino eh para o lado esquerdo
    return (meu_id%4)? 3 : 0;
}

//faz loop para receber e enviar msg de fim de execucao
void tratar_msg_fim_exec(int meu_id){
    InfoFlgTorus msg_flag;
    int id_envio = id_torus_fila[calcular_index_fila_envio(roteador(meu_id),meu_id)];
    int ainda_n_reenviei = 1;
    int id_envio_sem = calcular_id_vizinho(roteador(meu_id),meu_id);

    // aguarda receber mensagens de fim de execucao \
    podem ser 15 - meu_id para a primeira fileira ou meu_id/4 - 3 \
    para o resto
    int numRep = (!(meu_id % 4))?15 - meu_id: abs((4 - meu_id%4) - 1);

    for(int i=0; i < numRep; ++i){
        // aguarda o sinal de que recebeu uma mensagem
        p_sem(id_torus_sem_volta[meu_id]);
        // printf("eu %d, nao estou esperando\n", meu_id);
        //Pega entao as mensagem que recebeu e reenvia
        for (int j = 1; j < 3; ++j)
        {
            int idfila = id_torus_fila[calcular_idfila_receber(j, meu_id)];
            //Recebe a msg
            if(msgrcv(idfila, &msg_flag , sizeof(msg_flag), 4, IPC_NOWAIT) > 0){
                // envia a msg_flag para o proximo nodo
                if(msgsnd(id_envio, &msg_flag, sizeof(msg_flag), IPC_NOWAIT)<0){
                    printf("erro na hora de enviar msg fim exec\n");
                    libera_mem();
                }
                v_sem(id_torus_sem_volta[id_envio_sem]);
                // ele sai caso ache uma msg
                break;
            }   
                
        } 
    }
}

// Aguarda o recebimento de mensagens vindas dos 4 vizinhos e depois envia 1 mensagem
InfoMsgTorus trata_broadcast(int * id_torus_fila, int meu_id, int * id_torus_sem){
    InfoMsgTorus mensagem;
    int i;
    int ainda_n_enviei = 1;

    //aguarda receber 4 mensagens
    for(i=0; i<4; i++){
        // printf("lala %d\n", meu_id);
        // aguarda o sinal de que recebeu uma mensagem
        p_sem(id_torus_sem[meu_id]);
        //Pega entao a mensagem que recebeu de algum dos lados
        for (int j = 0; j < 4; ++j)
        {
            int idfila = id_torus_fila[calcular_idfila_receber(j, meu_id)];
            if(msgrcv(idfila, &mensagem , sizeof(mensagem), 2, IPC_NOWAIT) > 0){
                // envia a mensagem para os quatro lados
                if(ainda_n_enviei){
                    envia_msgs_vizinho(mensagem, meu_id, id_torus_fila, id_torus_sem);
                    ainda_n_enviei = 0;
                }
                // ele sai caso ache uma msg
                break;
            }
        } 
    }

    InfoMsgTorus mensagem_aux = mensagem;

    return mensagem_aux;
}

//codigo do filho/gerenciador de execucao
void gerenciar_execucao(int meu_id, int * id_torus_fila, int * id_torus_sem){
    InfoMsgTorus mensagem;
    int status;
    int pid;
    while(1){
        //no gerenciador 0    
        if(meu_id==0){
            int id_ida_escal=-1;
            int id_volta_escal=-1;

            //pegar id para comunicacao com escalonador no gerenciador 0
            if((id_ida_escal = msgget(key_fila_4, 0x1B6)) < 0){
                printf("Nenhuma fila encontrada no gerenciador 0 para comunicacao com escalonador\n");
                libera_mem();
            }
            if((id_volta_escal = msgget(key_fila_5, 0x1B6)) < 0){
                printf("Nenhuma fila encontrada no gerenciador 0 para comunicacao com escalonador \n");
                libera_mem();

            }
            while(1){
                //recebe primeira msg do escalonador
                if(msgrcv(id_ida_escal, &mensagem , sizeof(mensagem), 2, 0)<0){
                    printf("erro na recepção de mensagem do escalonador\n");
                    libera_mem();
                }
                // printf("recebeu\n");
                // Envia mensagem para todos vizinhos 
                envia_msgs_vizinho(mensagem, meu_id, id_torus_fila, id_torus_sem);
                // printf("oi\n");

                //Roda o programa solicitado
                if((pid = fork())<0){
                    printf("erro na criacao de fork\n");
                    libera_mem();
                }
                if(pid == 0){
                    //printf("%s\n", mensagem.arq_exec);
                    if(execl(mensagem.arq_exec, mensagem.arq_exec, NULL)<0){
                        printf("erro no execl do %d\n", meu_id);
                        libera_mem();
                    }
                }

                //Aguarda o programa encerrar
                wait(&status);
                
                InfoFlgTorus msg_flag;


                int num_finalizados = 0;
                //laco para receber todos os avisos de termino
                while(1){
                    p_sem(id_torus_sem_volta[meu_id]);
                    for (int j = 1; j < 3; ++j)
                    {
                        int idfila = id_torus_fila[calcular_idfila_receber(j, meu_id)];
                        //Recebe a msg
                        if(msgrcv(idfila, &msg_flag , sizeof(msg_flag), 4, IPC_NOWAIT) > 0){
                            //incrementa num finalizados
                            num_finalizados++;
                            // ele sai caso ache uma msg
                            break;
                        }
                    }
                    if(num_finalizados == 15){
                        break;
                    }
                }
                msgsnd(id_volta_escal, &msg_flag , sizeof(msg_flag), 0);
            }
        }
        else{
            //Aguarda o recebimento de mensagens vindas dos 4 vizinhos e depois envia 1 mensagem
            mensagem = trata_broadcast(id_torus_fila, meu_id, id_torus_sem);
            //Roda o arqexec
            if((pid = fork())<0){
                printf("erro na criacao de fork\n");
                libera_mem();
            }
            if(pid == 0){
                if(execl(mensagem.arq_exec, mensagem.arq_exec, NULL)<0){
                    printf("erro na execução do arquivo");
                }
            }
            //Aguarda o programa encerrar
            wait(&status);

            //Mensagem para avisar que encerrou a execucao
            InfoFlgTorus msg_flag;
            msg_flag.type = 4;
            msg_flag.flag = 1;

            int id_envio = id_torus_fila[calcular_index_fila_envio(roteador(meu_id),meu_id)];
            //Enviar mensagem termino para o proximo nodo
            if(msgsnd(id_envio, &msg_flag, sizeof(msg_flag), IPC_NOWAIT)<0){
                printf("erro na hora de enviar msg fim exec\n");
                libera_mem();
            }
            //avisa o destinatario
            id_envio = calcular_id_vizinho(roteador(meu_id),meu_id);
            v_sem(id_torus_sem_volta[id_envio]);
            
            //faz loop para receber e reenviar msg de fim de execucao
            tratar_msg_fim_exec(meu_id);
        }
    }
}

void criar_filasNsem_torus(int * id_torus_fila, int *id_torus_fila_sem){
    int key_fila = key_fila_6;
    int key_sem = key_sem_1;
    int key_sem_volta = key_sem_volta_1;
    int i;
    int idfila;
    int idsem;
    int idsem_volta;


    for(i=0; i<64; i++){
        //Criar fila
        idfila = criar_fila(key_fila);

        if(i < 16){
            //Criar semaforo
            idsem = criar_semaforo(key_sem);
            idsem_volta = criar_semaforo(key_sem_volta);
            //salvar o id sem
            id_torus_sem[i] = idsem;
            id_torus_sem_volta[i] = idsem_volta;
            //atualizar a key sem
            key_sem++; 
            key_sem_volta++; 
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
            libera_mem();
        }
        //codigo do filho
        if(pid == 0){
            gerenciar_execucao(meu_id, id_torus_fila, id_torus_sem);
            break;
        }
        pid_filho[i]=pid;
    }
}

void shutdown(){
    //soh entra no if se tiver recebido a mensagem do shutdown para desligar.
    //imprime as informações dos jobs que não foram executados
    imprimir_remanescentes(tabela_jobs);

    printf("Desligando o Programa...\n");
    //Exclui as filas e os semaforos utilizados

    libera_mem();

}

void escalonar(){
    signal(SIGTERM, shutdown);
    InfoFlgTorus msg_flag;

    //para a contagem do turnaround
    clock_t inicio;
    clock_t fim;
    float turnaround;

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
            inicio = clock();
            informar_ger_exec_zero(tabela_jobs);

            tabela_jobs = pop_job(tabela_jobs);
            
        }

        if(msgrcv(idfila_escal_gerente0_volta, &msg_flag , sizeof(msg_flag), 4, IPC_NOWAIT) > 0){
            fim = clock();
            turnaround = (float)(fim - inicio)/ CLOCKS_PER_SEC;
            printf("Todos acabaram a execucao em %f\n", turnaround);

        }



    }
}


int main(int argc, char *argv[])
{

	escalonar();

	return 0;
}