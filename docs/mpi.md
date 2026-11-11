# Versão MPI

Paralelização do filtro em memória distribuída, conforme as Issues #16
(decomposição de domínio), #17 (troca de halo), #18 (operações coletivas),
#19 (overhead de comunicação) e #20 (speedup por número de processos).

Código: [../mpi/convolucao_mpi.cpp](../mpi/convolucao_mpi.cpp).

```bash
make mpi
mpirun -np 4 ./bin/convolucao_mpi 2000 30
```

Saída:

```
N=2000 passes=30  checksum=508325215.06  tempo=0.077238 s
processos=4  calculo=0.050734 s  halo=0.026767 s  gather=0.009396 s
```

A primeira linha tem o mesmo formato do sequencial, e por isso os scripts de
medição leem as duas versões do mesmo jeito. A segunda linha detalha o tempo
para a análise de comunicação (seção 5).

---

## 1. Decomposição de domínio (#16)

**Escolha: faixas de linhas (1D).** A imagem `N × N` é cortada em faixas
horizontais, e cada processo fica com uma faixa.

| Critério | Faixas (1D) | Blocos (2D) |
|---|---|---|
| Vizinhos por processo | até 2 (cima, baixo) | até 4 (ou 8 com cantos) |
| Dado de borda por processo | `2·N` floats | `≈ 4·N/√p` floats |
| Formato da mensagem | linha inteira, contígua na memória | coluna não contígua, exige tipo derivado ou cópia |
| Complexidade do código | baixa | alta |

A imagem é guardada linha por linha, então uma linha é um bloco contíguo e
pode ser enviada numa única mensagem, sem montar tipo derivado. O 2D só
compensa com muitos processos: com `p` processos, o halo 1D continua em
`2·N` por processo, e o 2D cai com `√p`. Na faixa medida (até 16 processos),
a diferença não paga a complexidade a mais.

**Divisão não exata.** Quando `N` não é múltiplo de `p`, os `N mod p`
primeiros processos recebem uma linha a mais:

```
base   = N / p
resto  = N % p
linhas = base + (rank < resto ? 1 : 0)
inicio = rank·base + min(rank, resto)
```

A diferença de carga entre processos fica em no máximo uma linha. Testado
com `N = 1001` e 3, 4 e 7 processos: o checksum confere com o sequencial.

**Sem distribuição de dados.** Cada processo gera só as suas linhas, com a
mesma fórmula do sequencial aplicada ao índice global do pixel. O resultado
é idêntico ao de gerar a imagem toda no processo 0 e espalhar com
`MPI_Scatterv`, sem gastar essa comunicação (ver seção 3).

---

## 2. Troca de halo (#17)

O filtro 3×3 de um pixel lê a linha de cima e a de baixo. Na primeira e na
última linha de uma faixa, essas linhas pertencem ao processo vizinho.

**Tamanho do halo: 1 linha em cima e 1 embaixo**, porque o raio do kernel é
`k/2 = 3/2 = 1`. Cada processo aloca `linhas + 2` linhas:

```
linha local 0            halo de cima   (cópia da última linha do vizinho de cima)
linhas locais 1..linhas  faixa do processo
linha local linhas+1     halo de baixo  (cópia da primeira linha do vizinho de baixo)
```

**A troca acontece a cada passe**, antes do cálculo: o passe anterior mudou
as linhas de borda do vizinho, e o halo antigo deixa de valer. Com 30
passes, são 30 trocas.

**`MPI_Sendrecv` em vez de `Send` + `Recv`.** Se todo processo fizesse
primeiro um `Send` bloqueante para o vizinho e só depois o `Recv`, todos
poderiam ficar parados esperando alguém receber: deadlock. O `Sendrecv`
envia e recebe na mesma chamada, e a biblioteca resolve a ordem. São duas
chamadas por passe:

1. envio a minha primeira linha para cima e recebo, de baixo, o halo de baixo;
2. envio a minha última linha para baixo e recebo, de cima, o halo de cima.

