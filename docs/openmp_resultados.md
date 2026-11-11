# Comparacao de schedules OpenMP

Medianas dos tempos em segundos; quatro threads, cinco repeticoes apos aquecimento.

| N | Schedule | Chunk | Mediana (s) |
|---:|---|---:|---:|
| 500 | dynamic | 1 | 0.005960 |
| 500 | dynamic | 64 | 0.006255 |
| 500 | guided | 1 | 0.005891 |
| 500 | static | 1 | 0.007226 |
| 1000 | dynamic | 1 | 0.021081 |
| 1000 | dynamic | 64 | 0.021085 |
| 1000 | guided | 1 | 0.020625 |
| 1000 | static | 1 | 0.021937 |
| 2000 | dynamic | 1 | 0.093663 |
| 2000 | dynamic | 64 | 0.107885 |
| 2000 | guided | 1 | 0.084367 |
| 2000 | static | 1 | 0.086970 |

Interprete os resultados junto de [docs/openmp.md](openmp.md): o custo por pixel e uniforme,
entao static deve empatar ou ganhar. Dynamic e guided acrescentam custo de despacho e sincronizacao;
sem desbalanceamento suficiente para compensar esse custo, tendem a ser mais lentos.
