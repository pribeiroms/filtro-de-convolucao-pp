# Comparacao de schedules OpenMP

Medianas dos tempos em segundos; quatro threads, cinco repeticoes apos aquecimento.

| N | Schedule | Chunk | Mediana (s) |
|---:|---|---:|---:|
| 500 | dynamic | 1 | 0.007637 |
| 500 | dynamic | 64 | 0.006362 |
| 500 | guided | 1 | 0.005818 |
| 500 | static | 0 | 0.005803 |
| 500 | static | 1 | 0.007855 |
| 1000 | dynamic | 1 | 0.022120 |
| 1000 | dynamic | 64 | 0.022764 |
| 1000 | guided | 1 | 0.021308 |
| 1000 | static | 0 | 0.022526 |
| 1000 | static | 1 | 0.023012 |
| 2000 | dynamic | 1 | 0.100409 |
| 2000 | dynamic | 64 | 0.095699 |
| 2000 | guided | 1 | 0.094194 |
| 2000 | static | 0 | 0.094812 |
| 2000 | static | 1 | 0.101273 |

Interprete os resultados junto de [docs/openmp.md](openmp.md) e da secao 5 de
[docs/analise-experimental.md](analise-experimental.md). O custo por pixel e uniforme,
entao nao ha desbalanceamento para as politicas dinamicas corrigirem. O que separa as
politicas neste problema e o tamanho do pedaco: com chunk pequeno o static perde a
localidade dos blocos contiguos, que e sua principal vantagem, e o guided -- que comeca
com pedacos grandes e contiguos -- leva vantagem nos tres tamanhos medidos.