**Bordas da imagem.** O primeiro processo não tem vizinho em cima, e o
último não tem embaixo. Para eles o vizinho é `MPI_PROC_NULL`: enviar ou
receber dele não faz nada, e o mesmo código serve para todos. Esse halo
vazio nunca é lido, porque as linhas 0 e `N−1` da imagem não são
recalculadas, igual ao sequencial (o laço vai de 1 a `N−2`).

**Por que o resultado é idêntico, e não só próximo.** O sequencial usa dois
vetores e troca os dois a cada passe (`a.swap(b)`). As bordas nunca são
escritas em `b`, que começa zerado, e por isso alternam entre o valor
original e zero a cada passe. A versão MPI reproduz isso: `b` local também
começa zerado e as mesmas posições ficam sem escrita. A soma dos 9 vizinhos
segue a mesma ordem (`dy`, depois `dx`), então cada pixel dá exatamente o
mesmo `float`.

---

## 3. Operações coletivas (#18)

| Coletiva | Onde | Por quê |
|---|---|---|
| `MPI_Bcast` | parâmetros `N` e `P` | O padrão MPI só garante `argv` no processo 0. Ele lê e repassa a todos. |
| `MPI_Gatherv` | junta as faixas no processo 0 | As faixas podem ter tamanhos diferentes quando `N mod p ≠ 0`. O `Gather` comum exige o mesmo tamanho para todos; o `Gatherv` recebe contagens e deslocamentos por processo. |
| `MPI_Reduce` (`MPI_MAX`) | tempos de cálculo e de halo | O processo mais lento define o tempo total. Os dois tempos vão num só vetor, numa só chamada. |
| `MPI_Barrier` | antes de iniciar o cronômetro | Todos começam a medição juntos. |

**Por que não `MPI_Scatterv`.** A entrada é sintética e cada pixel depende
só do próprio índice. Gerar no processo 0 e espalhar seria mandar pela rede
`N²` floats que cada processo calcula sozinho em menos tempo. Se a entrada
viesse de um arquivo, o `Scatterv` seria necessário, pelo mesmo motivo que
o `Gatherv`: faixas de tamanhos diferentes.

**Por que o checksum não usa `MPI_Reduce` de somas parciais.** A soma em
ponto flutuante não é associativa: somar as faixas separadamente e depois
juntar muda a ordem das parcelas e os últimos dígitos do resultado. O
processo 0 soma a imagem inteira na mesma ordem do sequencial, e o checksum
sai idêntico ao gabarito, sem precisar de tolerância.

---

## 4. O que é medido

Conforme o [protocolo](protocolo-medicao.md), o tempo do MPI vai de antes da
distribuição dos dados até depois da coleta do resultado:

- **Início:** depois de uma `MPI_Barrier`, logo antes do laço de passes. Não
  há distribuição, porque cada processo gera a própria faixa.
- **Fim:** depois do `MPI_Gatherv`, no processo 0.
- **Fora:** `MPI_Init`, geração da imagem, checksum e `MPI_Finalize`, como
  no sequencial.

A segunda linha da saída separa o tempo em:

| Campo | O que mede |
|---|---|
| `calculo` | soma do laço do filtro em todos os passes (maior valor entre os processos) |
| `halo` | soma das duas chamadas `Sendrecv` em todos os passes (maior valor entre os processos) |
| `gather` | o `MPI_Gatherv` no processo 0 |

O tempo de `halo` inclui **espera**, não só transferência: o `Sendrecv` só
retorna quando o vizinho também chegou na troca. Se um processo atrasa (carga
desigual, CPU disputada), os vizinhos registram esse atraso como halo.

---

## 5. Overhead de comunicação (#19)

Medição oficial de 11/11/2026, na máquina de referência do baseline.
Medianas de 5 execuções por configuração, com uma de aquecimento
descartada. Dados brutos em
[../dados/medicoes/mpi.csv](../dados/medicoes/mpi.csv).

