# Análise experimental

Comparação das curvas medidas de OpenMP e MPI com o limite teórico da Lei
de Amdahl, conforme a Issue #23.

Medição de 11/11/2026, na máquina de referência. Protocolo em
[protocolo-medicao.md](protocolo-medicao.md): 6 execuções por configuração,
a primeira descartada, mediana das 5 válidas. Todas as execuções produziram
checksum idêntico ao de referência.

Dados brutos em [../dados/medicoes/](../dados/medicoes/).

A seção 7 trata da variabilidade entre sessões de medição e de quais
conclusões se reproduzem.

---

## 1. Baseline

Medianas do código sequencial, denominador de todos os speedups:

| N | Mediana | Dispersão |
|---|---|---|
| 500 | 0,019070 s | 7,6% |
| 1000 | 0,074185 s | 4,2% |
| 2000 | 0,330327 s | 2,9% |

Fonte única: [../dados/medicoes/sequencial.csv](../dados/medicoes/sequencial.csv).
Detalhes em [baseline.md](baseline.md).

---

## 2. Escalabilidade forte

Tamanho fixo, número crescente de unidades.

### Speedup

| Unidades | OpenMP N=500 | OpenMP N=1000 | OpenMP N=2000 | MPI N=500 | MPI N=1000 | MPI N=2000 | Amdahl |
|---|---|---|---|---|---|---|---|
| 1 | 0,93× | 0,89× | 0,91× | 0,88× | 0,85× | 0,84× | 1,00× |
| 2 | 1,61× | 1,69× | 1,62× | 1,65× | 1,54× | 1,63× | 1,97× |
| 4 | 2,60× | 3,19× | 3,08× | 2,90× | 2,73× | 2,90× | 3,85× |
| 8 | 4,07× | 3,40× | 3,75× | 3,40× | 3,12× | 3,16× | 7,32× |
| 16 | 1,82× | 4,64× | 4,00× | 3,24× | 4,00× | 4,12× | 13,36× |

### Eficiência em N = 2000

| Unidades | OpenMP | MPI | Amdahl |
|---|---|---|---|
| 1 | 90,9% | 84,3% | 100% |
| 2 | 80,8% | 81,3% | 98,7% |
| 4 | 77,1% | 72,5% | 96,2% |
| 8 | 46,9% | 39,5% | 91,5% |
| 16 | 25,0% | 25,8% | 83,5% |

### Dispersão

As configurações com 8 e 16 unidades têm dispersão alta — em N = 500 com 16
threads, 36%; em N = 500 com 16 processos, 143%. As linhas de 1 e 2 unidades
ficam abaixo de 8%.

As conclusões deste documento se apoiam em N = 2000, o tamanho com menor
dispersão, e nas tendências que se repetem nas três sessões de medição
descritas na seção 7, não em valores isolados.

---

## 3. O afastamento em relação a Amdahl

A curva medida acompanha a teórica até 4 unidades e se separa dela a partir
daí. Em N = 2000 com 8 unidades, Amdahl prevê 7,32× e o medido é 3,75× no
OpenMP e 3,16× no MPI — menos da metade.

O padrão mais claro não é o speedup parar de crescer, e sim **a eficiência
cair pela metade entre 4 e 8 unidades**: de 77,1% para 46,9% no OpenMP e de
72,5% para 39,5% no MPI. Cada unidade acrescentada a partir daí rende cerca
de metade do que rendia antes.

Quatro fatores explicam a diferença, em ordem de peso.

### 3.1 Largura de banda de memória

É o fator dominante.

O filtro lê 9 valores e escreve 1 por pixel, fazendo 8 somas e 1 divisão.
São cerca de 40 bytes movidos para 9 operações aritméticas — menos de 0,25
operação por byte. Trabalho desse tipo não é limitado pelo processador, e
sim pelo caminho até a memória.

Os dois buffers de N = 2000 somam 32 MB, acima dos 16 MiB de cache L3 da
máquina. A cada passe a matriz inteira é percorrida, então os dados vêm da
memória principal. Um núcleo sozinho não satura o barramento; quatro, numa
máquina de canal duplo, costumam saturar. A partir daí, acrescentar núcleos
não traz mais banda — apenas mais concorrentes pela mesma.

