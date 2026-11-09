/*--------------------------------------------------------------------------------------------------------------------
Objetivo..: Leitura e escrita em arquivos de texto com fstream.
Arquivo...: 011.cpp
--------------------------------------------------------------------------------------------------------------------*/
#include <iostream>
// #include <fstream>: Inclui a biblioteca de "file stream" (fluxo de arquivo).
// É necessária para usar as classes `ofstream` (saída) e `ifstream` (entrada) para manipulação de arquivos.
#include <fstream> 
#include <string>
#include <vector>
// <utility> é implicitamente incluído por outros cabeçalhos, mas é a casa do std::pair.
using namespace std;

int main() {
    // 1) Escrever um arquivo texto simples

    // Usar um bloco de escopo `{}` é uma excelente prática com fluxos de arquivo.
    // Quando a execução sai do bloco, o destrutor do objeto `out` é chamado automaticamente,
    // o que garante que o arquivo seja fechado. Isso é um exemplo do princípio RAII.
    {
        // ofstream: (Output File Stream) Classe para escrever em arquivos. Pense nela como o `cout` para arquivos.
        // O construtor `ofstream out("saida.txt");` tenta abrir (ou criar, se não existir) o arquivo "saida.txt" em modo de escrita.
        // Se o arquivo já existir, seu conteúdo será apagado (overwrite).
        ofstream out("saida.txt");

        // É crucial verificar se o arquivo foi aberto com sucesso.
        // Objetos de stream podem ser avaliados em um contexto booleano. `!out` é `true` se a abertura falhou (ex: sem permissão).
        if (!out) {
            // cerr: (Console Error) É o fluxo de saída de erro padrão. É como o `cout`, mas destinado a mensagens de erro.
            // A principal vantagem é que `cerr` não usa buffer, então a mensagem de erro é exibida imediatamente.
            cerr << "Erro ao abrir saida.txt para escrita\n";
            // `return 1;`: Retornar um valor diferente de 0 de `main` é a convenção para indicar que o programa terminou com um erro.
            return 1;
        }
        // A sintaxe para escrever no arquivo é idêntica à do `cout`.
        out << "Linha 1" << '\n';
        out << "Linha 2" << '\n';
    } // `out` sai do escopo aqui, seu destrutor é chamado, e o arquivo "saida.txt" é fechado.

    // 2) Ler o mesmo arquivo linha a linha
    {
        // ifstream: (Input File Stream) Classe para ler de arquivos. Pense nela como o `cin` para arquivos.
        ifstream in("saida.txt");
        if (!in) { cerr << "Erro ao abrir saida.txt para leitura\n"; return 1; }

        string linha;
        cout << "Conteudo de saida.txt:" << endl;
        // O idioma `while (getline(in, linha))` é a forma padrão de ler um arquivo texto linha por linha.
        // - `getline(in, linha)` tenta ler uma linha do fluxo `in` e armazená-la em `linha`.
        // - Se a leitura for bem-sucedida, a expressão retorna um estado que é avaliado como `true`, e o laço continua.
        // - Ao atingir o final do arquivo, `getline` falha, a expressão é avaliada como `false`, e o laço termina.
        while (getline(in, linha)) {
            cout << linha << endl;
        }
    } // `in` sai do escopo, e o arquivo é fechado automaticamente.

    // 3) CSV simples: "nome,idade"
    {
        ofstream csv("dados.csv"); // Cria e abre o arquivo dados.csv para escrita.
        csv << "nome,idade" << '\n'; // Escreve o cabeçalho.
        csv << "Ana,30" << '\n';
        csv << "Bruno,25" << '\n';
    }
    {
        ifstream csv("dados.csv");
        string linha;
        // vector<pair<string,int>>: Vamos usar um vetor de pares para armazenar os dados.
        // - `pair`: É um template de struct da STL que armazena exatamente dois valores. É útil para agrupar dados simples.
        //   O primeiro valor é acessado com `.first` e o segundo com `.second`.
        vector<pair<string, int>> pessoas;

        getline(csv, linha); // Lê a primeira linha (cabeçalho) para descartá-la.

        while (getline(csv, linha)) {
            // auto: Uma palavra-chave que instrui o compilador a deduzir o tipo da variável automaticamente a partir do seu inicializador.
            // Aqui, `linha.find(',')` retorna um `string::size_type`, e `auto` evita que tenhamos que escrever esse tipo longo.
            auto pos = linha.find(',');

            // string::npos: É um valor estático especial retornado por `find` quando o caractere (ou substring) não é encontrado.
            if (pos == string::npos) continue; // Pula linhas malformadas sem vírgula.

            // substr(posicao_inicial, comprimento): Método de `string` que extrai uma parte da string.
            string nome = linha.substr(0, pos); // Extrai de 0 até a posição da vírgula.

            // stoi: (String to Integer) Função global que converte uma `string` para um `int`.
            int idade = stoi(linha.substr(pos + 1)); // Extrai da posição APÓS a vírgula até o fim.

            pessoas.push_back({ nome, idade }); // Cria um `pair` na hora e o adiciona ao vetor.
        }
        cout << "\nLidos do CSV:" << endl;
        // `for (auto &p : pessoas)`: Usar `auto&` (referência) no range-based for é mais eficiente
        // pois evita copiar cada `pair` do vetor a cada iteração.
        for (auto& p : pessoas) cout << p.first << " tem " << p.second << " anos\n";
    }

    system("pause");
    return 0;
}