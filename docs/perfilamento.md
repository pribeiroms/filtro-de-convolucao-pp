# Perfilamento do código sequencial

Identificação do hotspot do código de referência
`sequencial/04_convolucao.cpp`, conforme a Issue #6.

A evidência bruta das medições está em
[perfilamento_bruto.txt](perfilamento_bruto.txt).

---

## 1. Metodologia

O perfilamento usa `sequencial/04_convolucao_instrumentado.cpp`, cópia do
código de referência com temporizadores em volta de cada trecho. A lógica
de cálculo e a entrada são idênticas às do original — a instrumentação
apenas mede. O checksum produzido pela cópia é igual ao do código de
referência em todos os tamanhos, o que confirma que medir não alterou o
resultado.

Os trechos medidos são os três que o programa executa em sequência:

1. **Inicialização da imagem** — preenchimento determinístico do vetor de
   entrada.
2. **Convolução** — o laço de passes, com os laços aninhados de linhas,
   colunas e do kernel 3×3.
3. **Checksum** — a soma de todos os elementos da matriz final.

Protocolo adotado:

- Compilação com `-O2`, a mesma otimização usada no baseline e nas versões
  paralelas.
- 30 passes em todos os tamanhos.
- Quatro execuções por tamanho: a primeira é descartada como aquecimento e
  as três seguintes são as medições válidas.
- O valor reportado é a **mediana** das três execuções válidas, não a média,
  por ser menos sensível a uma execução atípica.

Ambiente da medição: g++ 13.3.0, Ubuntu 24.04 em contêiner, 16 CPUs
visíveis. Detalhes em [ambiente.md](ambiente.md).

---

## 2. Resultados

Medianas das três execuções válidas de cada tamanho:

### N = 500

| Trecho | Tempo | % do total |
|---|---|---|
| Inicialização da imagem | 0,000103 s | 0,52% |
| **Convolução 2D** | **0,019370 s** | **98,69%** |
| Cálculo do checksum | 0,000153 s | 0,78% |
| Total | 0,019627 s | 100% |

### N = 1000

| Trecho | Tempo | % do total |
|---|---|---|
| Inicialização da imagem | 0,000403 s | 0,50% |
| **Convolução 2D** | **0,079542 s** | **98,74%** |
| Cálculo do checksum | 0,000611 s | 0,76% |
| Total | 0,080557 s | 100% |

### N = 2000

| Trecho | Tempo | % do total |
|---|---|---|
| Inicialização da imagem | 0,001676 s | 0,53% |
| **Convolução 2D** | **0,314959 s** | **98,68%** |
| Cálculo do checksum | 0,002522 s | 0,79% |
| Total | 0,319182 s | 100% |

### Dispersão entre execuções

A variação entre as três execuções válidas de cada tamanho ficou abaixo de
2% no trecho da convolução — em N = 2000, entre 0,313609 s e 0,317110 s.
Essa estabilidade é o que torna três repetições suficientes.

---

## 3. O hotspot

O hotspot é o **laço de convolução**, com aproximadamente **98,7% do tempo
total** de execução. Concretamente, o trecho

```
passes → linhas → colunas → dy → dx
```

onde o laço mais interno percorre a vizinhança 3×3 de cada pixel.

Os outros dois trechos somados — inicialização e checksum — não passam de
1,3% do tempo. Mesmo que fossem eliminados por completo, o ganho máximo
seria de pouco mais de 1%.

**A conclusão para a paralelização é direta: o esforço tem de se concentrar
no laço de convolução.** Otimizar ou paralelizar a inicialização ou o
checksum não produz ganho mensurável.

---

## 4. A fração do hotspot não depende do tamanho

Os três tamanhos medidos dão praticamente o mesmo percentual:

| N | Fração da convolução |
|---|---|
| 500 | 98,69% |
| 1000 | 98,74% |
| 2000 | 98,68% |

Isso não é coincidência, e pode ser previsto pela contagem de operações por
pixel:

- A convolução faz 9 leituras por pixel por passe, ou seja `9 × 30 = 270`
  acessos por pixel ao todo.
- A inicialização faz 1 escrita por pixel.
- O checksum faz 1 leitura por pixel.

Todos os três trechos crescem com `N²`, então a razão entre eles é constante
em relação a `N`:

```
fração esperada = 270 / (270 + 1 + 1) = 270 / 272 ≈ 99,26%
```

O medido, 98,7%, fica um pouco abaixo dessa previsão. A diferença tem
explicação: a contagem de operações supõe que toda operação custa o mesmo, o
que não é verdade. A convolução lê uma vizinhança 3×3, e boa parte desses
nove acessos já está em cache — pixels vizinhos são reutilizados no cálculo
de pixels adjacentes. Já a inicialização e o checksum percorrem a matriz
inteira uma única vez, sem reúso, pagando o custo cheio de acesso à memória
principal. Por isso a convolução é proporcionalmente mais barata por
operação do que a contagem sugere.

---

## 5. Confirmação empírica da complexidade

A análise do algoritmo prevê custo `O(P × N²)`. Com `P` fixo em 30, dobrar
`N` deve quadruplicar o tempo. Os tempos totais medidos:

| N | Total | Razão em relação ao anterior |
|---|---|---|
| 500 | 0,019627 s | — |
| 1000 | 0,080557 s | 4,10× |
| 2000 | 0,319182 s | 3,96× |

Ambas as razões ficam muito próximas de 4, confirmando na prática o
comportamento quadrático deduzido na
[análise do algoritmo](algoritmo.md).

---

## 6. Consistência com o Marco 1

O documento do Marco 1 registrou 98,68% para N = 2000 e P = 30, a partir de
uma execução única feita semanas antes. A medição atual, com mediana de três
execuções, resultou **exatamente no mesmo valor**.

| Origem | N | Fração da convolução |
|---|---|---|
| Documento do Marco 1 | 2000 | 98,68% |
| Esta medição (mediana) | 2000 | 98,68% |

A reprodutibilidade confirma que o ambiente atual, definido pelo
`Dockerfile` do repositório, produz medições equivalentes às do ambiente em
que o Marco 1 foi medido.

---

## 7. Implicação para a Lei de Amdahl

A fração do tempo gasta no trecho paralelizável é a entrada da Lei de
Amdahl. Com o hotspot em 98,68% para N = 2000:

- Fração paralelizável: 0,9868
- Fração sequencial: 0,0132

Como a fração se mostrou praticamente idêntica nos três tamanhos, a
estimativa de Amdahl vale para toda a faixa medida, e não apenas para
N = 2000. O cálculo do speedup teórico é desenvolvido na Issue #9.

Vale lembrar que essa fração é um limite superior otimista: ela supõe que
todo o trecho da convolução seja perfeitamente paralelizável, sem custo de
criação de threads, sincronização, comunicação entre processos ou disputa
por largura de banda de memória. O speedup medido na prática será menor.