A evidência de que a causa é o hardware, e não a implementação: **a queda de
eficiência acontece no mesmo ponto nas duas versões**. Em N = 2000, de 4
para 8 unidades, o OpenMP cai de 77,1% para 46,9% e o MPI de 72,5% para
39,5%. São implementações com mecanismos completamente diferentes — threads
com memória compartilhada contra processos com troca de mensagens — e as
duas perdem rendimento no mesmo lugar.

Amdahl não modela isso. A fórmula supõe que a parcela paralelizável se
divide perfeitamente entre as unidades, o que pressupõe recursos
independentes. Largura de banda é um recurso compartilhado.

### 3.2 CPUs lógicas não são núcleos

A máquina tem 8 núcleos físicos e 16 CPUs lógicas. Os 13,36× que Amdahl
prevê para 16 unidades pressupõem 16 processadores independentes, que não
existem: acima de 8, duas threads dividem as unidades de execução de um
mesmo núcleo.

Em trabalho limitado por memória o ganho do hyperthreading tende a zero,
porque a thread extra não traz banda adicional. Os dados confirmam o
comportamento errático nessa faixa: em N = 500 o OpenMP **piora** de 8 para
16 threads (4,07× → 1,82×) e o MPI também (3,40× → 3,24×), com dispersões de
36% e 143%.

### 3.3 Barreiras entre passes

Os passes são sequencialmente dependentes: o passe `p+1` só começa quando o
passe `p` termina por completo. São 30 barreiras por execução, e em cada uma
todas as unidades esperam a mais lenta.

O custo relativo depende do tamanho. Cada passe leva cerca de 636 µs em
N = 500 e 11 011 µs em N = 2000, de modo que uma barreira de poucos
microssegundos pesa mais de dezessete vezes mais na imagem pequena. É o que
se vê na faixa de 16 unidades, onde N = 500 tem o pior desempenho relativo
dos três tamanhos.

### 3.4 Comunicação, só no MPI

A versão MPI paga o que a fórmula não prevê: a troca de halo a cada passe e
o `MPI_Gatherv` final. O gather custa entre 0,012 s e 0,016 s em N = 2000
qualquer que seja o número de processos, e é por isso que o MPI com 1
processo fica 16% mais lento que o sequencial.

Em N = 500 com 16 processos a comunicação chega a 82% do tempo total.
Detalhe em [mpi.md](mpi.md), seção 5.

---

## 4. O teto real desta máquina

| Limite | Valor | Observação |
|---|---|---|
| Amdahl, unidades ilimitadas | 75,76× | Só considera a fração sequencial |
| Amdahl, 16 unidades | 13,36× | Pressupõe 16 processadores independentes |
| Amdahl, 8 unidades | 7,32× | Pressupõe que os 8 núcleos escalem sem disputa |
| Medido, melhor caso | 4,64× | OpenMP, N = 1000, 16 threads |

A diferença entre 7,32× e o medido não é falha da paralelização. É o ponto
em que a característica do problema — muitos dados, pouca aritmética —
encontra a característica da máquina.

Um problema com mais operações por byte, como multiplicação de matrizes,
escalaria muito mais perto da curva teórica no mesmo hardware.

---

## 5. Comparação entre as políticas de escalonamento

Medição com 4 threads, mediana de 5 execuções após descarte de aquecimento.
Dados em
[../dados/medicoes/openmp_schedules.csv](../dados/medicoes/openmp_schedules.csv).

| N | `static,0` | `static,1` | `dynamic,1` | `dynamic,64` | `guided,1` |
|---|---|---|---|---|---|
| 500 | **0,005803 s** | 0,007855 s | 0,007637 s | 0,006362 s | 0,005818 s |
| 1000 | 0,022526 s | 0,023012 s | 0,022120 s | 0,022764 s | **0,021308 s** |
| 2000 | 0,094812 s | 0,101273 s | 0,100409 s | 0,095699 s | **0,094194 s** |

`static,0` usa o tamanho de pedaço padrão do runtime, que para a política
estática divide as linhas em blocos contíguos, um por thread. `static,1`
distribui linha a linha em rodízio.

