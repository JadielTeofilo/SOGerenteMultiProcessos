/**

Programa com funcoes para tratamento da 
estrutura de dados que contem a tabela de jobs
ordenada de acordo com a data

Implementacao em forma de uma lista encadeada


**/

#include <time.h>


//Definicoes de dados
struct lista_tabela
{
	//Dados de cada linha da tabela
	int job_index;
	time_t data;
	char arq_exec[6969];

	// Para apontar para a proxima linha
	struct lista_tabela * prox;

}typedef tipoTabela;


//Definicoes de funcoes

//Inicia a tabela/lista encadeada
tipoTabela * init_job_table();

//Libera todo o espaco utilizado pela tabela
void delete_job_table(tipoTabela* ptr);

//Insere de forma ordenada na tabela de acordo com a data
void append_job_ordenado(int job_num, time_t data, char * arq_exec, tipoTabela* ptr_tabela);

//Pegar um job da tabela pelo num job
void get_job(int job_num, tipoTabela* ptr_tabela);

//Retira e retorna o primeiro job da tabela
void pop_job(tipoTabela * ptr_tabela);


