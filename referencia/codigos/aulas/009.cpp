/*---------------------------------------------------------------------------------------------------------------
Objetivo..: Endereço (&), desreferência (*), ponteiros e referências.
Arquivo...: 009.cpp
---------------------------------------------------------------------------------------------------------------*/
#include <iostream>
// #include <memory>: Inclui a biblioteca necessária para usar "ponteiros inteligentes" (smart pointers),
// como o `std::unique_ptr`.
#include <memory> 
using namespace std;

// ------------------- Funções com Ponteiros e Referências -------------------

// void dobra(int *p): Esta função recebe um "ponteiro para um inteiro" como parâmetro.
// - `int *p`: `p` não é um inteiro, mas sim uma variável que armazena o ENDEREÇO de memória de um inteiro.
void dobra(int* p) {
    // if (p): É uma prática de segurança crucial verificar se um ponteiro não é nulo (`nullptr`) antes de usá-lo.
    // Tentar acessar um endereço nulo causa uma falha de segmentação (crash).
    if (p)
        // *p *= 2: O `*` aqui é o "operador de desreferência". Ele diz: "acesse o valor que está no endereço
        // de memória para o qual `p` aponta". A linha modifica o valor ORIGINAL que está sendo apontado.
        *p *= 2;
}
// void triplica(int &r): Esta função recebe uma "referência para um inteiro". (Revisão do código 007)
// A sintaxe é mais limpa e a referência não pode ser nula, tornando-a geralmente mais segura que ponteiros para este fim.
void triplica(int& r) { r *= 3; }


int main() {
    // ------------------- Ponteiros: O Básico -------------------
    int x = 5;

    // int *px = &x;: Esta é a linha mais importante. Vamos quebrá-la:
    // - `int *px;`: Declara uma variável chamada `px`. O `*` na declaração significa que `px` é um PONTEIRO. Seu tipo é "ponteiro para int".
    // - `&x`: O `&` aqui é o "operador de endereço". Ele obtém o endereço de memória da variável `x`.
    // - A linha completa significa: "Crie um ponteiro `px` e armazene nele o endereço de memória de `x`."
    int* px = &x;

    cout << "x = " << x << ", &x = " << &x << endl; // Imprime o valor de x e seu endereço de memória.

    // - `px`: Imprime o valor contido em `px`, que é o endereço de `x`. O resultado será igual ao de `&x`.
    // - `*px`: Usa o operador de desreferência para acessar o VALOR no endereço para o qual `px` aponta. O resultado será igual ao de `x`.
    cout << "px = " << px << ", *px = " << *px << endl;

    // Passamos o ponteiro `px` para a função. A função `dobra` usará o endereço para modificar o valor original de `x`.
    dobra(px);
    cout << "x após dobra = " << x << endl;

    // Passamos a própria variável `x` para uma função que espera uma referência. A sintaxe da chamada é mais simples.
    triplica(x);
    cout << "x após triplica = " << x << endl;

    // ------------------- Alocação Dinâmica de Memória (Heap) -------------------

    // new: É um operador que aloca memória no "heap" (ou "free store"), uma área de memória disponível para o programa durante sua execução.
    // A memória alocada no heap NÃO é liberada automaticamente no fim do escopo, como as variáveis `x` e `px`.
    // `new int(42)` aloca espaço para um inteiro, inicializa-o com 42 e retorna o endereço dessa memória.
    int* pHeap = new int(42);
    cout << "*pHeap = " << *pHeap << endl;

    // delete: É o operador que libera a memória alocada com `new`.
    // É RESPONSABILIDADE do programador chamar `delete` para cada `new`.
    // Se você esquecer, ocorre um "vazamento de memória" (memory leak).
    delete pHeap;

    // ------------------- Ponteiros Inteligentes (Modern C++) -------------------

    // `unique_ptr` é um "ponteiro inteligente" que encapsula um ponteiro bruto.
    // Ele implementa o princípio RAII ("Resource Acquisition Is Initialization"):
    // A posse do recurso (memória) está ligada ao tempo de vida do objeto (`up`).

    // `make_unique<int>(99)`: É a forma moderna e segura de criar um `unique_ptr`.
    // Ele aloca um `int` no heap com o valor 99 e o entrega para o `unique_ptr` gerenciar.
    unique_ptr<int> up = make_unique<int>(99);

    // Ele pode ser desreferenciado com `*` como um ponteiro normal.
    cout << "*up = " << *up << endl;

    // A MÁGICA: Quando `up` sai do escopo (no final de `main`), seu destrutor é chamado automaticamente,
    // e esse destrutor chama `delete` no ponteiro que ele gerencia.
    // Não há necessidade de `delete` manual, eliminando o risco de vazamentos de memória.

    system("pause");
    return 0;
}