### O que se sustenta

**O tamanho do pedaço importa mais que a política.** O `static,0` é mais
rápido que o `static,1` nos três tamanhos, com diferenças de 2% a 26%. A
mesma política, mudando só o pedaço, muda o resultado — e muda na direção
prevista pela localidade de cache.

A vantagem do `static` neste problema não é o balanceamento, é a
**localidade**: com blocos contíguos, cada thread percorre uma faixa da
matriz que cabe no cache do seu núcleo. Com `chunk = 1` a thread 0 fica com
as linhas 1, 5, 9, 13… e passa a tocar a matriz inteira.

O `guided` fica empatado com o `static,0`, o que é coerente: ele começa com
pedaços grandes e contíguos, o mesmo padrão de acesso.

### O que não se sustenta

**Entre as demais políticas, a ordenação não se reproduz.** Numa sessão
anterior de medição o `guided` venceu nos três tamanhos; noutra o
`dynamic,64` venceu em N = 500 e o `static,1` em N = 2000. As diferenças
entre `dynamic`, `guided` e `static,0` são da ordem de 2% a 5%, abaixo da
dispersão entre execuções.

A conclusão defensável é que, **fora do efeito do tamanho do pedaço, as
políticas são equivalentes neste problema**. Era o esperado: o custo por
linha é idêntico, sem desvios dependentes dos dados, de modo que não há
desbalanceamento para as políticas dinâmicas corrigirem.

### Consequência para as curvas de speedup

A medição de escalabilidade das seções 2 e 3 usou `static,1`, que estes
dados mostram não ser a melhor configuração. Em N = 2000 a diferença para o
`static,0` é de 6,4%, suficiente para deslocar os pontos da curva mas não
para alterar as conclusões: a queda de eficiência entre 4 e 8 unidades e a
distância para a curva de Amdahl permanecem de mesma ordem.

---

## 6. Escalabilidade fraca

Problema crescendo junto com o número de unidades, mantendo constante a
carga por unidade. Como o custo é `O(N²)`, o lado da imagem cresce com `√n`:

| Unidades | N | Pixels por unidade |
|---|---|---|
| 1 | 500 | 250 000 |
| 2 | 707 | 249 925 |
| 4 | 1000 | 250 000 |
| 8 | 1414 | 249 924 |
| 16 | 2000 | 250 000 |

Dados em
[../dados/medicoes/escalabilidade_fraca.csv](../dados/medicoes/escalabilidade_fraca.csv),
gerados por `scripts/medir_fraca.sh`. Os tamanhos 707 e 1414 não estão no
gabarito da Issue #3, que cobre apenas 500, 1000 e 2000, então o script
calcula a referência de corretude rodando o sequencial antes de cada
configuração. Todas as execuções conferiram.

### A série valida a complexidade

Antes de olhar o paralelo, o sequencial sozinho confirma o `O(N²)`
deduzido na [análise do algoritmo](algoritmo.md). O trabalho total cresce
`n` vezes a cada linha da tabela, e o tempo acompanha:

| Unidades | N | Tempo do sequencial | Razão contra N=500 | Esperado |
|---|---|---|---|---|
| 1 | 500 | 0,019411 s | — | — |
| 2 | 707 | 0,038196 s | 1,97× | 2× |
| 4 | 1000 | 0,077907 s | 4,01× | 4× |
| 8 | 1414 | 0,161283 s | 8,31× | 8× |
| 16 | 2000 | 0,336281 s | 17,32× | 16× |

### Resultado

A eficiência fraca é `T(1, N₀) / T(n, N₀·√n)`. Se fosse 100%, o tempo
ficaria constante ao longo da tabela.

| Unidades | N | OpenMP | Eficiência | MPI | Eficiência |
|---|---|---|---|---|---|
| 1 | 500 | 0,020700 s | 100% | 0,022156 s | 100% |
| 2 | 707 | 0,022351 s | 92,6% | 0,023859 s | 92,9% |
| 4 | 1000 | 0,023708 s | 87,3% | 0,026219 s | 84,5% |
| 8 | 1414 | 0,045653 s | 45,3% | 0,049068 s | 45,2% |
| 16 | 2000 | 0,065621 s | 31,5% | 0,109561 s | 20,2% |

