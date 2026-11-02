# Fração paralelizável e Lei de Amdahl

Estimativa do speedup teórico máximo a partir do perfilamento, conforme a
Issue #9.

---

## 1. Fração paralelizável

A fração paralelizável `p` é a parcela do tempo gasta no trecho que a
paralelização ataca. Pelo [perfilamento](perfilamento.md), esse trecho é o
laço de convolução:

| N | Fração da convolução | p | 1 − p |
|---|---|---|---|
| 500 | 98,69% | 0,9869 | 0,0131 |
| 1000 | 98,74% | 0,9874 | 0,0126 |
| 2000 | 98,68% | 0,9868 | 0,0132 |

**Valor adotado: p = 0,9868**, medido em N = 2000. É o tamanho com menor
dispersão entre execuções (1,1%, contra 5,4% e 8,4% nos menores) e o que
será usado nas demonstrações.

A fração restante, 1,32%, é a geração da imagem de entrada e o cálculo do
checksum — trechos que permanecem sequenciais.

---

## 2. Lei de Amdahl

```
S(n) = 1 / ((1 − p) + p/n)
```

onde `n` é o número de unidades de processamento. O termo `(1 − p)` não
depende de `n`: por mais unidades que se acrescente, essa parcela do tempo
continua sendo paga integralmente. É ela que impõe o teto.

No limite:

```
S(∞) = 1 / (1 − p) = 1 / 0,0132 = 75,76×
```

Mesmo com unidades ilimitadas, o programa não pode ficar mais de 75,76 vezes
mais rápido, porque 1,32% do trabalho nunca é dividido.

---

## 3. Speedup teórico

Com p = 0,9868:

| n | S(n) | Eficiência E(n) = S(n)/n |
|---|---|---|
| 2 | 1,97× | 98,7% |
| 4 | 3,85× | 96,2% |
| 8 | 7,32× | 91,5% |
| 16 | 13,36× | 83,5% |
| 32 | 22,71× | 71,0% |
| 64 | 34,94× | 54,6% |
| ∞ | 75,76× | 0% |

A eficiência cai conforme `n` cresce. Em 8 unidades ainda se aproveita 91,5%
do potencial; em 64, apenas 54,6%. O retorno de cada unidade adicional
diminui, porque a parcela sequencial passa a dominar o tempo restante.

---

## 4. Sensibilidade de p

A mesma tabela, calculada com as frações dos três tamanhos medidos:

| n | p = 0,9869 (N=500) | p = 0,9874 (N=1000) | p = 0,9868 (N=2000) |
|---|---|---|---|
| 2 | 1,97× | 1,98× | 1,97× |
| 4 | 3,85× | 3,85× | 3,85× |
| 8 | 7,33× | 7,35× | 7,32× |
| 16 | 13,37× | 13,46× | 13,36× |
| ∞ | 76,34× | 79,37× | 75,76× |

Na faixa que será medida na prática — até 16 unidades — a escolha de `p`
entre esses três valores é irrelevante: a diferença não passa de 0,1×.

No limite, porém, a mesma variação de 0,06 ponto percentual em `p` move
`S(∞)` em quase 4 unidades. Isso é característica da fórmula perto de
p = 1: `S(∞)` é `1/(1−p)`, e o denominador é a diferença entre dois números
próximos. Pequenos erros na estimativa de `p` produzem variações grandes no
teto teórico, o que torna `S(∞)` um número pouco confiável para citar com
precisão.

---

## 5. Por que o speedup real ficará abaixo

A Lei de Amdahl considera apenas a divisão do trabalho. Ela supõe que a
parcela paralelizável se divide perfeitamente, sem custo algum. Na prática,
quatro fatores afastam o resultado medido da curva teórica:

**Barreiras entre passes.** Os passes são sequencialmente dependentes: o
passe `p+1` só começa depois que o passe `p` termina. São 30 barreiras de
sincronização por execução. O custo relativo depende do tamanho: cada passe
leva cerca de 627 µs em N = 500 e 9 599 µs em N = 2000, de modo que uma
barreira de poucos microssegundos pesa mais de dez vezes mais no tamanho
menor.

**Largura de banda de memória.** Os dois buffers somam 32 MB em N = 2000,
acima dos 16 MiB de cache L3. Threads adicionais não trazem mais largura de
banda — disputam a mesma. Este é provavelmente o fator limitante mais forte
neste problema, que faz poucas operações aritméticas por byte lido.

**CPUs lógicas não são núcleos.** A máquina tem 8 núcleos físicos e 16 CPUs
lógicas. Em 16 threads, duas threads por núcleo disputam as mesmas unidades
de execução. O valor de 13,36× previsto para n = 16 é inalcançável; o
esperado é que a curva se achate perto de 8.

**Comunicação, no caso do MPI.** A fórmula não tem termo algum para troca de
dados entre processos. Distribuição, troca de halo a cada passe e coleta do
resultado são custo que cresce com o número de processos e não existe no
modelo.

---

## 6. Escopo da análise

Amdahl descreve **escalabilidade forte**: problema de tamanho fixo, número
crescente de unidades. É o cenário das curvas de speedup das Issues #15 e
#20.

A escalabilidade fraca — problema crescendo junto com o número de unidades,
mantendo a carga por unidade constante — é governada por outra relação e
será tratada na análise experimental da Issue #23.

---

## 7. Uso no Marco 2

A tabela da seção 3 é a referência teórica contra a qual as curvas medidas
de OpenMP e MPI serão comparadas. O interesse da comparação não é bater o
valor teórico, e sim quantificar e explicar o afastamento: qual parcela da
diferença vem de barreiras, qual de largura de banda e, no MPI, qual de
comunicação.