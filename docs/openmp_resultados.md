# Comparacao de schedules OpenMP

Medianas dos tempos em segundos; quatro threads, cinco repeticoes apos aquecimento.

| N | Schedule | Chunk | Mediana (s) |
|---:|---|---:|---:|
| 500 | dynamic | 1 | 0.018614 |
| 500 | dynamic | 64 | 0.018529 |
| 500 | guided | 1 | 0.018558 |
| 500 | static | 1 | 0.019528 |
| 1000 | dynamic | 1 | 0.074278 |
| 1000 | dynamic | 64 | 0.074627 |
| 1000 | guided | 1 | 0.070429 |
| 1000 | static | 1 | 0.070021 |
| 2000 | dynamic | 1 | 0.285937 |
| 2000 | dynamic | 64 | 0.284429 |
| 2000 | guided | 1 | 0.286621 |
| 2000 | static | 1 | 0.289076 |

Interprete os resultados junto de [docs/openmp.md](openmp.md): o custo por pixel e uniforme,
entao static deve empatar ou ganhar. Dynamic e guided acrescentam custo de despacho e sincronizacao;
sem desbalanceamento suficiente para compensar esse custo, tendem a ser mais lentos.
