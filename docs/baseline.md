# Baseline sequencial

Medição do código sequencial de referência em três tamanhos de entrada,
conforme a Issue #8.

Estes tempos são o denominador de todas as curvas de speedup do projeto:
`S(n) = T_sequencial / T_paralelo(n)`.

Dados brutos: [../dados/medicoes/sequencial.csv](../dados/medicoes/sequencial.csv).

---

## 1. Protocolo aplicado

Conforme [protocolo-medicao.md](protocolo-medicao.md):

- 6 execuções por tamanho, a primeira descartada como aquecimento.
- Mediana das 5 válidas; mínimo e máximo registrados.
- 30 passes, kernel 3×3, compilação com `-O2`.
- Tempo medido: o laço de passes, que é o que o código de referência
  cronometra. Geração da imagem e cálculo do checksum ficam fora.
- Máquina e versões em [ambiente.md](ambiente.md).

---

## 2. Resultados

| N | Mediana | Mínimo | Máximo | Dispersão | Checksum |
|---|---|---|---|---|---|
| 500 | 0,018824 s | 0,018164 s | 0,019182 s | 5,4% | 31456625.92 |
| 1000 | 0,077037 s | 0,075045 s | 0,081491 s | 8,4% | 126659356.09 |
| 2000 | 0,287969 s | 0,286519 s | 0,289759 s | 1,1% | 508325215.06 |

Dispersão é `(máximo − mínimo) / mediana`.

Os checksums conferem com o gabarito em todas as 15 execuções válidas.

A dispersão cai conforme o tamanho cresce: em N = 2000 fica em 1,1%, contra
5,4% e 8,4% nos tamanhos menores. Execuções curtas são proporcionalmente
mais afetadas por interferência do escalonador do sistema operacional — o
mesmo intervalo absoluto de ruído pesa mais sobre um tempo menor. Para
efeito de comparação de speedup, N = 2000 é o tamanho mais confiável.

---

## 3. Escalabilidade com N

Com 30 passes fixos, dobrar N deve quadruplicar o tempo:

| Transição | Razão medida |
|---|---|
| 500 → 1000 | 4,09× |
| 1000 → 2000 | 3,74× |

Normalizando pelo número de atualizações de pixel — `30 × (N−2)²`, já que a
moldura não é calculada:

| N | Atualizações de pixel | Tempo por atualização |
|---|---|---|
| 500 | 7 440 120 | 2,530 ns |
| 1000 | 29 880 120 | 2,578 ns |
| 2000 | 119 760 120 | 2,405 ns |

O custo por atualização fica praticamente constante, variando menos de 7%
numa faixa em que o problema cresce 16 vezes. É a confirmação direta do
`O(P × N²)` deduzido na [análise do algoritmo](algoritmo.md).

N = 2000 sai ligeiramente mais barato por atualização que os menores. O
custo fixo dos laços — incremento de contadores, teste de parada, cálculo de
índices — é amortizado por mais trabalho útil em cada iteração externa.

---

## 4. Comparação com o Marco 1

| N | Marco 1 (3 execuções) | Marco 2 (5 execuções) | Diferença |
|---|---|---|---|
| 500 | 0,020283 s | 0,018824 s | −7,2% |
| 1000 | 0,076750 s | 0,077037 s | +0,4% |
| 2000 | 0,295501 s | 0,287969 s | −2,5% |

As medições atuais são iguais ou ligeiramente mais rápidas. Duas causas: o
protocolo atual descarta explicitamente a execução de aquecimento, e as
medições foram feitas com a máquina sem outras cargas, condição que o
protocolo passou a exigir.

Os checksums são idênticos nas duas rodadas, o que confirma que a diferença
está no método de medição e não no cálculo.

---

## 5. Uso destes valores

Para cada tamanho, a mediana da tabela da seção 2 é o `T_sequencial` usado
no cálculo de speedup das versões OpenMP e MPI.

As medições paralelas precisam sair da mesma máquina e nas mesmas condições
destas, sob pena de o speedup medir diferença de ambiente em vez de
diferença de paralelização.