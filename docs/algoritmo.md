# Análise do algoritmo sequencial

Análise do código de referência `sequencial/04_convolucao.cpp`, fornecido
pelo professor, conforme a Issue #5.

---

## 1. O que o código faz

O programa aplica `P` passes de um **filtro de convolução 2D** sobre uma
imagem sintética de `N × N` pixels, representada por um vetor de `float`
em ordem de linhas (o pixel `(x, y)` fica no índice `y*N + x`).

A imagem é gerada deterministicamente:

```cpp
for (int i = 0; i < N*N; i++) a[i] = (float)((i * 131) % 256);
```

Não há leitura de arquivo: a entrada é sempre a mesma para um dado `N`, o
que torna o resultado reproduzível e permite usar o checksum como critério
de corretude.

### O kernel

Para cada pixel interno, o código soma os nove valores da vizinhança 3×3 e
divide por nove:

```cpp
float s = 0;
for (int dy = -1; dy <= 1; dy++)
    for (int dx = -1; dx <= 1; dx++)
        s += a[(y+dy)*N + (x+dx)];
b[y*N + x] = s / 9.0f;
```

Isso é a convolução com o kernel

```
1/9  1/9  1/9
1/9  1/9  1/9
1/9  1/9  1/9
```

ou seja, uma **média ponderada da vizinhança em que todos os pesos são
iguais**. É o caso particular chamado *box blur*: o efeito é suavizar a
imagem, atenuando variações bruscas entre pixels vizinhos.

O kernel não aparece como uma matriz no código. Ele está implícito na soma
seguida da divisão por nove — o que só é possível porque todos os pesos são
idênticos. Um kernel com pesos diferentes (gaussiano, por exemplo) exigiria
multiplicar cada vizinho pelo seu peso.

### Os dois buffers e a troca

O programa mantém dois vetores, `a` (entrada) e `b` (saída). Ao final de
cada passe:

```cpp
a.swap(b);
```

A troca faz o resultado do passe virar a entrada do passe seguinte. Como
`swap` apenas troca os ponteiros internos dos vetores, a operação é
barata — não há cópia de dados.

---

## 2. Tratamento das bordas

Os laços percorrem de `1` a `N-2` em ambas as dimensões:

```cpp
for (int y = 1; y < N-1; y++)
    for (int x = 1; x < N-1; x++)
```

Ou seja, **a moldura de um pixel de largura é ignorada**. O código não faz
zero-padding nem replicação de borda: ele simplesmente não calcula esses
pixels, porque a vizinhança 3×3 deles cairia fora da imagem.

### A consequência da combinação com o swap

Como as posições de borda do buffer de saída nunca são escritas, elas
mantêm o que já havia naquele buffer. E, por causa do `swap`, o buffer de
saída alterna a cada passe. O efeito é que **a borda do resultado final
depende da paridade do número de passes**:

| Número de passes | Borda do resultado |
|---|---|
| Ímpar | zeros |
| Par | valores originais da imagem |

A razão: `b` nasce preenchido com zeros, porque `vector<float> b(N*N)`
inicializa com zero. No primeiro passe, a borda de `b` permanece zerada.
Depois da troca, o buffer que recebe a escrita passa a ser o que continha a
imagem original, cuja borda nunca foi tocada.

Verificação com `N = 5`, imprimindo a primeira linha do resultado:

```
P= 0  (entrada)     0.00  131.00    6.00  137.00   12.00
P= 1                0.00    0.00    0.00    0.00    0.00
P= 2                0.00  131.00    6.00  137.00   12.00
P= 3                0.00    0.00    0.00    0.00    0.00
P=30                0.00  131.00    6.00  137.00   12.00
P=31                0.00    0.00    0.00    0.00    0.00
```

Com os `P = 30` usados nas medições, a borda final carrega os valores
originais da imagem.

### Quanto isso pesa no checksum

As bordas somam `4(N−1)` pixels e entram no checksum:

| N | Soma das bordas | % do checksum |
|---|---|---|
| 500 | 253 038,00 | 0,804% |
| 1000 | 506 390,00 | 0,400% |
| 2000 | 1 015 590,00 | 0,200% |

Não é desprezível. Qualquer versão paralela que trate a borda de forma
diferente — zerando-a, replicando-a ou calculando-a — produz um checksum
diferente e falha na validação, mesmo que o interior esteja perfeito.

### O que isso impõe ao MPI

Este é o ponto que mais afeta a decomposição de domínio. É preciso
distinguir dois tipos de fronteira:

- **A borda da imagem global** — nunca deve ser calculada por processo
  nenhum, e tem de preservar o comportamento de alternância descrito acima.
- **As fronteiras internas entre as faixas de cada processo** — são pixels
  *interiores* da imagem global e **precisam** ser calculados. Para isso, o
  processo precisa das linhas vizinhas, que pertencem a outro processo: é
  daí que vem a necessidade da troca de halo.

