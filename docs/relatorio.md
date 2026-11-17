# Relatório — Filtro de Convolução

Projeto da disciplina Programação Paralela (CCO085), IESB 2026/2 — Problema nº 4.
Integrantes: Aluisio Veloso Silva Carvalho, Miguel Ferreira Pedroso e Paula Ribeiro Moreira de Souza.

## Qual é o problema

O filtro de convolução 2D sequencial fornecido pelo professor é lento para imagens grandes, e precisamos acelerá-lo sem mudar o resultado. O custo cresce com P × N²: para N = 2000 e P = 30 são cerca de 120 milhões de atualizações de pixel, cada uma lendo 9 valores. Todo esse trabalho roda em um único núcleo, enquanto a máquina de teste tem oito.

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
| Checksum | Soma serial, fora da região paralela | `MPI_Gatherv` e soma serial no processo 0 |
| Execução | `OMP_NUM_THREADS=8 ./bin/convolucao_omp 2000 30` | `mpirun -np 4 ./bin/convolucao_mpi 2000 30` |

Nas duas versões o checksum é somado em série, de propósito. A soma de floats não é associativa: mudar a ordem das parcelas muda os últimos dígitos do resultado. Somando em série, na mesma ordem do sequencial, o checksum bate por igualdade exata, sem precisar de tolerância numérica. O ganho que uma redução paralela daria é irrelevante — a soma final é uma passada sobre a matriz, contra as 30 passadas do filtro.

Validamos as duas versões com `scripts/validar.sh`: 63 configurações, comparando o checksum caractere por caractere contra o gabarito em `dados/gabarito/`. Todas passaram. As medições de tempo saem de `scripts/medir.sh` (baseline sequencial), `scripts/medir_openmp.sh`, `scripts/medir_mpi.sh` e `scripts/medir_fraca.sh`, com os resultados consolidados em `dados/medicoes/resumo.csv`.

## Por que resolvemos assim

- **O problema se divide bem.** Dentro de um passe, cada pixel depende só da matriz do passe anterior. As linhas podem ser calculadas em qualquer ordem e por qualquer núcleo.
- **Duas matrizes evitam condição de corrida.** Como ninguém escreve onde outro lê, não são necessários locks nem seções críticas.
- **Dividir por linhas reduz a comunicação.** No MPI, cada processo troca só 2 linhas de N pixels por passe, contra cerca de N² / p pixels de cálculo local.
- **As duas versões cobrem os dois cenários.** OpenMP aproveita os núcleos de uma máquina com pouca mudança no código. MPI permite escalar para várias máquinas.
- **O checksum garante a mesma saída.** Comparar com o gabarito comprova que a paralelização não alterou o resultado.

## Resultados

Speedup em N = 2000 e P = 30, mediana de 5 execuções, contra o baseline sequencial de 0,330327 s:

| Unidades | OpenMP | MPI | Amdahl |
| --- | --- | --- | --- |
| 2 | 1,62× | 1,63× | 1,97× |
| 4 | 3,08× | 2,90× | 3,85× |
| 8 | 3,75× | 3,16× | 7,32× |
| 16 | 4,00× | 4,12× | 13,36× |

Com 4 unidades as duas versões ficam perto do previsto. A partir daí descolam: em 8 unidades Amdahl prevê 7,32× e o medido é menos da metade disso. O padrão mais claro não é o speedup parar de crescer, e sim **a eficiência cair pela metade entre 4 e 8 unidades** — de 77,1% para 46,9% no OpenMP e de 72,5% para 39,5% no MPI.

A causa não é a implementação. OpenMP e MPI usam mecanismos completamente diferentes — threads com memória compartilhada contra processos que trocam mensagens — e perdem eficiência no mesmo ponto. A escalabilidade fraca cai no mesmo lugar e com valores quase idênticos entre as duas (45,3% e 45,2% com 8 unidades). O que as duas compartilham é a memória: os dois buffers de N = 2000 somam 32 MB, acima dos 16 MiB de cache L3 da máquina, então cada passe tem de buscar os dados na RAM. A banda de memória satura antes dos núcleos.

A análise completa, com os dados de dispersão, escalabilidade fraca e comparação de políticas de escalonamento, está em `docs/analise-experimental.md`.