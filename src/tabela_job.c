/**

Programa com funcoes para tratamento da 
estrutura de dados que contem a tabela de jobs
ordenada de acordo com a data

Implementacao em forma de uma lista encadeada


**/

#include "tabela_job.h"
#include <stdio.h>


//Inicia a tabela/lista encadeada
tipoTabela * init_job_table(){
	return NULL;
}

//Libera todo o espaco utilizado pela tabela
void free_job_table(tipoTabela* ptr_tabela){
	tipoTabela * ptr_prox = ptr_tabela;
	while(ptr_tabela!=NULL){
		ptr_prox = ptr_tabela->prox;
		free(ptr_tabela);
		ptr_tabela = ptr_prox;
	}
}

int eh_vazia(tipoTabela * ptr){
	return (ptr == NULL);
}

//Insere de forma ordenada na tabela de acordo com a data
tipoTabela* append_job_ordenado(int job_num, time_t data, char * arq_exec, tipoTabela* ptr_tabela){
	//Cria um anterior para verificar a ordem e inserir no meio
	tipoTabela * ptr_anterior;

	//Cria nodo e coloca os novos dados
	tipoTabela * ptr_new_node = (tipoTabela*) malloc(sizeof(tipoTabela));
	ptr_new_node->data = data;
	ptr_new_node->job_num = job_num;
	strcpy(ptr_new_node->arq_exec, arq_exec);
	tipoTabela* ptr_inicial = ptr_tabela;

	//Caso em que a tabela jah esta vazia
	if(eh_vazia(ptr_inicial)){
		ptr_new_node->prox = NULL;
		return ptr_new_node;
	}

	//Achar a posicao para colocar o nodo (tem que ser ordenado)
	while(!eh_vazia(ptr_tabela)){

		//Caso a data seja maior e nao eh o ultimo nodo
		if(ptr_new_node->data > ptr_tabela->data){
			if(ptr_tabela->prox != NULL){
				ptr_tabela->prox = ptr_new_node;
				return ptr_inicial;
			}else{
				ptr_anterior = ptr_tabela;
				ptr_tabela = ptr_tabela->prox;
			}
		}
		else{
			// Se a posicao a ser colocada for a inicial
			if(ptr_tabela == ptr_inicial){
				// Coloca o nodo no inicio
				ptr_new_node->prox = ptr_inicial;
				return ptr_new_node;
			}
			//Caso a posicao seja a ultima
			if(ptr_tabela->prox == NULL){
				// Coloca o nodo no inicio
				ptr_new_node->prox = NULL;
				ptr_tabela->prox = ptr_new_node;
				return ptr_inicial;
			}
			//Caso seja em algum lugar no meio
			ptr_anterior->prox = ptr_new_node;
			ptr_new_node->prox = ptr_tabela;
			return ptr_inicial;
		}
	}
}

//Pegar um job da tabela pelo num job
tipoTabela* get_job(int job_num, tipoTabela* ptr_tabela){
	
	for(;eh_vazia(ptr_tabela); ptr_tabela = ptr_tabela->prox){
		if(job_num == ptr_tabela->job_num){
			return ptr_tabela;
		}
	}
	printf("Job nao encontrado.\n");
	return NULL;
}

//Retira e retorna o primeiro job da tabela
tipoTabela * pop_job(tipoTabela * ptr_tabela){
	tipoTabela * ptr_aux;

	//salva o ponteiro do primeiro nodo e atualiza o primeiro nodo
	ptr_aux = ptr_tabela;
	ptr_tabela = ptr_tabela->prox;
	free(ptr_aux);

	return ptr_tabela;
}