Confundir as duas é o erro clássico da decomposição: um processo trata a
primeira linha da sua faixa como se fosse borda da imagem, deixa de
calculá-la, e o resultado sai errado em todas as emendas entre faixas.

---

## 3. Complexidade assintótica

Em cada passe são processados os pixels internos, `(N−2)²`, e para cada um
deles o kernel `k × k` executa `k²` iterações:

```
custo por passe = (N − 2)² × k²
custo total     = P × (N − 2)² × k²
```

Com o kernel fixo em 3×3 (`k² = 9`, constante) e `(N − 2)² ≈ N²`:

```
O(P × N²)
```

Se `P` também for tratado como constante, a complexidade em função apenas
do tamanho da imagem é `O(N²)` — linear no número de pixels.

### Espaço

Dois vetores de `N²` floats:

```
O(N²)
```

Em bytes: `2 × 4 × N²`. Para `N = 2000` são cerca de **32 MB**, bem acima
dos 16 MiB de cache L3 da máquina de desenvolvimento. Isso significa que o
algoritmo não cabe em cache e depende de tráfego com a memória principal —
fator que limita o speedup real, já que threads adicionais disputam a mesma
largura de banda de memória.

### Localidade de acesso

Os dados estão em ordem de linhas e o laço interno percorre `x`, ou seja,
posições contíguas na memória. É o padrão de acesso mais favorável ao
cache. Paralelizar o laço externo (`y`, as linhas) mantém cada thread
trabalhando sobre faixas contíguas, o que preserva essa localidade.

---

## 4. Independência entre os pixels

**Dentro de um mesmo passe**, o cálculo de cada pixel de saída:

- lê **apenas** o buffer de entrada `a`, que não é modificado durante o
  passe;
- escreve em **uma única** posição de `b`, distinta da de qualquer outro
  pixel.

Como nenhuma posição de `b` é escrita por mais de um pixel, e `a` é apenas
lido, **não existe condição de corrida** no laço de convolução. Não são
necessárias diretivas `critical` nem `atomic` — o que é a melhor notícia
possível para a paralelização, porque essas construções serializam trechos
do código e corroem o ganho.

Isso só é verdade porque entrada e saída são **buffers separados**. Se o
algoritmo escrevesse sobre a própria entrada, o cálculo de um pixel passaria
a depender de pixels já atualizados por outro, criando dependência de dados
e tornando o resultado dependente da ordem de execução.

**Entre passes**, a dependência é estrita e sequencial: o passe `p+1` lê o
resultado completo do passe `p`. Nenhum pixel do passe seguinte pode ser
calculado antes que todos os pixels do passe atual estejam prontos. Essa é
a barreira de sincronização que separa os passes, e o laço de passes não é
paralelizável.

Resumindo: **o paralelismo está dentro de cada passe, não entre eles.**

---

## 5. Valor agregado: o checksum

Depois de todos os passes, o programa calcula:

```cpp
double soma = 0;
for (int i = 0; i < N*N; i++) soma += a[i];
```

É uma **redução**: todos os elementos colapsam num único valor. É o único
ponto do programa com dependência entre iterações — cada passo acumula
sobre o resultado anterior.

Na paralelização, este trecho vira:

- **OpenMP:** `reduction(+:soma)`, que dá a cada thread uma cópia privada do
  acumulador e combina as cópias ao final.
- **MPI:** uma soma parcial por processo, combinada com `MPI_Reduce`.

Note que `soma` é `double`, enquanto a imagem é de `float`. A acumulação em
precisão dupla reduz o erro de arredondamento ao somar milhões de termos.

### Atenção à comparação do checksum

A soma em ponto flutuante **não é associativa**: mudar a ordem das parcelas
pode alterar os últimos dígitos do resultado. Paralelizar a redução muda
exatamente isso — cada thread ou processo soma um pedaço, e as parciais são
combinadas em outra ordem.

Como o checksum é impresso com duas casas decimais sobre um valor da ordem
de 5 × 10⁸, uma diferença de arredondamento pode aparecer no valor impresso.
A validação das versões paralelas, portanto, deve comparar os checksums com
**tolerância numérica**, e não por igualdade exata de texto.

---

## 6. Resumo para a paralelização

| Aspecto | Conclusão |
|---|---|
| Hotspot | O laço de convolução (98,68% do tempo em N=2000, P=30) |
| Paralelismo | Espacial, dentro de cada passe |
| Dependência | Sequencial entre passes — barreira obrigatória |
| Condição de corrida no cálculo | Não existe, graças aos buffers separados |
| Redução | Apenas no checksum, ao final |
| Bordas | Nunca calculadas, e alternam conforme a paridade dos passes |
| Limite prático | Largura de banda de memória: 32 MB em N=2000 não cabe em cache |