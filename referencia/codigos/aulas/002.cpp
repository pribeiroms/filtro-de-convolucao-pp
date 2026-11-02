/*-------------------------------------------------------------------------------------------------------------------------
Disciplina: Programação Paralela.
Autor.....: Prof. Rodrigo Gonçalves
Objetivo..: Revisão de variáveis, tipos básicos, const e auto.
Bimestre..: 1º/2025
Solucao...: RevisaoCpp
Arquivo...: 002.cpp
--------------------------------------------------------------------------------------------------------------------------
Data Criação.................: 20/08/2025
Data Alteração...............: 20/08/2025
Alteração Feita..............: Arquivo inicial
--------------------------------------------------------------------------------------------------------------------------
Versão 1.0
*/
#include <iostream>
#include <string>
using namespace std;


int main() {
	// Tipos básicos
	int idade = 20; // inteiro
	double altura = 1.82; // ponto flutuante de dupla precisão
	char inicial = 'R'; // caractere
	bool aprovado = true; // booleano


	// const e auto
	const double PI = 3.14159; // constante
	auto soma = idade + 10; // dedução de tipo (int)

    cout << "Idade: " << idade << '\n'
		 << "Altura: " << altura << '\n'
		 << "Inicial: " << inicial << '\n'
		 << boolalpha << "Aprovado: " << aprovado << '\n'
		 << "PI: " << PI << '\n'
		 << "Soma: " << soma << endl;
	


	// sizeof
	cout << "sizeof(int) = " << sizeof(int) << " bytes" << endl;
	cout << "sizeof(double) = " << sizeof(double) << " bytes" << endl;


	system("pause");
	return 0;
}