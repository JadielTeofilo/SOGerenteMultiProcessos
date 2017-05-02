#include "shutdown.h"

void aviso_nao_exec(tipoTabela * tabela_jobs){
	//tem que acessar a lista de tabelas e percorrer ela
	tabela_jobs = shmat(id_mem_lista, (char *)0, 0);
	printf("programas que não serao executados:\n");
	for(;eh_vazia(tabela_jobs); tabela_jobs = tabela_jobs->prox){
		pop_job(tabela_jobs);
	}
}

int main(){
	//struct que evia para o escalonador desligar caso seja 1
	shutInfo desligar;
	int idfila = -1;
	tipoTabela * tabela_jobs;

	desligar.desliga = 1;

	//Verifica a existencia de filas 
	if(msgget(0x1225, 0x1B6) < 0){
		printf("Nenhuma fila encontrada, processo escalonador pode ja estar desligado \n");
		exit(1);
	}
	idfila = msgget(0x1225, 0x1B6);
	//envia mensagem para o escalonador desligar
	if(msgsnd(idfila, &desligar, sizeof(desligar), 0) < 0){
    //if(msgsnd(idfila, &mensagem, sizeof(mensagem)-sizeof(long), 0) >= 0){
		printf("Problema ao enviar as info do novo job\n");
		exit(1);
	}
	//mostrar as informacoes
	//verifica se tem programas nao executados
	aviso_nao_exec(tabela_jobs);
	/*
	caso tenha programas nao executados, deve ser avisado que eles nao serao executados
	imprimir estatisticas de programas executados
		pid
		nome do arquivo executado
		tempo de submissao
		tempo de inicio
		tempo de termino


	*/
	exit(0);

}