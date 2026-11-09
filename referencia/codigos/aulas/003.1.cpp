// Este trecho garante que só será incluído no código quando
// estivermos compilando no sistema operacional Windows.
// Em Linux/Mac, "_WIN32" não é definido, então o bloco será ignorado.
#ifdef _WIN32
#include <windows.h> // Biblioteca com funções específicas do Windows (API do Windows).
#endif

#include <iostream> // Biblioteca padrão para entrada e saída (cin, cout, etc.)

int main() {
    // Novamente, só será executado se o código estiver rodando no Windows.
    // Isso evita erros de compilação em outros sistemas operacionais.
#ifdef _WIN32
    // SetConsoleOutputCP define o "code page" (tabela de caracteres)
    // usado para exibir texto no console.
    // CP_UTF8 = 65001, ou seja, muda o console para aceitar UTF-8 na saída.
    SetConsoleOutputCP(CP_UTF8);

    // SetConsoleCP define o "code page" usado para ler entradas do usuário (cin).
    // Também é configurado para UTF-8, permitindo que o usuário digite acentos corretamente.
    SetConsoleCP(CP_UTF8);
#endif

    // A string "u8" indica que o literal está codificado em UTF-8.
    // Assim, os caracteres acentuados são interpretados corretamente.
    std::cout << u8"Oi, Rodrigo Gonçalves Pinto. Você tem 47 anos.\n";

    return 0; // Retorna 0 ao sistema operacional → execução bem-sucedida.
}
