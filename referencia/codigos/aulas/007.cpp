/*-------------------------------------------------------------------------------------------------------------------------
Objetivo..: Declaração/definição de funções, sobrecarga, passagem por referência.
Arquivo...: 007.cpp
--------------------------------------------------------------------------------------------------------------------------*/
#include <iostream>
using namespace std;

// ------------------- Protótipos de Funções (Declarações) -------------------

// Um "protótipo" ou "declaração de função" informa ao compilador sobre a existência de uma função ANTES que ela seja usada.
// Ele especifica o tipo de retorno, o nome e os tipos dos parâmetros. Isso permite que você chame a função
// na `main` e defina (implemente) o corpo da função mais tarde no arquivo, melhorando a organização.

// Assinatura: int soma(int a, int b)
// - `int`: Tipo de retorno. A função `soma` retornará um valor inteiro.
// - `soma`: Nome da função.
// - `(int a, int b)`: Lista de parâmetros. A função espera receber dois inteiros como entrada. Por padrão,
//   eles são passados "por valor", o que significa que a função recebe CÓPIAS dos valores originais.
int soma(int a, int b);

// Esta é uma "sobrecarga" (overload) da função `soma`.
// Sobrecarga de função significa ter múltiplas funções com o MESMO nome, mas com listas de parâmetros DIFERENTES
// (seja no número de parâmetros ou nos seus tipos). O compilador decide qual versão chamar com base nos
// argumentos que você fornece na chamada.
int soma(int a, int b, int c);

// Assinatura: void incrementa(int& valor)
// - `void`: Tipo de retorno. Significa que esta função NÃO retorna nenhum valor.
// - `incrementa`: Nome da função.
// - `(int& valor)`: Passagem "por referência".
//   O `&` aqui NÃO é o operador "endereço de". Neste contexto, ele declara que `valor` é uma REFERÊNCIA.
//   Uma referência funciona como um "apelido" para a variável original que foi passada.
//   Qualquer modificação feita em `valor` DENTRO da função afetará DIRETAMENTE a variável original FORA da função.
void incrementa(int& valor);

int main() {
	// O compilador vê que estamos chamando `soma` com dois argumentos inteiros.
	// Ele então procura uma função com a assinatura `soma(int, int)` e a utiliza.
	cout << "soma(2,3) = " << soma(2, 3) << endl;

	// Aqui, a chamada é com três argumentos. O compilador seleciona a versão sobrecarregada
	// com a assinatura `soma(int, int, int)`.
	cout << "soma(2,3,4) = " << soma(2, 3, 4) << endl;

	int n = 10;
	// Aqui chamamos `incrementa`. Como ela espera uma referência, estamos passando um "apelido" para `n`.
	// A função `incrementa` não recebe o valor 10, ela recebe o próprio `n`.
	incrementa(n);
	// Como a função modificou a variável original através da referência, o valor de `n` agora é 11.
	cout << "n apos incrementar = " << n << endl;

	system("pause");
	return 0;
}

// ------------------- Definições das Funções (Implementações) -------------------

// Esta é a "definição" ou "implementação" da função. Contém o corpo real com o código a ser executado.
// A assinatura aqui deve corresponder exatamente à do protótipo.
int soma(int a, int b) {
	// A palavra-chave `return` envia o valor da expressão `a + b` de volta para onde a função foi chamada.
	return a + b;
}

// Definição da versão sobrecarregada.
int soma(int a, int b, int c) {
	return a + b + c;
}

// Definição da função que usa passagem por referência.
void incrementa(int& valor) {
	// += : É um "operador de atribuição composto". `valor += 1` é uma forma curta e eficiente de escrever `valor = valor + 1`.
	// Como `valor` é uma referência para `n` (da `main`), esta linha está, na prática, executando `n = n + 1`.
	valor += 1;
}