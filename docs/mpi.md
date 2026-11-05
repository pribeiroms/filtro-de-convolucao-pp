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

> **A preencher** com a medição oficial (seção 7).

Comportamento esperado:

- **Cálculo** cai com `1/p`: cada processo fica com `N/p` linhas.
- **Halo** não cai: cada processo troca sempre `2·N` floats por passe,
  independentemente de `p`. Com mais processos, há mais trocas acontecendo
  ao mesmo tempo e mais pontos de espera.
- **Gather** move a imagem inteira (`N²` floats) para o processo 0, qualquer
  que seja `p`.

A fração de comunicação, portanto, **cresce com o número de processos**. E
como cálculo cresce com `N²` e halo com `N`, ela é **maior em imagens
pequenas**: em N = 500 a comunicação domina antes do que em N = 2000.

### Teste preliminar (não é medição oficial)

Rodado no notebook da Paula (Windows, MS-MPI, 12 CPUs lógicas), N = 2000,
mediana de 5 execuções. Serve só para conferir que os scripts funcionam e
que a tendência aparece. **Não usar no relatório:** é outra máquina e outro
sistema, diferentes do baseline.

| Processos | Tempo | Cálculo | Halo | Gather | Comunicação |
|---|---|---|---|---|---|
| 1 | 0,187 s | 0,180 s | 0,000 s | 0,008 s | 4% |
| 2 | 0,088 s | 0,080 s | 0,005 s | 0,007 s | 13% |
| 4 | 0,080 s | 0,065 s | 0,017 s | 0,008 s | 31% |
| 16 | 0,115 s | 0,021 s | 0,079 s | 0,029 s | 93% |

Com 16 processos, mais do que as 12 CPUs da máquina, o cálculo continua
caindo, mas o halo explode: processos dividem a mesma CPU e um espera o
outro ser escalonado para concluir a troca. O tempo total piora em relação a
4 processos.

---

## 6. Speedup por número de processos (#20)

> **A preencher** com a medição oficial (seção 7).

```
S(p) = T_sequencial / T_mpi(p)
E(p) = S(p) / p
```

`T_sequencial` é o tempo do código de referência, não o do MPI com 1
processo (protocolo, seção 8).

**O resultado não muda com o número de processos.** O
`scripts/medir_mpi.sh` compara o checksum de cada execução com o gabarito e
termina com erro se algum divergir. O `scripts/graficos_mpi.py` confere de
novo que todas as configurações de um mesmo `N` deram o mesmo checksum.

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
