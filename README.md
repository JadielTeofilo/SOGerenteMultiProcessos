# SOGerenteMultiProcessos
 
 O projeto implementa um comportamento parecido com o do comando “at” no unix, após a execução do escalonador em segundo plano o programa executa postergado pode ser chamado para agendar a execução de dado arquivo binário. 
 
 O diferencial implementado aqui é que o programa de entrada será executado por 16 gerenciadores de execução distintos.


## Como ter o projeto rodando
 
 O programa possui um MakeFile que ao ser executado gera os binários necessários. Para tanto, deve ser acessado pelo terminal a pasta /src e, estando dentro dela, o comando “makefile” chamado.

 Tendo os binários gerados é importante perceber que, dada a natureza desse projeto, se faz necessária a presença de um outro programa que possa ser utilizado como entrada, para fins demonstrativos, na pasta input, se encontra um programa que imprime “Hello, waiting 2 sec!” na tela e morre após 2 segundos.

 Para executar os binários da forma correta deve-se executar primeiro o “escalonador“ em segundo plano e então quaisquer chamadas pelo “executa_postergado” podem ser feitas. Supondo que o diretório que o usuário esteja presente seja o /src, os comandos para o que foi descrito acima são:
 
 * ../binary/escalonador &
 * ../binary/executa_postergado <X segundos> ../input/helloworld
 
 Onde <X segundo> atende por um número positivo qualquer.
 
 Para desligar o escalonador basta executar o binário “shutdown”. De novo, supondo que o usuário esteja na pasta /src, o seguinte comando se aplica:
 
 * ../binary/shutdown
