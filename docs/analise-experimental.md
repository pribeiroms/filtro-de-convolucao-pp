# Análise experimental

Comparação das curvas medidas de OpenMP e MPI com o limite teórico da Lei
de Amdahl, conforme a Issue #23.

Medição de 11/11/2026, na máquina de referência. Protocolo em
[protocolo-medicao.md](protocolo-medicao.md): 6 execuções por configuração,
a primeira descartada, mediana das 5 válidas. 165 execuções válidas, todas
com checksum idêntico ao gabarito.

Dados brutos em [../dados/medicoes/](../dados/medicoes/).

---

## 1. Baseline

Medianas do código sequencial, denominador de todos os speedups:

| N | Mediana |
|---|---|
| 500 | 0,018378 s |
| 1000 | 0,074939 s |
| 2000 | 0,287750 s |

---

## 2. Escalabilidade forte

Tamanho fixo, número crescente de unidades.

### Speedup

| Unidades | OpenMP N=500 | OpenMP N=1000 | OpenMP N=2000 | MPI N=500 | MPI N=1000 | MPI N=2000 | Amdahl |
|---|---|---|---|---|---|---|---|
| 1 | 0,94× | 0,93× | 0,89× | 0,83× | 0,88× | 0,83× | 1,00× |
| 2 | 1,71× | 1,75× | 1,75× | 1,51× | 1,69× | 1,59× | 1,97× |
| 4 | 3,19× | 3,29× | 3,32× | 2,93× | 3,03× | 2,96× | 3,85× |
| 8 | 3,24× | 3,42× | 3,44× | 3,67× | 3,21× | 3,00× | 7,32× |
| 16 | 2,31× | 2,99× | 5,58× | 4,10× | 4,60× | 3,71× | 13,36× |

### Eficiência

| Unidades | OpenMP N=2000 | MPI N=2000 | Amdahl |
|---|---|---|---|
| 1 | 88,9% | 83,1% | 100% |
| 2 | 87,7% | 79,6% | 98,7% |
| 4 | 83,1% | 74,0% | 96,2% |
| 8 | 43,0% | 37,5% | 91,5% |
| 16 | 34,9% | 23,2% | 83,5% |

---

## 3. O afastamento em relação a Amdahl

A curva medida acompanha a teórica até 4 unidades e se separa dela a partir
daí. Em N = 2000 com 8 unidades, Amdahl prevê 7,32× e o medido é 3,44× no
OpenMP e 3,00× no MPI — menos da metade.

Quatro fatores explicam a diferença, em ordem de peso.

### 3.1 Largura de banda de memória

É o fator dominante, e o que explica o platô entre 4 e 8 unidades.

O filtro lê 9 valores e escreve 1 por pixel, fazendo 8 somas e 1 divisão.
São cerca de 40 bytes movidos para 9 operações aritméticas — menos de 0,25
operação por byte. Trabalho desse tipo não é limitado pelo processador, e
sim pelo caminho até a memória.

Os dois buffers de N = 2000 somam 32 MB, acima dos 16 MiB de cache L3 da
máquina. A cada passe a matriz inteira é percorrida, então os dados vêm da
memória principal. Um núcleo sozinho não satura o barramento; quatro, numa
máquina de canal duplo, costumam saturar. A partir daí, acrescentar núcleos
não traz mais banda — apenas mais concorrentes pela mesma.

A evidência de que a causa é o hardware, e não a implementação: **o platô
aparece igual nas duas versões**. Em N = 2000, de 4 para 8 unidades, o
OpenMP vai de 3,32× para 3,44× e o MPI de 2,96× para 3,00×. São
implementações com mecanismos completamente diferentes — threads com
memória compartilhada contra processos com troca de mensagens — e as duas
param no mesmo lugar.

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
comportamento errático nessa faixa: em N = 500 e N = 1000 o OpenMP **piora**
de 8 para 16 threads (3,24× → 2,31× e 3,42× → 2,99×), e a dispersão entre
execuções sobe de menos de 1% para 54%.

### 3.3 Barreiras entre passes

Os passes são sequencialmente dependentes: o passe `p+1` só começa quando o
passe `p` termina por completo. São 30 barreiras por execução, e em cada uma
todas as unidades esperam a mais lenta.

O custo relativo depende do tamanho. Cada passe leva cerca de 613 µs em
N = 500 e 9 592 µs em N = 2000, de modo que uma barreira de poucos
microssegundos pesa mais de dez vezes mais na imagem pequena. É o que se vê
na faixa de 16 unidades, onde N = 500 tem o pior desempenho relativo dos
três tamanhos.

### 3.4 Comunicação, só no MPI

A versão MPI paga o que a fórmula não prevê: a troca de halo a cada passe e
o `MPI_Gatherv` final. O gather custa entre 0,010 s e 0,015 s em N = 2000
qualquer que seja o número de processos, e é por isso que o MPI com 1
processo fica 17% mais lento que o sequencial.

É também a razão de o MPI ficar sistematicamente abaixo do OpenMP em
N = 2000: mesma limitação de banda, mais o custo de comunicação. Detalhe em
[mpi.md](mpi.md), seção 5.

