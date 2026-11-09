/*-------------------------------------------------------------------------------------------------------------------------
Objetivo..: Operadores aritméticos, relacionais, lógicos e casting.
Arquivo...: 004.cpp
--------------------------------------------------------------------------------------------------------------------------*/
#include <iostream>
using namespace std;

int main() {
	// = : Este é o operador de ATRIBUIÇÃO. Ele pega o valor à direita e o armazena na variável à esquerda.
	// A vírgula (,) aqui é usada para declarar múltiplas variáveis do mesmo tipo (`int`) em uma única linha.
	int a = 7, b = 3;

	// ------------------- Operadores Aritméticos -------------------

	// + : Operador de Adição.
	cout << "a + b = " << (a + b) << endl;
	// - : Operador de Subtração.
	cout << "a - b = " << (a - b) << endl;
	// * : Operador de Multiplicação.
	cout << "a * b = " << (a * b) << endl;

	// / : Operador de Divisão.
	// IMPORTANTE: Quando ambos os operandos (`a` e `b`) são inteiros, o C++ realiza uma "divisão inteira".
	// Isso significa que qualquer parte fracionária do resultado é descartada (truncada), não arredondada.
	// Ex: 7 dividido por 3 é 2.333..., mas o resultado aqui será apenas 2.
	cout << "a / b = " << (a / b) << " (divisão inteira)" << endl;

	// ------------------- Conversão de Tipo (Casting) -------------------

	// static_cast<double>(b): Esta é uma operação de "casting" (conversão de tipo) explícita e segura.
	// - `static_cast`: É um operador do C++ que converte um tipo de dado em outro em tempo de compilação.
	// - `<double>`: É o tipo de destino. `double` é um tipo para números de ponto flutuante (com casas decimais).
	// Ao converter `b` para `double`, a expressão `a / (um double)` força o compilador a promover `a` para `double` também
	// antes de realizar a operação. O resultado é uma "divisão real", que preserva as casas decimais.
	cout << "a / (double)b = " << (a / static_cast<double>(b)) << " (divisão real)" << endl;

	// ------------------- Manipulador de Fluxo e Operadores Lógicos/Relacionais -------------------

	// boolalpha: É um "manipulador de fluxo" (como o `endl`). Uma vez usado no `cout`, ele faz com que
	// valores booleanos (true/false) sejam impressos como texto ("true" ou "false") em vez de números (1 ou 0).
	cout << boolalpha;

	// == : Operador relacional de IGUALDADE. Compara se os dois valores são iguais. Retorna `true` ou `false`.
	cout << "a == b? " << (a == b) << endl;
	// != : Operador relacional de DIFERENÇA. Compara se os dois valores são diferentes.
	cout << "a != b? " << (a != b) << endl;

	// && : Operador lógico "E" (AND). A expressão inteira só é `true` se AMBAS as condições (`a > b` e `b > 0`) forem `true`.
	// > : Operador relacional "MAIOR QUE".
	cout << "a > b && b > 0? " << (a > b && b > 0) << endl;

	// ! : Operador lógico "NÃO" (NOT). Ele inverte o valor booleano da expressão à sua frente.
	// < : Operador relacional "MENOR QUE".
	// A expressão `(a < b)` resulta em `false` (pois 7 não é menor que 3). O `!` inverte `false` para `true`.
	cout << "!(a < b) ? " << !(a < b) << endl;

	// ------------------- Operador Condicional (Ternário) -------------------

	// (condição) ? valor_se_true : valor_se_false
	// Este é o operador ternário, uma forma compacta de escrever uma instrução `if-else`.
	// 1. `(a > b)`: A condição é avaliada. Como 7 > 3, o resultado é `true`.
	// 2. `? a`: Como a condição foi `true`, o valor antes dos dois pontos (`:`) é escolhido, que é o valor de `a` (7).
	// 3. `: b`: Se a condição fosse `false`, o valor de `b` seria escolhido.
	// O valor resultante (7) é então atribuído à variável `maior`.
	int maior = (a > b) ? a : b;
	cout << "Maior = " << maior << endl;


	system("pause");
	return 0;
}