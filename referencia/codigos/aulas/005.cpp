/*-------------------------------------------------------------------------------------------------------------------------
Objetivo..: Revisão de if/else, switch e escopo.
Arquivo...: 005.cpp
--------------------------------------------------------------------------------------------------------------------------*/
#include <iostream>
using namespace std;

int main() {
	int nota;
	cout << "Digite a nota (0 a 10): ";
	cin >> nota;

	// ------------------- Estrutura de Decisão: if / else if / else -------------------

	// if: É uma palavra-chave que inicia uma "estrutura condicional". O bloco de código seguinte
	// só será executado SE a condição dentro dos parênteses for avaliada como verdadeira (true).
	// >= : Operador relacional "maior ou igual a".
	if (nota >= 6) {
		// { }: As chaves definem um "bloco de código" ou "escopo". Tudo que está dentro das chaves
		// pertence a este `if`. Se a condição for verdadeira, todo o bloco é executado.
		// Se houver apenas uma instrução, as chaves são opcionais, mas é uma boa prática sempre usá-las para evitar erros.
		cout << "Aprovado" << endl;
	}
	// else if: Esta cláusula é avaliada somente se o `if` anterior (ou `else if` anterior) for falso.
	// Permite criar uma cadeia de condições excludentes. A primeira que for verdadeira será executada, e o resto da cadeia será ignorado.
	else if (nota >= 4) {
		cout << "Prova final" << endl;
	}
	// else: É a cláusula "pega-tudo". O bloco de código do `else` é executado se NENHUMA das condições
	// anteriores (`if` ou `else if`) for verdadeira. Não possui uma condição própria.
	else {
		cout << "Reprovado" << endl;
	}

	int opcao;
	cout << "\nMenu: 1-Listar 2-Inserir 3-Sair -> ";
	cin >> opcao;

	// ------------------- Estrutura de Seleção: switch / case -------------------

	// switch: É uma estrutura de controle que permite a uma variável ser testada contra uma lista de valores (cases).
	// É frequentemente uma alternativa mais limpa a uma longa cadeia de `if-else if` quando se compara uma única variável
	// com valores constantes e integrais (int, char, etc.).
	// A variável `opcao` será o valor de controle.
	switch (opcao) {
		// case: É um "rótulo" que marca um ponto de entrada no bloco `switch`. Se o valor de `opcao` for igual
		// ao valor do `case` (neste caso, 1), a execução do programa "salta" para esta linha.
	case 1:
		cout << "Listando..." << endl;
		// break: É uma instrução crucial que finaliza a execução do `switch`. Sem o `break`,
		// o código continuaria a executar as instruções do próximo `case` ("fall-through"),
		// o que geralmente é um comportamento indesejado.
		break;
	case 2:
		cout << "Inserindo..." << endl;
		break;
	case 3:
		cout << "Saindo..." << endl;
		break;
		// default: Este rótulo é opcional e funciona como o `else`. O código dentro do `default`
		// é executado se o valor da variável (`opcao`) não corresponder a nenhum dos rótulos `case` anteriores.
	default:
		cout << "Opção inválida" << endl;
		// O `break` no final do `default` não é estritamente necessário (pois já é o fim do `switch`),
		// mas é considerado uma boa prática de programação para manter a consistência.
		break;
	}


	system("pause");
	return 0;
}