---

## 4. O teto real desta máquina

| Limite | Valor | Observação |
|---|---|---|
| Amdahl, unidades ilimitadas | 75,76× | Só considera a fração sequencial |
| Amdahl, 16 unidades | 13,36× | Pressupõe 16 processadores independentes |
| Amdahl, 8 unidades | 7,32× | Pressupõe que os 8 núcleos escalem sem disputa |
| Medido, melhor caso | 5,58× | OpenMP, N = 2000, 16 threads |

A diferença entre 7,32× e o medido não é falha da paralelização. É o ponto
em que a característica do problema — muitos dados, pouca aritmética —
encontra a característica da máquina.

Um problema com mais operações por byte, como multiplicação de matrizes,
escalaria muito mais perto da curva teórica no mesmo hardware.

---

## 5. Comparação entre as políticas de escalonamento

Dados em
[../dados/medicoes/openmp_schedules.csv](../dados/medicoes/openmp_schedules.csv),
medidos com 4 threads.

A diferença entre `static`, `dynamic` e `guided` é pequena neste problema, e
a razão é estrutural: o custo de processar cada linha é idêntico ao de
qualquer outra — mesmo número de pixels, mesmo número de operações por
pixel, sem desvios dependentes dos dados. Não há desbalanceamento para
nenhuma política corrigir.

O `static` é a escolha adequada porque distribui as iterações sem consultar
nada em tempo de execução. O `dynamic` mantém um contador compartilhado que
as threads disputam a cada pedaço, custo que só se paga quando as iterações
têm duração variável.

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
gabarito da Issue #3, então o script calcula a referência de corretude
rodando o sequencial antes de cada configuração. Todas as execuções
conferiram.

### A série valida a complexidade

Antes de olhar o paralelo, o sequencial sozinho confirma o `O(N²)`
deduzido na [análise do algoritmo](algoritmo.md). O trabalho total cresce
`n` vezes a cada linha da tabela, e o tempo acompanha:

| Unidades | N | Tempo do sequencial | Razão contra N=500 | Esperado |
|---|---|---|---|---|
| 1 | 500 | 0,018577 s | — | — |
| 2 | 707 | 0,038856 s | 2,09× | 2× |
| 4 | 1000 | 0,074750 s | 4,02× | 4× |
| 8 | 1414 | 0,148885 s | 8,01× | 8× |
| 16 | 2000 | 0,286628 s | 15,43× | 16× |

### Resultado

A eficiência fraca é `T(1, N₀) / T(n, N₀·√n)`. Se fosse 100%, o tempo
ficaria constante ao longo da tabela.

| Unidades | N | OpenMP | Eficiência | MPI | Eficiência |
|---|---|---|---|---|---|
| 1 | 500 | 0,020823 s | 100% | 0,021495 s | 100% |
| 2 | 707 | 0,022474 s | 92,7% | 0,023010 s | 93,4% |
| 4 | 1000 | 0,027927 s | 74,6% | 0,023567 s | 91,2% |
| 8 | 1414 | 0,044183 s | 47,1% | 0,044981 s | 47,8% |
| 16 | 2000 | 0,059330 s | 35,1% | 0,065323 s | 32,9% |

### Leitura

**Até 2 unidades o tempo quase não cresce**, com eficiência em torno de
93%. Dobrar o problema e dobrar as unidades mantém o tempo praticamente
igual, que é o comportamento ideal.

**Entre 4 e 8 unidades a eficiência cai pela metade**, de cerca de 90% para
47%, nas duas versões. É o mesmo ponto de inflexão da escalabilidade forte,
e pela mesma razão: dobrar as unidades não dobra a largura de banda
disponível, mas dobra o volume de dados a mover.

**Com 16 unidades fica em torno de um terço**, com as duas versões
praticamente empatadas.

### Ressalva sobre os pontos de 4 unidades

As medições com 4 unidades têm dispersão alta — 68% no OpenMP e 54% no MPI,
contra menos de 13% nas demais configurações. Houve interferência durante
essa janela da medição. O valor mínimo do OpenMP em 4 threads foi 0,022667 s,
que daria 91,9% de eficiência, alinhado com os 91,2% do MPI.

O ponto de 4 unidades, portanto, deve ser lido como aproximadamente 90% e
não como 74,6%. A conclusão não muda: a queda acentuada acontece entre 4 e 8
unidades.

### Comparação com a escalabilidade forte

| Unidades | Eficiência forte (N=2000) | Eficiência fraca |
|---|---|---|
| 2 | 87,7% | 92,7% |
| 4 | 83,1% | ~90% |
| 8 | 43,0% | 47,1% |
| 16 | 34,9% | 35,1% |

As duas curvas têm o mesmo formato e caem no mesmo ponto. Se o limite fosse
a fração sequencial do programa — o que a Lei de Amdahl mede —, a
escalabilidade fraca se sustentaria bem melhor que a forte, porque a parcela
paralelizável cresce junto com o problema. Elas caírem juntas é mais uma
evidência de que o gargalo é a largura de banda de memória, que não cresce
em nenhum dos dois cenários.