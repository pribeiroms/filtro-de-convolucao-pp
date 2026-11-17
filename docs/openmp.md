# Implementacao OpenMP (Issues #12–#15)

## Regiao paralela e ausencia de corrida

Na regiao `parallel for`, `a`, `b`, `N` e `lado` sao `shared`; `y` e `x` sao
privados implicitamente pelo OpenMP para as variaveis de iteracao, e `s`,
`dx` e `dy` sao automaticas locais a cada iteracao/thread. `P`, `p` e os
parametros do schedule sao lidos fora da regiao. O checksum `soma` e calculado
depois da regiao, serialmente. O temporario da soma do kernel nao e a mesma
variavel de `soma` do checksum.

Cada pixel interno de `b` e escrito por uma unica iteracao e so lemos `a`;
logo nao ha escrita concorrente nem leitura/escrita no mesmo buffer. `a.swap(b)`
ocorre depois da barreira implicita no fim de cada `parallel for`. Assim, o
pass seguinte so comeca depois de todas as escritas. Bordas nao sao escritas,
tal como no baseline, e seus valores sao carregados entre passes nos buffers.

Se os buffers fossem unificados, uma thread poderia sobrescrever um vizinho
antes de outra thread le-lo. Se o checksum fosse calculado dentro da regiao
sem sincronizacao, haveria corrida na variavel global de soma. Para um checksum
paralelo a escolha adequada seria `reduction(+:soma)`, pois cada thread acumula
localmente e combina ao final; `atomic` sincronizaria cada adicao e `critical`
serializaria todo o bloco. Nesta implementacao mantemos o checksum serial por
dois motivos: a soma de floats nao e associativa, e uma reducao paralela mudaria
a ordem das parcelas e os ultimos digitos do resultado, o que exigiria tolerancia
numerica na validacao em vez de comparacao exata; e a soma final e uma unica
passada sobre a matriz, contra as P passadas do filtro, entao esta fora do
hotspot.

## Escalonamento

O kernel faz as mesmas nove leituras e uma escrita em todo pixel interno. Nao
ha variacao significativa de custo entre linhas ou pixels, entao o desbalanceo
que `dynamic` e `guided` existem para corrigir nao aparece aqui — eles so pagam
o custo de despacho por bloco. A expectativa inicial era de que `static`
vencesse por isso.

O script `scripts/medir_openmp.sh` compara `static` com chunk 0 (padrao) e 1,
`dynamic` com chunks 1 e 64, e `guided`, em N = 500, 1000 e 2000 com 4 threads.
A tabela medida esta em `dados/medicoes/openmp_schedules.csv`. Medianas de 5
execucoes:

| Schedule | N = 500 | N = 1000 | N = 2000 |
|---|---|---|---|
| `static`, chunk 0 | 5,80 ms | 22,53 ms | 94,81 ms |
| `static`, chunk 1 | 7,86 ms | 23,01 ms | 101,27 ms |
| `dynamic`, chunk 1 | 7,64 ms | 22,12 ms | 100,41 ms |
| `dynamic`, chunk 64 | 6,36 ms | 22,76 ms | 95,70 ms |
| `guided` | 5,82 ms | 21,31 ms | 94,19 ms |

O resultado nao confirma a expectativa como ela foi formulada. O que os dados
sustentam e mais restrito:

- **`static,0` vence `static,1` nos tres tamanhos**, com diferencas de 2% a 26%.
  Esse e o unico efeito que aparece de forma consistente. A causa e localidade:
  com chunk 0 cada thread recebe um bloco contiguo de linhas e percorre memoria
  sequencial; com chunk 1 as linhas sao intercaladas entre as threads, cada
  thread salta de N em N posicoes e as linhas de cache carregadas por uma thread
  sao parcialmente desperdicadas.
- **`static,1` e o pior nos tres tamanhos.** O ganho de balanceamento que um
  chunk pequeno daria nao existe aqui, e o custo de localidade fica.
- **As diferencas entre `static,0`, `guided` e `dynamic,64` sao de 2% a 5%**,
  abaixo da dispersao medida entre execucoes da mesma configuracao. Nao da para
  ordenar essas tres politicas com esses dados; o que se pode afirmar e que
  nenhuma delas perde de forma relevante para as outras.

A conclusao pratica: o que importa neste kernel nao e a politica, e o tamanho
do chunk. Chunk pequeno custa caro por localidade, e nenhuma politica compensa
isso. A versao mantem `schedule(runtime)` e `omp_set_schedule`, com `static` e
chunk 0 como padrao.

## Escalabilidade

O mesmo script mede threads 1, 2, 4, 8 e 16 (ou valor adicional em
`THREADS_EXTRA`) e valida cada checksum contra o gabarito. O grafico de
speedup e gravado em `graficos/openmp_speedup.svg`.

A saturacao medida aparece entre 4 e 8 threads, nao em 16 como a contagem de
nucleos sugeriria. Em N = 2000 a eficiencia cai de 77,1% com 4 threads para
46,9% com 8 — pela metade. O motivo principal e banda de memoria: os dois
buffers de N = 2000 somam 32 MB, acima dos 16 MiB de L3 da maquina, entao cada
passe busca os dados na RAM, e acrescentar threads nao acrescenta banda.
Acima de 8 threads entra tambem o SMT/hyper-threading, em que duas threads
logicas dividem as unidades de execucao de um nucleo fisico. False sharing nos
limites de cache entre blocos de linhas existe, mas com chunk 0 fica restrito
as fronteiras entre blocos e e pequeno diante do custo de acesso aos buffers.

A evidencia de que o limite e a memoria e nao a implementacao: a versao MPI,
com mecanismo completamente diferente, perde eficiencia no mesmo ponto (72,5%
para 39,5% entre 4 e 8 processos). A analise completa esta em
`docs/analise-experimental.md`.

Os CSVs e o grafico devem ser interpretados usando a maquina/ambiente anotados
junto da execucao, pois nao sao transferiveis entre maquinas.

## Reproducao e protocolo

`make openmp` compila com `-fopenmp`; numero de threads pode vir de
`OMP_NUM_THREADS` ou do terceiro argumento. Os argumentos opcionais 4 e 5
selecionam schedule (`static`, `dynamic`, `guided`) e chunk. O script faz uma
execucao de aquecimento e cinco execucoes registradas por configuracao,
seguindo `docs/protocolo-medicao.md`.

O CSV de escalabilidade inclui tempos sequenciais e paralelos e o checksum em
cada linha, mas as linhas sequenciais dele nao sao usadas como baseline. O
baseline unico do projeto e `dados/medicoes/sequencial.csv`, gerado por
`scripts/medir.sh`, e `speedup = mediana desse arquivo / mediana OpenMP`. Ter
mais de uma fonte de baseline foi o que gerou valores divergentes entre
documentos e graficos antes dessa padronizacao.