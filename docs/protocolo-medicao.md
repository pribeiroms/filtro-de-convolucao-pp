# Protocolo de medição

Regras válidas para todas as medições do projeto — baseline sequencial,
OpenMP e MPI —, conforme a Issue #7.

Tempos obtidos por métodos diferentes não são comparáveis. Este protocolo
fixa o método.

---

## 1. O que é medido

> **A revisar.**

Tempo da resolução completa do problema em cada versão:

| Versão | Início da medição | Fim da medição |
|---|---|---|
| Sequencial | Antes do laço de passes | Depois do laço de passes |
| OpenMP | Antes do laço de passes | Depois do laço de passes |
| MPI | Antes da distribuição dos dados | Depois da coleta do resultado |

Fora da medição em todas as versões: geração da imagem de entrada e cálculo
do checksum — cerca de 1,3% do tempo, conforme
[perfilamento.md](perfilamento.md), e não são alvo da paralelização.
Também fora: `MPI_Init` e `MPI_Finalize`.

A comunicação do MPI (distribuição, halo, coleta) entra na medição. Ela não
existe no sequencial, mas é trabalho necessário para resolver o mesmo
problema; excluí-la produziria speedup que não corresponde ao custo real de
usar MPI. Consequência esperada: o speedup do MPI cresce menos que o do
OpenMP, e em N pequeno pode piorar com mais processos. Esse comportamento é
objeto das Issues #19 e #20.

---

## 2. Repetições e estatística

- 6 execuções por configuração: a primeira descartada, as 5 seguintes
  válidas.
- Valor reportado: **mediana** das 5 válidas.
- Registrar também **mínimo** e **máximo**.

Mediana em vez de média: a média é deslocada por execução atípica, comum
quando o sistema operacional escalona outra tarefa durante a medição.

5 repetições em vez de 3: a dispersão de 2% observada no perfilamento vale
para o sequencial. Versões paralelas variam mais, por escalonamento de
threads e comunicação.

> O Marco 1 usou medianas de 3 execuções, protocolo anterior a este. As
> medições do Marco 2 seguem as 5 repetições definidas aqui e podem diferir
> dos valores daquele documento.

---

## 3. Descarte de aquecimento

A primeira execução de cada configuração é sempre descartada.

Motivo: na primeira execução os vetores recém-alocados ainda não têm páginas
de memória atribuídas pelo sistema operacional, e o primeiro acesso a cada
página gera falha de página dentro do trecho cronometrado. Caches e
preditores de desvio também começam vazios.

---

## 4. Tamanhos de entrada e kernel

| Rótulo | N | Pixels | Memória dos dois buffers |
|---|---|---|---|
| Pequeno | 500 | 250 mil | ~2 MB |
| Médio | 1000 | 1 milhão | ~8 MB |
| Grande | 2000 | 4 milhões | ~32 MB |

Passes: **30** em todos os tamanhos.

Kernel: **3×3 fixo**, definido pelo código de referência e inalterável.

Os três tamanhos atravessam a hierarquia de memória da máquina de medição,
cujo L3 é de 16 MiB: pequeno e médio cabem em cache, grande não cabe.

---

## 5. Graus de paralelismo

Para cada tamanho, as versões paralelas são medidas com **1, 2, 4, 8 e 16
unidades** — threads via `OMP_NUM_THREADS` no OpenMP, processos via
`mpirun -np` no MPI.

A máquina tem 8 núcleos físicos e 16 CPUs lógicas. A faixa vai até 16 para
cobrir a região além dos núcleos reais, onde duas threads de um mesmo núcleo
disputam as mesmas unidades de execução.

Configurações com mais processos que CPUs exigem `--oversubscribe` no
`mpirun`; registrar isso junto da medição.

---

## 6. Máquina e condições

Todas as medições de uma mesma curva: mesma máquina, mesma sessão. Tempos de
máquinas diferentes não podem ser misturados num gráfico.

Máquina e versões de ferramentas em [ambiente.md](ambiente.md).

Condições exigidas:

- Navegador, editores e demais aplicativos fechados.
- Nenhum outro contêiner em execução.
- Notebook na tomada e em modo de desempenho. Em economia de bateria a
  frequência do processador cai e os tempos ficam inconsistentes.
- Máquina sem uso durante a medição.
- Bateria completa de medições rodada de uma vez, para que todas as
  configurações enfrentem as mesmas condições térmicas.

---

## 7. Formato dos dados

CSV em `dados/medicoes/`, uma linha por execução válida:

```
versao,N,passes,unidades,repeticao,tempo_s,checksum
```

| Coluna | Conteúdo |
|---|---|
| `versao` | `sequencial`, `openmp` ou `mpi` |
| `N` | Lado da imagem |
| `passes` | Número de passes (30) |
| `unidades` | Threads ou processos; 1 para o sequencial |
| `repeticao` | 1 a 5; a execução de aquecimento não é gravada |
| `tempo_s` | Tempo medido em segundos |
| `checksum` | Checksum produzido |

Gravar execuções individuais, não medianas: a mediana é recalculável a
partir dos dados brutos, o inverso não. O CSV é a evidência.

O checksum em cada linha permite identificar configuração com tempo válido e
resultado errado.

### Resumo derivado

`scripts/resumir.py` lê os CSV brutos e grava `dados/medicoes/resumo.csv`,
com uma linha por configuração:

```
versao,N,passes,unidades,execucoes,mediana_s,min_s,max_s,dispersao_pct,checksum
```

É daí que saem as tabelas dos documentos e os gráficos da Issue #23. O
arquivo é derivado e pode ser regerado a qualquer momento sem repetir as
medições. Dispersão é `(máximo − mínimo) / mediana`, em percentual.

O script avisa quando uma configuração produziu checksums diferentes entre
execuções, o que indica não determinismo na implementação.

---

## 8. Speedup e eficiência

```
S(n) = T_sequencial / T_paralelo(n)
E(n) = S(n) / n
```

`T_sequencial` é o tempo da versão sequencial medida, não o da versão
paralela com uma unidade. Usar a paralela com 1 thread como referência infla
o speedup, porque esse tempo já inclui criação de threads e custo das
diretivas.

Por isso o CSV registra a versão sequencial em linhas próprias, com
`versao=sequencial` e `unidades=1`.

---

## 9. Validação junto da medição

Toda execução registra o checksum. Configuração só é válida se o checksum
conferir com o gabarito em `dados/gabarito/`.

A comparação usa **tolerância numérica**, não igualdade exata de texto: a
soma em ponto flutuante não é associativa, e paralelizar a redução altera a
ordem das parcelas, podendo mudar os últimos dígitos mesmo em implementação
correta. Critério de tolerância definido na Issue #21.

Tempo de configuração com resultado errado não é dado de desempenho.