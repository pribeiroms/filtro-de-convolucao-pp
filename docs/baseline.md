# Baseline sequencial

Medição do código sequencial de referência em três tamanhos de entrada,
conforme a Issue #8.

Estes tempos são o denominador de todas as curvas de speedup do projeto:
`S(n) = T_sequencial / T_paralelo(n)`.

Dados brutos: [../dados/medicoes/sequencial.csv](../dados/medicoes/sequencial.csv).
Este arquivo é a **única** fonte do baseline no repositório.

---

## 1. Protocolo aplicado

Conforme [protocolo-medicao.md](protocolo-medicao.md):

- 6 execuções por tamanho, a primeira descartada como aquecimento.
- Mediana das 5 válidas; mínimo e máximo registrados.
- 30 passes, kernel 3×3, compilação com `-O2`.
- Tempo medido: o laço de passes, que é o que o código de referência
  cronometra. Geração da imagem e cálculo do checksum ficam fora.
- Máquina e versões em [ambiente.md](ambiente.md).

A medição foi feita em 11/11/2026, na mesma sessão das medições de OpenMP e
MPI. A seção 6 do protocolo exige que todas as medições de uma curva saiam
da mesma máquina e sessão, e o baseline é o denominador das duas curvas.

---

## 2. Resultados

| N | Mediana | Mínimo | Máximo | Dispersão | Checksum |
|---|---|---|---|---|---|
| 500 | 0,019070 s | 0,018084 s | 0,019532 s | 7,6% | 31456625.92 |
| 1000 | 0,074185 s | 0,072423 s | 0,075566 s | 4,2% | 126659356.09 |
| 2000 | 0,330327 s | 0,325515 s | 0,334947 s | 2,9% | 508325215.06 |

Dispersão é `(máximo − mínimo) / mediana`.

Os checksums conferem com o gabarito nas 15 execuções válidas.

A dispersão cai conforme o tamanho cresce, de 7,6% em N = 500 para 2,9% em
N = 2000. Execuções curtas são proporcionalmente mais afetadas por
interferência do escalonador do sistema operacional: o mesmo intervalo
absoluto de ruído pesa mais sobre um tempo menor. Por isso N = 2000 é o
tamanho em que as conclusões do projeto se apoiam.

---

## 3. Escalabilidade com N

Com 30 passes fixos, dobrar N deve quadruplicar o tempo:

| Transição | Razão medida |
|---|---|
| 500 → 1000 | 3,89× |
| 1000 → 2000 | 4,45× |

Normalizando pelo número de atualizações de pixel — `30 × (N−2)²`, já que a
moldura não é calculada:

| N | Atualizações de pixel | Tempo por atualização |
|---|---|---|
| 500 | 7 440 120 | 2,563 ns |
| 1000 | 29 880 120 | 2,483 ns |
| 2000 | 119 760 120 | 2,758 ns |

O custo por atualização varia cerca de 11% numa faixa em que o problema
cresce 16 vezes, confirmando o `O(P × N²)` deduzido na
[análise do algoritmo](algoritmo.md). A variação restante é da mesma ordem
da dispersão entre execuções e não indica desvio do comportamento
quadrático.

A [análise experimental](analise-experimental.md) estende essa verificação a
cinco tamanhos, com o mesmo resultado.

---

## 4. Comparação com o Marco 1

| N | Marco 1 (3 execuções) | Marco 2 (5 execuções) | Diferença |
|---|---|---|---|
| 500 | 0,020283 s | 0,019070 s | −6,0% |
| 1000 | 0,076750 s | 0,074185 s | −3,3% |
| 2000 | 0,295501 s | 0,330327 s | +11,8% |

Os checksums são idênticos nas duas rodadas, o que confirma que a diferença
está no ambiente de medição e não no cálculo.

A diferença de 11,8% em N = 2000 está dentro da variação observada entre
sessões de medição distintas na mesma máquina, discutida na seção 7 da
[análise experimental](analise-experimental.md). É o motivo de o protocolo
exigir que todas as medições de uma curva saiam da mesma sessão: comparar
tempos de sessões diferentes mede o estado da máquina, não o efeito da
paralelização.

---

## 5. Uso destes valores

Para cada tamanho, a mediana da tabela da seção 2 é o `T_sequencial` usado
no cálculo de speedup das versões OpenMP e MPI, nos documentos e nos
gráficos.

O `scripts/medir_openmp.sh` também grava linhas `sequencial` dentro de
`openmp.csv`, como subproduto da sua execução. Essas linhas **não** são
usadas como baseline: o `scripts/resumir.py` e o `scripts/graficos_openmp.py`
as descartam e leem apenas este arquivo, para que exista um único baseline
no repositório.