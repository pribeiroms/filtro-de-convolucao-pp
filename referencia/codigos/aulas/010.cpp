/*-----------------------------------------------------------------------------------------------------------------
Objetivo..: Definir struct e class com construtor e métodos.
Arquivo...: 010.cpp
-----------------------------------------------------------------------------------------------------------------*/
#include <iostream>
#include <string>
using namespace std;

// ------------------- Agregação de Dados com `struct` -------------------

// struct: Palavra-chave que define uma "estrutura", um tipo de dado composto que agrupa diferentes variáveis sob um único nome.
// É uma das maneiras de criar seus próprios tipos de dados.
// Por padrão, todos os membros de uma `struct` são PÚBLICOS (`public`).
struct Pessoa {
    // Estes são os "membros" da struct. Cada objeto do tipo `Pessoa` terá sua própria `nome` e `idade`.
    string nome;
    // int idade{};: A inicialização com chaves `{}` aqui é chamada de "inicialização de membro na classe" (in-class member initialization).
    // Se um objeto `Pessoa` for criado sem especificar uma idade, ela será inicializada com zero por padrão.
    int idade{};
};

// ------------------- Encapsulamento de Dados com `class` -------------------

// class: Palavra-chave que define uma "classe", o bloco de construção fundamental da Programação Orientada a Objetos em C++.
// Assim como a `struct`, agrupa dados (membros) e funções (métodos).
// A principal diferença é que, por padrão, todos os membros de uma `class` são PRIVADOS (`private`).
class ContaBancaria {
    // private: É um "especificador de acesso". Membros declarados como `private` só podem ser acessados
    // por funções-membro (métodos) da própria classe. Isso é chamado de ENCAPSULAMENTO, pois protege os dados de modificações externas indesejadas.
private:
    string titular;
    double saldo{ 0.0 }; // Inicialização de membro na classe.

    // public: Outro especificador de acesso. Membros declarados como `public` formam a "interface" da classe.
    // Eles podem ser acessados de qualquer lugar fora da classe, permitindo a interação com o objeto.
public:
    // Este é um "Construtor". É um método especial que é chamado automaticamente quando um objeto da classe é criado.
    // Seu propósito é inicializar os membros do objeto. Ele tem o mesmo nome da classe e não tem tipo de retorno.
    // - `const string &t`: Parâmetro passado por "referência constante". É eficiente (evita copiar a string) e seguro (o `const` impede a modificação do original).
    // - `double s = 0.0`: Parâmetro com "valor padrão". Se o segundo argumento não for fornecido na criação do objeto, `s` será 0.0.
    // - `: titular(t), saldo(s)`: Esta é a "lista de inicialização de membros". É a forma preferida e mais eficiente de inicializar membros
    //   em um construtor, em vez de usar atribuições no corpo `{}`.
    ContaBancaria(const string& t, double s = 0.0) : titular(t), saldo(s) {}

    // `depositar` é uma "função-membro" ou "método". É uma função que pertence à classe e pode operar em seus membros.
    void depositar(double valor) { saldo += valor; }

    bool sacar(double valor) {
        if (valor <= saldo) { saldo -= valor; return true; }
        return false;
    }

    // `const` no final de um método: Isso declara que o método é "constante".
    // É uma promessa de que este método NÃO modificará nenhum membro da classe (como `titular` ou `saldo`).
    // Ele apenas lê os dados. Isso melhora a segurança e permite que o método seja chamado em objetos constantes.
    void imprimir() const {
        cout << "Titular: " << titular << ", Saldo: R$ " << saldo << endl;
    }
};

int main() {
    // Pessoa p{"Rodrigo", 35};: Cria uma "instância" (ou "objeto") da struct `Pessoa`.
    // A sintaxe com `{}` é chamada de "inicialização uniforme" (uniform initialization).
    Pessoa p{ "Rodrigo", 35 };
    // . (ponto): É o "operador de acesso a membro". Usado para acessar os membros públicos de um objeto `struct` ou `class`.
    cout << p.nome << ", " << p.idade << " anos" << endl;

    // Cria um objeto `c` da classe `ContaBancaria`. O construtor é chamado aqui com "Maria" e 100.0.
    ContaBancaria c{ "Maria", 100.0 };
    // c.depositar(50);: Chama o método `depositar` no objeto `c`. O estado interno de `c` (seu saldo) é modificado.
    c.depositar(50);
    if (!c.sacar(200)) cout << "Saldo insuficiente" << endl;
    // c.imprimir();: Chama o método `imprimir`, que lê e exibe o estado atual do objeto.
    c.imprimir();

    system("pause");
    return 0;
}