### N = 2000

| Processos | Tempo | Cálculo | Halo | Gather | Comunicação |
|---|---|---|---|---|---|
| 1 | 0,346319 s | 0,330064 s | 0,000016 s | 0,013330 s | 3,9% |
| 2 | 0,180680 s | 0,166464 s | 0,002750 s | 0,010945 s | 7,6% |
| 4 | 0,097152 s | 0,084558 s | 0,003825 s | 0,010352 s | 14,6% |
| 8 | 0,095946 s | 0,079751 s | 0,037606 s | 0,014097 s | 53,9% |
| 16 | 0,077513 s | 0,044822 s | 0,016004 s | 0,014768 s | 39,7% |

### N = 1000

| Processos | Tempo | Cálculo | Halo | Gather | Comunicação |
|---|---|---|---|---|---|
| 1 | 0,085414 s | 0,082620 s | 0,000005 s | 0,002304 s | 2,7% |
| 2 | 0,044234 s | 0,041019 s | 0,001024 s | 0,002476 s | 7,9% |
| 4 | 0,024762 s | 0,020594 s | 0,001284 s | 0,002463 s | 15,1% |
| 8 | 0,023346 s | 0,020801 s | 0,010763 s | 0,002911 s | 58,6% |
| 16 | 0,016280 s | 0,011142 s | 0,001587 s | 0,003535 s | 31,5% |

### N = 500

| Processos | Tempo | Cálculo | Halo | Gather | Comunicação |
|---|---|---|---|---|---|
| 1 | 0,022111 s | 0,021145 s | 0,000003 s | 0,000603 s | 2,7% |
| 2 | 0,012184 s | 0,011037 s | 0,000994 s | 0,000663 s | 13,6% |
| 4 | 0,006277 s | 0,005343 s | 0,000639 s | 0,000700 s | 21,3% |
| 8 | 0,005005 s | 0,004266 s | 0,002014 s | 0,000725 s | 54,7% |
| 16 | 0,004482 s | 0,002734 s | 0,000660 s | 0,001251 s | 42,6% |

`Comunicação` é `(halo + gather) / tempo`.

### Como ler estes números

`Cálculo` e `halo` são o **maior valor entre os processos**, obtido por
`MPI_Reduce` com `MPI_MAX`, e podem vir de processos diferentes. Por isso a
soma das três colunas não fecha com o tempo total: em N = 2000 com 8
processos, cálculo e halo somados passam do tempo medido. A coluna de
comunicação é, portanto, um limite superior, não uma partição exata.

### O que os dados confirmam

**O cálculo cai com o número de processos, mas não com `1/p`.** Em
N = 2000, de 1 para 16 processos o cálculo cai de 0,330 s para 0,0448 s —
fator 7,4, não 16. A queda acompanha o mesmo platô observado no OpenMP e
discutido na [análise experimental](analise-experimental.md): o filtro é
limitado por largura de banda de memória, e processos adicionais não trazem
mais banda.

**O gather não depende de `p`.** Fica entre 0,010 s e 0,015 s em todas as
configurações de N = 2000, como previsto: ele move `N²` floats para o
processo 0 qualquer que seja o número de processos. Com 1 processo já custa
0,013 s, e é a razão de a versão MPI com 1 processo ser mais lenta que o
sequencial.

**O halo cresce com `p`, mas de forma irregular.** Com 1 processo é
praticamente zero — não há vizinho, e o `MPI_PROC_NULL` não transfere nada.
Cresce até 0,0376 s com 8 processos e cai para 0,0160 s com 16. Essa
irregularidade vem do que o halo realmente mede: o `Sendrecv` só retorna
quando o vizinho chega à troca, então o tempo inclui **espera**, não só
transferência. Quanto mais processos disputam as mesmas CPUs, mais variável
fica esse tempo de espera — a dispersão nessas configurações chegou a 37%
em N = 2000 com 16 processos.

