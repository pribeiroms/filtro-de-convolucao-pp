# Relatório — Filtro de Convolução

Projeto da disciplina Programação Paralela (CCO085), IESB 2026/2 — Problema nº 4.
Integrantes: Aluisio Veloso Silva Carvalho, Miguel Ferreira Pedroso e Paula Ribeiro Moreira de Souza.

## Qual é o problema

O filtro de convolução 2D sequencial fornecido pelo professor é lento para imagens grandes, e precisamos acelerá-lo sem mudar o resultado. O custo cresce com P × N²: para N = 2000 e P = 30 são cerca de 120 milhões de atualizações de pixel, cada uma lendo 9 valores. Todo esse trabalho roda em um único núcleo, enquanto a máquina do laboratório tem vários.

## O que o programa faz

O programa aplica P passes de um filtro de média 3×3 (blur) sobre uma imagem sintética de N × N pixels. Ele recebe dois argumentos, N e P:

1. Gera a imagem de entrada.
2. Em cada passe, cada pixel interno recebe a média dos seus 9 vizinhos. As bordas não são recalculadas.
3. Ao final, soma toda a matriz e imprime um checksum.

O checksum é o critério de corretude: as versões paralelas têm de imprimir o mesmo valor do sequencial. A lógica de cálculo e a entrada de `sequencial/04_convolucao.cpp` não podem ser alteradas.

## Como resolvemos

Paralelizamos o laço de convolução, que o perfilamento (`04_convolucao_instrumentado.cpp`) apontou como hotspot, em duas versões. Cada passe lê uma matriz e escreve em outra, e as duas trocam de papel no passe seguinte; por isso nenhum pixel é lido depois de ter sido sobrescrito.

| | OpenMP (`openmp/`) | MPI (`mpi/`) |
| --- | --- | --- |
| Modelo | Threads, memória compartilhada | Processos, memória distribuída |
| Divisão do trabalho | `parallel for` sobre as linhas da imagem | Cada processo recebe um bloco de linhas |
| Dados entre vizinhos | Lidos direto da matriz compartilhada | Troca de halo: 1 linha com cada vizinho por passe |
| Checksum | `reduction(+)` | `MPI_Reduce` no processo 0 |
| Execução | `OMP_NUM_THREADS=8 ./bin/convolucao_omp 2000 30` | `mpirun -np 4 ./bin/convolucao_mpi 2000 30` |

Validamos cada versão contra o gabarito do sequencial (N = 500, 1000 e 2000, P = 30) e medimos speedup e eficiência com `scripts/medir.sh`.

## Por que resolvemos assim

- **O problema se divide bem.** Dentro de um passe, cada pixel depende só da matriz do passe anterior. As linhas podem ser calculadas em qualquer ordem e por qualquer núcleo.
- **Duas matrizes evitam condição de corrida.** Como ninguém escreve onde outro lê, não são necessários locks nem seções críticas.
- **Dividir por linhas reduz a comunicação.** No MPI, cada processo troca só 2 linhas de N pixels por passe, contra cerca de N² / p pixels de cálculo local.
- **As duas versões cobrem os dois cenários.** OpenMP aproveita os núcleos de uma máquina com pouca mudança no código. MPI permite escalar para várias máquinas.
- **O checksum garante a mesma saída.** Comparar com o gabarito comprova que a paralelização não alterou o resultado.

Em aberto: os números de speedup e eficiência entram aqui quando as medições em `dados/medicoes/` estiverem prontas.
