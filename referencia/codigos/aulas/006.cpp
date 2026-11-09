/*-------------------------------------------------------------------------------------------------------------------------
Objetivo..: Revisão de laços e controle de fluxo (break/continue).
Arquivo...: 006.cpp
--------------------------------------------------------------------------------------------------------------------------*/
#include <iostream>
using namespace std;

int main() {
	cout << "for de 1 a 5: ";
	// ------------------- Laço de Repetição: for -------------------

	// for: Inicia um laço de repetição comumente usado quando o número de iterações é conhecido.
	// A estrutura dentro dos parênteses tem três partes, separadas por ponto e vírgula:
	// 1. Inicialização (int i = 1;): Executada uma única vez no início do laço. Aqui, declaramos e inicializamos um contador `i`.
	//    O escopo da variável `i` é limitado a este laço `for`.
	// 2. Condição (i <= 5;): Verificada ANTES de cada iteração. Enquanto for verdadeira, o laço continua.
	// 3. Incremento/Atualização (++i;): Executado AO FINAL de cada iteração. Aqui, incrementamos o contador.
	// ++i: Operador de pré-incremento. Ele incrementa o valor de `i` e então retorna o novo valor. Em um laço `for` simples,
	//      seu comportamento é efetivamente o mesmo que o pós-incremento (`i++`).
	for (int i = 1; i <= 5; ++i) cout << i << ' '; // Como há apenas uma instrução, as chaves {} são opcionais.
	cout << endl;

	cout << "while (contagem regressiva): ";
	int c = 5;
	// ------------------- Laço de Repetição: while -------------------

	// while: Inicia um laço que executa enquanto uma condição for verdadeira. É um "laço de pré-teste",
	// pois a condição é verificada ANTES de cada iteração. Se a condição for falsa na primeira vez, o corpo do laço nunca executa.
	while (c > 0) {
		// c--: Operador de pós-decremento. O valor ATUAL de `c` é usado na expressão (enviado para o `cout`),
		//      e SÓ DEPOIS o valor é decrementado. Por isso, a saída começa em 5 e vai até 1.
		cout << c-- << ' ';
	}
	cout << endl;

	cout << "do-while (executa ao menos uma vez): ";
	int x = 0;
	// ------------------- Laço de Repetição: do-while -------------------

	// do: Inicia um laço "do-while", que é um "laço de pós-teste".
	// A principal característica é que o corpo do laço é GARANTIDO a executar pelo menos uma vez,
	// pois a condição só é verificada no final.
	do {
		// x++: Operador de pós-incremento. Similar ao `c--`, o valor atual de `x` (começando em 0) é usado,
		//      e depois `x` é incrementado.
		cout << x++ << ' ';
	} while (x < 3); // A condição é verificada aqui, no final da iteração. Note o ponto e vírgula obrigatório.
	cout << endl;

	cout << "continue/break exemplo: ";
	for (int i = 0; i < 10; ++i) {
		// % : Operador Módulo (ou resto da divisão). `i % 2` retorna o resto da divisão de `i` por 2.
		// Se o resto é 0, o número é par.
		if (i % 2 == 0)
			// continue: É uma instrução de controle de fluxo que interrompe a iteração ATUAL
			// e "pula" imediatamente para o início da próxima iteração (no caso do `for`, para a parte de incremento `++i`).
			// Neste código, se `i` for par, o `cout` abaixo será ignorado.
			continue;

		if (i > 7)
			// break: Esta instrução (já vista no `switch`) interrompe e sai IMEDIATAMENTE do laço
			// em que está contida (`for`, `while` ou `do-while`). A execução continua na primeira linha após o laço.
			// Aqui, assim que `i` for maior que 7, o laço `for` termina prematuramente.
			break;

		cout << i << ' '; // Esta linha só será executada se `i` for ímpar E `i` não for maior que 7.
	}
	cout << endl; // Saída esperada: 1 3 5 7

	system("pause");
	return 0;
}