# Implementacao OpenMP (Issues #12–#15)

## Regiao paralela e ausencia de corrida

Na regiao `parallel for`, `a`, `b` e `N` sao `shared`; `y` e `x` sao
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
serializaria todo o bloco. Nesta implementacao mantemos o checksum serial para
preservar a rotina do baseline e nao paralelizamos trabalho fora do hotspot.

## Escalonamento

O kernel faz as mesmas nove leituras e uma escrita em todo pixel interno.
Assim, `static` deve empatar ou superar `dynamic`/`guided`: nao ha variacao
significativa de custo entre linhas/pixels que justifique distribuir trabalho
continuamente. `dynamic` paga sincronizacao e despacho por bloco; `guided`
reduz o tamanho do bloco gradualmente, mas tambem tem custo de despacho. O
script `scripts/medir_openmp.sh` compara `static`, `dynamic` com chunks 1 e 64,
e `guided`. A tabela de `dados/medicoes/openmp_schedules.csv` deve ser gerada
na maquina do laboratorio; nao registrar valores estimados como medicao.

## Escalabilidade

O mesmo script mede threads 1, 2, 4, 8 e 16 (ou valor adicional em
`THREADS_EXTRA`) e valida cada checksum contra o gabarito. O grafico de
speedup e gravado em `graficos/openmp_speedup.svg`. A saturacao esperada ocorre
quando os nucleos deixam de receber trabalho em paralelo suficiente: banda de
memoria limita o fluxo de vizinhos, e threads acima dos nucleos fisicos
compartilham unidades de execucao via SMT/hyper-threading. Escritas contiguas
podem causar false sharing nos limites de cache entre blocos de linhas, embora
seja esperado que esse custo seja menor que o do calculo e acesso aos buffers.
Os CSVs e o grafico devem ser interpretados usando a maquina/ambiente anotados
junto da execucao, pois nao sao transferiveis entre maquinas.

## Reproducao e protocolo

`make openmp` compila com `-fopenmp`; numero de threads pode vir de
`OMP_NUM_THREADS` ou do terceiro argumento. Os argumentos opcionais 4 e 5
selecionam schedule (`static`, `dynamic`, `guided`) e chunk. O script faz uma
execucao de aquecimento e cinco execucoes registradas por configuracao,
seguindo `docs/protocolo-medicao.md`. O CSV de escalabilidade inclui os tempos
sequenciais e paralelos e checksum em cada linha; `speedup = mediana sequencial
/ mediana OpenMP`.
