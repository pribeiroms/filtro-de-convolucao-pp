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

| Processos | Tempo | Dispersão | Cálculo | Halo | Gather | Comunicação |
|---|---|---|---|---|---|---|
| 1 | 0,391941 s | 3,4% | 0,373923 s | 0,000027 s | 0,016088 s | 4,1% |
| 2 | 0,203234 s | 7,1% | 0,188701 s | 0,003904 s | 0,012717 s | 8,2% |
| 4 | 0,113940 s | 86,6% | 0,097076 s | 0,004989 s | 0,012507 s | 15,4% |
| 8 | 0,104660 s | 22,9% | 0,091085 s | 0,039284 s | 0,013667 s | 50,6% |
| 16 | 0,080151 s | 51,6% | 0,051710 s | 0,016748 s | 0,015884 s | 40,7% |

### N = 1000

| Processos | Tempo | Dispersão | Cálculo | Halo | Gather | Comunicação |
|---|---|---|---|---|---|---|
| 1 | 0,087206 s | 4,9% | 0,084490 s | 0,000009 s | 0,002858 s | 3,3% |
| 2 | 0,048117 s | 4,0% | 0,042391 s | 0,001994 s | 0,004201 s | 12,9% |
| 4 | 0,027209 s | 10,1% | 0,021626 s | 0,001903 s | 0,004992 s | 25,3% |
| 8 | 0,023787 s | 52,1% | 0,018719 s | 0,010052 s | 0,003642 s | 57,6% |
| 16 | 0,018543 s | 61,7% | 0,011037 s | 0,004286 s | 0,005325 s | 51,8% |

### N = 500

| Processos | Tempo | Dispersão | Cálculo | Halo | Gather | Comunicação |
|---|---|---|---|---|---|---|
| 1 | 0,021553 s | 4,7% | 0,020905 s | 0,000003 s | 0,000639 s | 3,0% |
| 2 | 0,011587 s | 5,4% | 0,010738 s | 0,000648 s | 0,000701 s | 11,6% |
| 4 | 0,006567 s | 23,5% | 0,005278 s | 0,001047 s | 0,000860 s | 29,0% |
| 8 | 0,005609 s | 46,2% | 0,004684 s | 0,002448 s | 0,000851 s | 58,8% |
| 16 | 0,005881 s | 142,7% | 0,002660 s | 0,003410 s | 0,001420 s | 82,1% |

`Comunicação` é `(halo + gather) / tempo`. `Dispersão` é
`(máximo − mínimo) / mediana` entre as 5 execuções.

### Como ler estes números

`Cálculo` e `halo` são o **maior valor entre os processos**, obtido por
`MPI_Reduce` com `MPI_MAX`, e podem vir de processos diferentes. Por isso a
soma das três colunas não fecha com o tempo total. A coluna de comunicação é
um limite superior, não uma partição exata.

As configurações com 8 e 16 processos têm dispersão alta — até 142,7% em
N = 500 com 16 processos. São 8 ou 16 processos disputando 8 núcleos
físicos, e o tempo de halo inclui espera pelo vizinho. As medianas dessas
linhas devem ser lidas como ordem de grandeza, não como valor preciso.

### O que os dados confirmam

**O cálculo cai com o número de processos, mas não com `1/p`.** Em
N = 2000, de 1 para 16 processos o cálculo cai de 0,374 s para 0,052 s —
fator 7,2, não 16. A queda acompanha o mesmo limite observado no OpenMP e
discutido na [análise experimental](analise-experimental.md): o filtro é
limitado por largura de banda de memória, e processos adicionais não trazem
mais banda.

**O gather não depende de `p`.** Fica entre 0,012 s e 0,016 s em todas as
configurações de N = 2000, como previsto: ele move `N²` floats para o
processo 0 qualquer que seja o número de processos. Com 1 processo já custa
0,016 s, e é a razão de a versão MPI com 1 processo ser mais lenta que o
sequencial.

**O halo cresce com `p`, mas de forma irregular.** Com 1 processo é
praticamente zero — não há vizinho, e o `MPI_PROC_NULL` não transfere nada.
Cresce até 0,039 s com 8 processos e cai para 0,017 s com 16. Essa
irregularidade vem do que o halo realmente mede: o `Sendrecv` só retorna
quando o vizinho chega à troca, então o tempo inclui **espera**, não só
transferência.

**A fração de comunicação cresce com `p` e é maior em imagens pequenas.**
Com 4 processos, ela é de 15,4% em N = 2000 e 29,0% em N = 500. O motivo é
dimensional: o cálculo cresce com `N²` e o halo com `N`, então quanto menor
a imagem, maior o peso relativo da borda. Em N = 500 com 16 processos a
comunicação chega a 82% do tempo.

---

## 6. Speedup por número de processos (#20)

```
S(p) = T_sequencial / T_mpi(p)
E(p) = S(p) / p
```

`T_sequencial` é o tempo do código de referência, não o do MPI com 1
processo (protocolo, seção 8). As medianas do baseline usadas como
denominador, de [../dados/medicoes/sequencial.csv](../dados/medicoes/sequencial.csv):
0,019070 s para N = 500, 0,074185 s para N = 1000 e 0,330327 s para
N = 2000.

| Processos | N = 500 | N = 1000 | N = 2000 | Amdahl |
|---|---|---|---|---|
| 1 | 0,88× | 0,85× | 0,84× | 1,00× |
| 2 | 1,65× | 1,54× | 1,63× | 1,97× |
| 4 | 2,90× | 2,73× | 2,90× | 3,85× |
| 8 | 3,40× | 3,12× | 3,16× | 7,32× |
| 16 | 3,24× | 4,00× | 4,12× | 13,36× |

Eficiência correspondente:

| Processos | N = 500 | N = 1000 | N = 2000 |
|---|---|---|---|
| 1 | 88,5% | 85,1% | 84,3% |
| 2 | 82,3% | 77,1% | 81,3% |
| 4 | 72,6% | 68,2% | 72,5% |
| 8 | 42,5% | 39,0% | 39,5% |
| 16 | 20,3% | 25,0% | 25,8% |

### Leitura

**Com 1 processo o MPI é mais lento que o sequencial** (0,84× em N = 2000).
Não é erro: a versão MPI paga o `MPI_Gatherv` da imagem inteira, que o
sequencial não tem. São 0,016 s de gather sobre 0,374 s de cálculo.

**Até 4 processos a escala é razoável**, com eficiência entre 68% e 73%.

**Entre 4 e 8 processos a eficiência cai pela metade**, de 72,5% para 39,5%
em N = 2000. O speedup continua subindo, mas cada processo acrescentado
rende muito menos. O mesmo acontece na versão OpenMP, que não tem
comunicação alguma — o que indica que a causa está no hardware, não no MPI.

**Com 16 processos o comportamento é irregular** e a dispersão sobe muito
(de 7,1% com 2 processos para 51,6% com 16). São 16 processos disputando 8
núcleos físicos, com `--oversubscribe`. Em N = 500 o speedup chega a piorar
em relação a 8 processos.

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