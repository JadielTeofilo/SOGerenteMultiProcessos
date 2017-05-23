#include "escalonador.h"


#define CIMA        0
#define DIREITA     1   
#define BAIXO       2
#define ESQUERDA    3

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
tipoExec * tabela_exec;

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

//funao que soh libera as estruturas de dados
void libera_mem(){
    
    //libera todas as estruturas de dados utilizados
    free_job_table(tabela_jobs);
    free_job_table_exec(tabela_exec);
    excluir_fila(idfila_num_job);
    excluir_fila(idfila_estrutura);
    excluir_fila(idfila_shutdown);
    excluir_fila(idfila_escal_gerente0_ida);
    excluir_fila(idfila_escal_gerente0_volta);

    //libera filas e semaforos
    fechar_filasNsems_torus();

    //liberar a memoria compartilhada
    shmctl(id_shm, IPC_RMID, 0);

    // Matar os gerentes
    for (int i = 0; i < 16; i++)
       kill(pid_filho[i], SIGKILL);

    int status;
    //aguardar a morte dos gerentes
    while(wait(&status) != -1);


    

    exit(1);
}

//envia o valor do ultimo job para o executa postergado
void enviar_num_job(jobNumType job_anterior, int idfila){
    if(msgsnd(idfila, &job_anterior, sizeof(job_anterior), IPC_NOWAIT) < 0){
    //if(msgsnd(idfila, &job_anterior, sizeof(job_anterior)-sizeof(long), 0, 0) >= 0){
        printf("Erro no envio da mensagem p/ a fila de mensagem\n");
    }
}

//se tiver mensagem do executa postergado, a lista de jobs eh atualizada
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


    tabela_jobs = append_job_ordenado(info_job.job, info_job.submissao, info_job.data, 
                                        info_job.arq_exec, tabela_jobs);

    //envia o ultimo job atualizado
    //define o identificador unico do job como o anterior + 1
    job_anterior.type = 1;
    job_anterior.job_num = info_job.job+1;
    enviar_num_job(job_anterior, idfila_num_job);

    return tabela_jobs;
}

//Seta alarme para para o momento da execucao do job mais proximo
int def_alrm_execucao_job(tipoTabela * tabela_jobs){
    time_t current_time = time(NULL);
    int segundos_para_execucao = 0;
    //se a tabela for vazia, não tem programa para executar
    if(tabela_jobs==NULL){
        return -1;
    }
    
    //calcula quantos segundos falta
    segundos_para_execucao = tabela_jobs->data - current_time;


    if(segundos_para_execucao >= 0){
        //seta alarme para a quantidade de segundos que falta 
        alarm(segundos_para_execucao);
        return 0;
    }
    else{
        //caso o horario jah tenha passado por algum motivo, envia o sinal imediatamente
        kill(getpid(), SIGALRM);
        return 0;
    }
}

// imprimir dados dos jobs que nao foram executados
void imprimir_remanescentes(tipoTabela * tabela_jobs){
    tipoTabela * tabela_aux;
    char* c_time_string;

    //tem que acessar a lista de tabelas e percorrer ela
    printf("\nprogramas que não serao executados:\n");
    printf("job         arquivo executavel      data\n");
    for(;tabela_jobs!=NULL; tabela_jobs = tabela_jobs->prox){
        c_time_string = ctime(&tabela_jobs->data);
        tabela_aux = tabela_jobs;
        printf("%d          %s      %s", tabela_aux->job_num, tabela_aux->arq_exec, c_time_string);
        printf("----------------------------------------------------------------\n");
    }
}