**A fração de comunicação cresce com `p` e é maior em imagens pequenas.**
Com 4 processos, ela é de 14,6% em N = 2000 e 21,3% em N = 500. O motivo é
dimensional: o cálculo cresce com `N²` e o halo com `N`, então quanto menor
a imagem, maior o peso relativo da borda.

---

## 6. Speedup por número de processos (#20)

```
S(p) = T_sequencial / T_mpi(p)
E(p) = S(p) / p
```

`T_sequencial` é o tempo do código de referência, não o do MPI com 1
processo (protocolo, seção 8). As medianas do baseline usadas como
denominador: 0,018378 s para N = 500, 0,074939 s para N = 1000 e
0,287750 s para N = 2000.

| Processos | N = 500 | N = 1000 | N = 2000 | Amdahl |
|---|---|---|---|---|
| 1 | 0,83× | 0,88× | 0,83× | 1,00× |
| 2 | 1,51× | 1,69× | 1,59× | 1,97× |
| 4 | 2,93× | 3,03× | 2,96× | 3,85× |
| 8 | 3,67× | 3,21× | 3,00× | 7,32× |
| 16 | 4,10× | 4,60× | 3,71× | 13,36× |

Eficiência correspondente:

| Processos | N = 500 | N = 1000 | N = 2000 |
|---|---|---|---|
| 1 | 83,1% | 87,7% | 83,1% |
| 2 | 75,4% | 84,7% | 79,6% |
| 4 | 73,2% | 75,7% | 74,0% |
| 8 | 45,9% | 40,1% | 37,5% |
| 16 | 25,6% | 28,8% | 23,2% |

### Leitura

**Com 1 processo o MPI é mais lento que o sequencial** (0,83×). Não é erro:
a versão MPI paga o `MPI_Gatherv` da imagem inteira, que o sequencial não
tem. Em N = 2000 são 0,013 s de gather sobre 0,330 s de cálculo.

**Até 4 processos a escala é boa**, com eficiência em torno de 74%, apesar
da penalidade inicial do gather.

**Entre 4 e 8 processos a curva empaca**: em N = 2000 o speedup sai de
2,96× para 3,00×. A eficiência cai de 74% para 37,5%. O mesmo platô aparece
na versão OpenMP, que não tem comunicação alguma — o que indica que a causa
está no hardware, não no MPI.

**Com 16 processos o comportamento é irregular**, e a dispersão entre
execuções sobe muito (de 5% em 2 processos para 37% em 16). São 16
processos disputando 8 núcleos físicos, com `--oversubscribe`.

A discussão completa do afastamento em relação a Amdahl, comum às duas
versões, está na [análise experimental](analise-experimental.md).

---

## 7. Como reproduzir a medição

Na **mesma máquina e na mesma sessão** do baseline sequencial, dentro do
contêiner ou no laboratório:

```bash
make
bash scripts/medir.sh              # baseline: dados/medicoes/sequencial.csv
bash scripts/medir_mpi.sh          # MPI:      dados/medicoes/mpi.csv
python3 scripts/resumir.py         # medianas: dados/medicoes/resumo.csv
python3 scripts/graficos_mpi.py    # graficos/mpi_*.png e tabela em Markdown
```

Por padrão o MPI é medido com 1, 2, 4, 8 e 16 processos (protocolo, seção
5). Acima do número de CPUs o script acrescenta `--oversubscribe` sozinho.
Para outra faixa: `PROCESSOS="1 2 4 8" bash scripts/medir_mpi.sh`.

O `mpi.csv` tem as colunas do protocolo e mais três no fim
(`calculo_s,halo_s,gather_s`). O `resumir.py` ignora as colunas a mais.

Gráficos gerados:

| Arquivo | Conteúdo |
|---|---|
| `graficos/mpi_speedup.png` | speedup por processos, um traço por `N`, com a reta ideal |
| `graficos/mpi_eficiencia.png` | eficiência por processos |
| `graficos/mpi_comunicacao.png` | fração do tempo em cálculo, halo e gather, por `N` |