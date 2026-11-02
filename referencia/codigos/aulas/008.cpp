/*-------------------------------------------------------------------------------------------------------------------------
Objetivo..: Uso de arrays estáticos e std::vector.
Arquivo...: 008.cpp
--------------------------------------------------------------------------------------------------------------------------*/
#include <iostream>
// #include <vector>: Inclui a biblioteca da STL (Standard Template Library) que define a classe de template `std::vector`.
// É necessário para usar vetores dinâmicos em seu código.
#include <vector>
using namespace std;

int main() {
	// ------------------- Array Estático (estilo C) -------------------

	// int arr[5]: Declara um "array" (arranjo) estático.
	// - Um array é uma coleção de elementos do mesmo tipo, armazenados em posições contíguas de memória.
	// - "Estático" aqui significa que seu tamanho (5) é fixo e definido em tempo de compilação. Ele não pode ser alterado durante a execução.
	// = { 1,2,3,4,5 };: Esta é uma "lista inicializadora" que preenche o array com os valores fornecidos no momento da sua criação.
	int arr[5] = { 1, 2, 3, 4, 5 };

	int somaArr = 0;
	// A iteração sobre um array estático geralmente requer saber seu tamanho exato (o "número mágico" 5 aqui).
	for (int i = 0; i < 5; ++i) {
		// arr[i]: Acessa um elemento do array usando o "operador de subscrito" [].
		// Os índices em C++ são baseados em zero, então `arr[0]` é o primeiro elemento (1), `arr[1]` é o segundo (2), e assim por diante, até `arr[4]`.
		somaArr += arr[i];
	}
	cout << "Soma (array) = " << somaArr << endl;

	// ------------------- Vector Dinâmico (estilo C++) -------------------

	// vector<int> v: Declara uma variável `v` do tipo `vector`.
	// - `vector`: É uma classe da STL que implementa um array dinâmico. Seu tamanho pode crescer ou diminuir durante a execução do programa.
	// - `<int>`: `vector` é um "template de classe", o que significa que você deve especificar o tipo de dado que ele irá armazenar dentro dos colchetes angulares (`<>`).
	// = { 10,20,30 };: Assim como no array, podemos usar uma lista inicializadora. `v` começa com 3 elementos.
	vector<int> v = { 10, 20, 30 };

	// v.push_back(40): `push_back()` é um "método" (uma função que pertence a um objeto) da classe `vector`.
	// Ele adiciona um novo elemento (neste caso, 40) ao FINAL do vetor, aumentando seu tamanho automaticamente.
	// Este é o principal exemplo de sua natureza "dinâmica".
	v.push_back(40);

	int somaVec = 0;
	// for (int x : v): Este é um "range-based for loop" (ou laço "for-each"), introduzido no C++11.
	// É uma forma muito mais simples e segura de iterar sobre todos os elementos de um contêiner (como um `vector` ou array).
	// - `v`: O contêiner sobre o qual vamos iterar.
	// - `int x`: A cada iteração, `x` receberá uma CÓPIA do próximo elemento de `v`.
	// O laço executa uma vez para cada elemento em `v`, sem a necessidade de gerenciar índices manualmente.
	for (int x : v) somaVec += x;
	cout << "Soma (vector) = " << somaVec << endl;

	// v.size(): Outro método útil de `vector`. Ele retorna o número atual de elementos no vetor.
	// Usar `.size()` torna o código mais robusto, pois não dependemos de um tamanho fixo codificado manualmente.
	cout << "Tamanho do vector = " << v.size() << endl;

	system("pause");
	return 0;
}