//imprimir informacoes de dados que jah foram executados
void imprimir_executados(){
    tipoExec * tabela_aux;

    //tem que acessar a lista de tabelas e percorrer ela
    printf("\nprogramas executados:\n");
    printf("    job         arquivo executavel\n");
    for(;tabela_exec!=NULL; tabela_exec = tabela_exec->prox){
        tabela_aux = tabela_exec;
        printf("    %d          %s\n", 
        tabela_aux->job_num, tabela_aux->arq_exec);
        printf("tempo de submissão: %s", asctime(localtime(&tabela_exec->data)) );
        printf("inicio:             %s", asctime(localtime(&tabela_exec->inicio)));
        printf("termino:            %s", asctime(localtime(&tabela_exec->fim)));
        printf("----------------------------------------------------------------\n");
        
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

    //mandar mensagem para o zero com dados do programa a ser executado
    if(msgsnd(idfila_escal_gerente0_ida, &mensagem, sizeof(mensagem), IPC_NOWAIT)<0){
    //if(msgsnd(idfila_escal_gerente0_ida, &dados_job, sizeof(tipoTabela)-sizeof(long), IPC_NOWAIT)<0){
        printf("erro na hora de enviar dados para o gerente 0\n");
        libera_mem();
    }
}

//calcula id da fila de envio para cada processo escalonador
int calcular_index_fila_envio(int lado, int meu_id){
    switch (lado){
        //caso de fila para cima
        case CIMA: 
            return(meu_id*4);
        //caso de fila para a direita  
        case DIREITA:
            return((meu_id*4)+1);
        //caso de fila para baixo
        case BAIXO:
            return((meu_id*4)+2);
        //caso de fila para a esquerda
        case ESQUERDA:
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
        case CIMA:
            lin = (lin-1)%4;
            //caso menor que zero pegar o equivalente positivo
            (lin < 0)?lin += 4 : lin;
            break;
        case DIREITA:
            col = (col+1)%4;
            (col < 0)?col += 4 : col;
            break;
        case BAIXO:
            lin = (lin+1)%4;
            (lin < 0)?lin += 4 : lin;
            break;
        case ESQUERDA:
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

    //descobre quem deve enviar para meu_id, sabendo assim de quem meu_id deve receber
    switch(lado){
        case CIMA:
            return calcular_index_fila_envio( BAIXO, id);
        case DIREITA:
            return calcular_index_fila_envio( ESQUERDA, id);
        case BAIXO:
            return calcular_index_fila_envio( CIMA, id);
        case ESQUERDA:
            return calcular_index_fila_envio( DIREITA, id);
    }
}

//Envia a "mensagem" para os quatro vizinhos
void envia_msgs_vizinho(InfoMsgTorus mensagem, int meu_id, int * id_torus_fila, int * id_torus_sem){
    int i;

    for(i=0; i<4; i++){
        int idfila = id_torus_fila[calcular_index_fila_envio(i, meu_id)];
      
        if(msgsnd(idfila, &mensagem, sizeof(mensagem), IPC_NOWAIT)<0){
        //if(msgsnd(idfila_escal_gerente0_ida, &dados_job, sizeof(tipoTabela)-sizeof(long), IPC_NOWAIT)<0){
            printf("erro na hora de enviar dados para os vizinhos errno: %d - %d:%d\n",errno, meu_id, idfila);
            libera_mem();
            //return;
        }
        //desbloqueia o vizinho para receber a msg
        v_sem(id_torus_sem[calcular_id_vizinho(i, meu_id)]);
    }
}

//Recebe atual posicao e devolve a saida para chegar no gerente 0
int roteador(int meu_id){

    //Caso sejam da primeira coluna o destino eh para cima
    //Caso contrario o destino eh para o lado esquerdo

    // 0 <- 1 <- 2 <- 3
    // ^
    // |
    // 4 <- 5 <- 6 <- 7
    // ^
    // |
    // 8 <- 9 <- 10<- 11
    // ^
    // |
    // 12<- 13<- 14<- 15

    return (meu_id%4)? ESQUERDA : CIMA;
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
                //libera o nodo avisando que a mensagem foi enviada
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
    InfoMsgTorus mensagem_aux;
    int i;
    int ainda_n_enviei = 1;

    //aguarda receber 4 mensagens
    for(i=0; i<4; i++){
        // aguarda o sinal de que recebeu uma mensagem
        p_sem(id_torus_sem[meu_id]);
        //Pega entao a mensagem que recebeu de algum dos lados
        for (int j = 0; j < 4; ++j)
        {
            int idfila = id_torus_fila[calcular_idfila_receber(j, meu_id)];
            if(msgrcv(idfila, &mensagem , sizeof(mensagem), 2, IPC_NOWAIT) > 0){
                // envia a mensagem para os quatro lados
                mensagem_aux = mensagem;
                if(ainda_n_enviei){
                    //soh envia para os vizinhos se não tiver enviado desse conjunto de mensagens
                    envia_msgs_vizinho(mensagem_aux, meu_id, id_torus_fila, id_torus_sem);
                    ainda_n_enviei = 0;
                }
                // ele sai caso ache uma msg
                break;
            }
        } 
    }
    return mensagem_aux;
}

//codigo do filho/gerenciador de execucao
void gerenciar_execucao(int meu_id, int * id_torus_fila, int * id_torus_sem){
    InfoMsgTorus mensagem;
    int status;
    int pid;
    while(1){
        //o gerenciador 0 possui comportamento diferente pois se comunica com o escalonador  
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
                    // printf("erro na recepção de mensagem do escalonador\n");
                    libera_mem();
                }
                // Envia mensagem para todos vizinhos 
                envia_msgs_vizinho(mensagem, meu_id, id_torus_fila, id_torus_sem);
                //laco para receber todos os avisos de termino
                int i;
                for(i=0; i<4; i++){
                    // aguarda o sinal de que recebeu uma mensagem
                    p_sem(id_torus_sem[meu_id]);
                    //Pega entao a mensagem que recebeu de algum dos lados
                    for (int j = 0; j < 4; ++j)
                    {
                        int idfila = id_torus_fila[calcular_idfila_receber(j, meu_id)];
                        if(msgrcv(idfila, &mensagem , sizeof(mensagem), 2, IPC_NOWAIT) > 0){
                            // ele sai caso ache uma msg
                            break;
                        }
                    } 
                }
                //Roda o programa solicitado
                if((pid = fork())<0){
                    printf("erro na criacao de fork\n");
                    libera_mem();
                }
                if(pid == 0){
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
                        //Recebe a msg de termino
                        if(msgrcv(idfila, &msg_flag , sizeof(msg_flag), 4, IPC_NOWAIT) > 0){
                            //incrementa num finalizados
                            num_finalizados++;
                            // ele sai caso ache uma msg
                            break;
                        }
                    }
                    //o nodo 0 deve receber 15 avisos de finalizado para enviar informações para o escalonador
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

//cria as filas e os semaforos
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

// Realiza a criacao da estrutura torus
void montar_torus(){
    int pid, i;
    int meu_id = 0;
    int *psem;

    //Criar as filas para comunicacao entre os gerentes
    criar_filasNsem_torus(id_torus_fila, id_torus_sem);

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

//desliga o programa
void shutdown(){
    //imprime as informaçoes dos jobs que jah foram executados
    imprimir_executados(tabela_exec);

    //imprime as informações dos jobs que não foram executados
    imprimir_remanescentes(tabela_jobs);
    // printf("Desligando o Programa...\n");

    //Exclui as filas e os semaforos utilizados
    libera_mem();
}

//Na chegada de um sinal avisando do horario realiza 
void tratar_sig_horario_chegou(){
    //para a contagem do turnaround
    time_t inicio;
    time_t fim;
    float turnaround;
    //para a conversao do time_t
    struct tm* timeinfo;


    //para o recebimento da mensagem
    InfoFlgTorus msg_flag;
    
    //começa a contar o tempo de execuçao
    time(&inicio);


    //funcao chama os gerenciadores de execucao para executar o job
    informar_ger_exec_zero(tabela_jobs);
    //Aguarda o fim da execucao de todos
    if(msgrcv(idfila_escal_gerente0_volta, &msg_flag , sizeof(msg_flag), 4, 0) > 0){
        //marca do fim do tempo de execução
        time(&fim);

        //calcula o tempo de exec
        turnaround = (float)(fim - inicio);
         tabela_exec = insere_job(tabela_jobs, tabela_exec, inicio, fim);
        //converte o tempo agendado
        timeinfo = localtime ( &inicio );
        printf("job = %d, arquivo = %s, turnaround = %f, execucao iniciada = %s",
                tabela_jobs->job_num, tabela_jobs->arq_exec, turnaround, asctime(timeinfo));
        //insere dados na tabela de jobs jah executados
        // printf("%d:%s %d:%s %d:%s\n",tabela_exec->data, asctime(localtime(&tabela_exec->data)), 
        //tabela_exec->inicio, asctime(localtime(&tabela_exec->inicio)), tabela_exec->fim, asctime(localtime(&tabela_exec->fim)));
       
    }

    //começa a verificar o proximo job
    tabela_jobs = pop_job(tabela_jobs); 

    //seta o alarm de novo, agora para o proximo na fila
    def_alrm_execucao_job(tabela_jobs);
}

// Na chegada do aviso de um novo job eh feito o tratamento do mesmo
void trata_sig_novo_job(){
    int job_num = -5;
    if(tabela_jobs != NULL)
        job_num = tabela_jobs->job_num;
    //Le um novo job inserido 
    tabela_jobs = atualiza_info_job(idfila_estrutura, tabela_jobs, idfila_num_job);

    //Caso o job com tempo de execucao mais proximo tenha mudado atualiza o alarm
    if(job_num != tabela_jobs->job_num){
        def_alrm_execucao_job(tabela_jobs);
    }
}

void main(){
    //quando receber um sinal de termino, desliga
    signal(SIGTERM, shutdown);
    //p quando chegar aviso novo job 
    signal(SIGUSR2, trata_sig_novo_job);
    //p quando chegar aviso de que deu o horario 
    signal(SIGALRM, tratar_sig_horario_chegou);



    //utilizado para informar valor do ultimo job
    jobNumType job_anterior;
    job_anterior.type = 1;
    job_anterior.job_num = 1;

    //para enviar e receber mensagem de outros processos
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

    //envia o primeiro job para a fila de mensagens
    enviar_num_job(job_anterior, idfila_num_job);

    //Iniciar tabela de jobs
    tabela_jobs = init_job_table();
    tabela_exec = init_job_table_exec();

    //cria os gerentes de execução segundo a topologia torus
    montar_torus();

    //laco de execucao
    while(1){
        //Aguarda a signals que de novo job ou de chegada de horario
        pause();
    }
}
