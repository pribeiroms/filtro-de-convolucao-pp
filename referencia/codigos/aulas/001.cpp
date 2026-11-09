/*-------------------------------------------------------------------------------------------------------------------------
Disciplina: Programação Paralela.
Autor.....: Prof. Rodrigo Gonçalves
Objetivo..: Revisão de conceitos básicos de C++.
Bimestre..: 1º Bimestre 2025
Solucao...: RevisaoCpp
--------------------------------------------------------------------------------------------------------------------------
Data Criação....................: 20/08/2025
Data Alteração..................: 20/08/2025  nome: xxxxxx
Alteração Feita: Inclusão do comando system("pause");
--------------------------------------------------------------------------------------------------------------------------
Versão 1.0
*/
#define PausarTexto "Pause" // define uma constante chamada PausarTexto com o valor "Pause"
#define LimparTela "cls"    // define uma constante chamada LimparTela com o valor "cls"
#define Retorne return      // define uma constante chamada Retorne com o valor return.


#include <iostream>
// a diretiva de pré-processamento #include é usada para incluir o conteúdo
// de um arquivo de cabeçalho em um programa C++
// no caso vamos incluir o arquivo iostream que contém
// as definições para os objetos de entrada e saída padrão como std::cin, std::cout, std::cerr

// CTRL + K + C = comenta a linha selecionada
// CTRL + K + U = descomenta a linha selecionada
// CTRL + F5 = executa o programa sem depuração
// F5 = executa o programa com depuração
// CTRL + SHIFT + B = compila o programa
// ALT + SHIFT + ENTER = executa o programa em tela cheia
// CTRL + D = duplica a linha selecionada.
// CTRL + C = no final da linha, copia a linha selecionada. Sem necessidade de selecionar a linha para a cópia.
using namespace std; // espaço de nomes padrão

// a diretiva using namespace std; permite o uso de elementos do namespace std
// :: é o operador de resolução de escopo que é usado para acessar membros de um namespace ou classe


// chaves: indicam o início e o fim de um bloco de código.
int main() {
	// a função main é o ponto de entrada de um programa C++ - função principal.
	// a função main retorna um valor inteiro que indica o status de término do programa
	// o valor 0 geralmente indica que o programa terminou com sucesso
	// a função std::cout é usada para imprimir texto na saída padrão (geralmente o console).

	// << é o operador de inserção que é usado para enviar dados para a saída padrão.
	std::cout << "Hello, World!" << std::endl; // endl indica para inserir um caractere de quebra de linha.

	//system("cls"); // chama o shell do windows e coloca o comando (cls) para limpar a tela do console.
	system("dir"); // chama o shell do windows e coloca o comando (dir) para listar os arquivos do diretório atual.
	// system("color 0A"); // chama o shell do windows e coloca o comando (color 0A) para mudar a cor do texto do console.
	system("pause"); // chama o shell do windows e coloca o comando (pause) para pausar a execução do programa até que o usuário pressione uma tecla.

	//system(LimparTela); // chama o shell do windows e coloca o comando (cls) para limpar a tela do console.
	return 0; // retorna para o sistema operacional o valor 0 indicando que o programa terminou com sucesso.
} // indica o fim do bloco de código da função main
