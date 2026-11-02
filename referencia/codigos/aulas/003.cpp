/*---------------------------------------------------------------------------------------------------------------------
Objetivo..: Revisão de entrada/saída com cin e getline.
Arquivo...: 003.cpp
--------------------------------------------------------------------------------------------------------------------------*/

// ------------------- Diretivas de Pré-processador -------------------

// #include: É uma "diretiva de pré-processador". Ela diz ao compilador para incluir o conteúdo
// de outro arquivo neste ponto. É como copiar e colar o código de uma biblioteca.

#include <iostream> // Inclui a biblioteca de Input/Output Stream. Essencial para usar cin (entrada), cout (saída) e endl.
#include <string>   // Inclui a biblioteca para manipulação de strings, permitindo o uso do tipo string.
#include <limits>   // Embora não esteja explícito, <iostream> geralmente inclui <limits>, que é necessário para `numeric_limits`. É uma boa prática incluí-lo se for usar `numeric_limits` diretamente.

// ------------------- Declaração de Namespace -------------------

// using namespace std;: Esta linha diz ao compilador que usaremos elementos do "namespace" (espaço de nomes) std.
// O namespace std contém toda a biblioteca padrão do C++. Sem essa linha, teríamos que escrever `std::cout`, `std::cin`, `std::string`, etc.
// OBS) Em projetos grandes, é considerado uma má prática usar `using namespace std;` no escopo global
// para evitar conflitos de nomes. Para exemplos pequenos como este, é totalmente aceitável.
using namespace std;

// ------------------- Função Principal -------------------

// int main(): É o ponto de entrada de todo programa C++. O código dentro desta função é o primeiro a ser executado.
// O `int` indica que a função retornará um valor inteiro para o sistema operacional (geralmente 0 para sucesso).
int main() {
	// ------------------- Declaração de Variáveis -------------------

	// string: É um tipo de dado (na verdade, uma classe) da biblioteca padrão para armazenar sequências de caracteres (texto).
	string nome;
	// int: É um tipo de dado primitivo para armazenar números inteiros.
	int idade;

	// ------------------- Entrada de Dados (Problemática Comum) -------------------

	// cout: (Console Output) É o objeto de fluxo de saída padrão, geralmente a tela do console.
	// <<: É o "operador de inserção". Ele "insere" os dados da direita no fluxo de saída à esquerda.
	cout << "Digite seu primeiro nome: ";

	// cin: (Console Input) É o objeto de fluxo deentrada padrão, geralmente o teclado.
	// >>: É o "operador de extração". Ele "extrai" dados do fluxo de entrada e os armazena na variável à direita.
	// IMPORTANTE: O operador `>>` para de ler no primeiro caractere de espaço em branco (espaço, tab, quebra de linha).
	cin >> nome;

	cout << "Digite sua idade: ";
	cin >> idade;

	// ------------------- Limpeza do Buffer de Entrada -------------------

	// O PONTO CRÍTICO: Quando o usuário digita a idade (ex: 25) e pressiona Enter, o buffer de entrada recebe "25\n".
	// `cin >> idade;` lê o "25", mas deixa o caractere de nova linha (`\n`) no buffer.
	// A próxima chamada `getline` leria esse `\n` e pensaria que o usuário digitou uma linha vazia.
	// Para corrigir isso, limpamos o buffer.

	// cin.ignore(): É um método do objeto `cin` que descarta (ignora) caracteres do buffer de entrada.
	// Ele recebe dois argumentos:
	// 1. A quantidade máxima de caracteres a ignorar.
	// 2. O caractere delimitador onde a função deve parar de ignorar.

	// numeric_limits<streamsize>::max(): É uma forma padrão e robusta de dizer "um número muito grande".
	// - `numeric_limits`: É um template de classe que fornece informações sobre as propriedades de tipos numéricos.
	// - `<streamsize>`: É o tipo usado para representar o tamanho de um fluxo (stream).
	// - `::max()`: Retorna o valor máximo que `streamsize` pode conter.
	// Juntos, eles dizem para `ignore()` descartar um número virtualmente infinito de caracteres.

	// '\n': É o caractere delimitador. `ignore()` vai parar assim que encontrar e descartar este caractere.
	// Resumo da linha: "Ignore quantos caracteres forem necessários até encontrar e remover um `\n` do buffer de entrada."
	cin.ignore(numeric_limits<streamsize>::max(), '\n');

	// ------------------- Leitura de Linha Completa -------------------

	string nomeCompleto;
	cout << "Digite seu nome completo: ";
	// getline(): É uma função (não um operador) projetada para ler uma linha inteira de um fluxo de entrada.
	// Ela lê do primeiro argumento (`cin`) e armazena na variável do segundo argumento (`nomeCompleto`),
	// parando quando encontra o caractere de nova linha (`\n`), que também é consumido do buffer.
	getline(cin, nomeCompleto);

	// ------------------- Saída Formatada -------------------

	cout << "Oi, " << nomeCompleto << ". Você tem " << idade << " anos." << endl;

	// ------------------- `endl` vs `\n` -------------------
	// endl: (End Line) É um "manipulador de fluxo". Ele faz duas coisas:
	// 1. Insere um caractere de nova linha (`\n`).
	// 2. Força o "flush" (esvaziamento) do buffer de saída. Isso garante que o texto seja imediatamente escrito na tela.
	cout << "Linha com endl" << endl;

	// '\n': É simplesmente o caractere de nova linha.
	// Ele não força o flush do buffer. O sistema operacional pode decidir agrupar várias saídas e escrevê-las
	// de uma vez só para otimizar o desempenho.
	// Para programas interativos de console, a diferença é imperceptível. Em aplicações que escrevem
	// milhões de linhas em um arquivo (logging, por exemplo), usar `'\n'` é significativamente mais rápido.
	cout << "Linha com \\n" << '\n';

	// ------------------- Pausa para Visualização (Específico do Windows) -------------------

	// system(): É uma função da biblioteca C padrão (<cstdlib>) que executa um comando do sistema operacional.
	// "pause": É um comando do Prompt de Comando do Windows que exibe "Pressione qualquer tecla para continuar..." e aguarda uma tecla.
	// OBS) Isso não é portável (só funciona no Windows) e é geralmente considerado uma má prática em software profissional.
	// Para fins de aprendizado no Visual Studio, é uma forma rápida e fácil de impedir que a janela do console feche imediatamente.
	system("pause");

	// ------------------- Retorno da Função `main` -------------------

	// return: É a palavra-chave que finaliza a execução de uma função e, opcionalmente, retorna um valor.
	// 0: Por convenção, retornar `0` da função `main` indica que o programa foi executado com sucesso.
	// Valores diferentes de zero geralmente indicam que ocorreu algum tipo de erro.
	return 0;
}