### Leitura

**Até 4 unidades o tempo cresce pouco**, com eficiência entre 84% e 93%.
Dobrar o problema e dobrar as unidades mantém o tempo quase igual, que é o
comportamento ideal.

**Entre 4 e 8 unidades a eficiência cai pela metade**, de cerca de 85% para
45%, nas duas versões, com valores quase idênticos entre elas (45,3% e
45,2%). É o mesmo ponto de inflexão da escalabilidade forte, e pela mesma
razão: dobrar as unidades não dobra a largura de banda disponível, mas
dobra o volume de dados a mover.

**Com 16 unidades o MPI cai mais que o OpenMP** (20,2% contra 31,5%), o que
é consistente com o custo de comunicação, que o OpenMP não tem.

### Comparação com a escalabilidade forte

| Unidades | Eficiência forte (N=2000, OpenMP) | Eficiência fraca (OpenMP) |
|---|---|---|
| 2 | 80,8% | 92,6% |
| 4 | 77,1% | 87,3% |
| 8 | 46,9% | 45,3% |
| 16 | 25,0% | 31,5% |

As duas curvas caem no mesmo ponto. Se o limite fosse a fração sequencial do
programa — o que a Lei de Amdahl mede —, a escalabilidade fraca se
sustentaria bem melhor que a forte, porque a parcela paralelizável cresce
junto com o problema. Elas caírem juntas é mais uma evidência de que o
gargalo é a largura de banda de memória, que não cresce em nenhum dos dois
cenários.

---

## 7. Variabilidade entre sessões de medição

A bateria completa foi executada três vezes em 11/11/2026, na mesma máquina.
Os dados versionados são os da terceira. As três sessões permitem separar o
que é efeito real do que é variação do ambiente.

### Speedup em N = 2000 nas três sessões

| Unidades | OpenMP s1 | OpenMP s2 | OpenMP s3 | MPI s1 | MPI s2 | MPI s3 |
|---|---|---|---|---|---|---|
| 2 | 1,82× | 1,63× | 1,62× | 1,65× | 1,55× | 1,63× |
| 4 | 3,45× | 2,82× | 3,08× | 3,07× | 2,84× | 2,90× |
| 8 | 3,57× | 3,72× | 3,75× | 3,11× | 3,03× | 3,16× |
| 16 | 5,79× | 4,56× | 4,00× | 3,85× | 3,74× | 4,12× |

**O MPI é estável**: nenhum ponto varia mais de 10% entre sessões.

**O OpenMP varia mais**, principalmente em 2, 4 e 16 unidades. A primeira
sessão produziu valores que as outras duas não reproduziram.

### O que se reproduz nas três sessões

- Com 1 unidade, as duas versões são mais lentas que o sequencial.
- A eficiência fica entre 68% e 86% até 4 unidades.
- A eficiência cai para a faixa de 39% a 47% com 8 unidades, nas duas
  versões.
- Com 16 unidades o comportamento é irregular e a dispersão entre execuções
  cresce muito.
- Nenhuma configuração chega perto da curva de Amdahl a partir de 8
  unidades.

### O que não se reproduz

- O valor exato do speedup em cada ponto do OpenMP.
- Qual política de escalonamento é a mais rápida, fora o efeito do tamanho
  do pedaço.
- Se o speedup **para** de crescer entre 4 e 8 unidades, como ocorreu na
  primeira sessão, ou apenas cresce menos, como nas outras duas. O que se
  mantém é a queda de eficiência.

### Por que a variação existe

A máquina de medição é um notebook com sistema operacional de uso geral,
executando a medição dentro de um contêiner. Serviços do sistema, o próprio
Docker e a gestão térmica do processador competem por CPU de forma que não
se pode desligar por completo. O protocolo reduz esse efeito — aquecimento
descartado, mediana em vez de média, máquina sem outras cargas — mas não o
elimina.

Reportar as três sessões, em vez de apresentar uma como se fosse definitiva,
é o que permite distinguir conclusão de coincidência.