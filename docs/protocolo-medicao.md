# Protocolo de medição

Regras que valem para **todas** as medições do projeto — baseline
sequencial, OpenMP e MPI —, conforme a Issue #7.

O objetivo do protocolo é garantir que os tempos comparados entre as três
versões tenham sido obtidos da mesma forma. Sem isso, qualquer curva de
speedup é inválida, porque a diferença medida pode vir do método e não da
paralelização.

---

## 1. O que é medido


O tempo medido é o da **resolução completa do problema** em cada versão:
desde que os dados estão prontos para serem processados até que o resultado
final esteja consolidado e o checksum possa ser calculado.

Concretamente, em cada versão:

| Versão | Início da medição | Fim da medição |
|---|---|---|
| Sequencial | Antes do laço de passes | Depois do laço de passes |
| OpenMP | Antes do laço de passes | Depois do laço de passes |
| MPI | Antes da distribuição dos dados | Depois da coleta do resultado |

**Ficam de fora** em todas as versões: a geração da imagem de entrada e o
cálculo do checksum. Juntos eles representam cerca de 1,3% do tempo
(ver [perfilamento.md](perfilamento.md)) e não são alvo da paralelização.

**Fica de fora também** a inicialização do ambiente MPI (`MPI_Init`) e o seu
encerramento, por serem custo de infraestrutura e não de resolução do
problema.

### Por que incluir a comunicação do MPI

A distribuição e a coleta dos dados não existem no sequencial, mas são
trabalho necessário para que a versão MPI resolva o mesmo problema. Excluí-las
da medição produziria um speedup que não corresponde ao custo real de usar
MPI — seria comparar o problema inteiro resolvido sequencialmente com apenas
a parte conveniente dele resolvida em paralelo.

Com a comunicação incluída, o overhead aparece naturalmente na curva: é
esperado que o speedup do MPI cresça menos que o do OpenMP, e que em
tamanhos pequenos ele chegue a piorar com mais processos. Esse é o
comportamento que as Issues #19 e #20 analisam, e escondê-lo tiraria o
sentido delas.

---

## 2. Repetições e estatística

- **6 execuções** por configuração: a primeira é descartada como aquecimento
  e as **5 seguintes** são as medições válidas.
- O valor reportado é a **mediana** das 5 medições válidas.
- São registrados também o **mínimo** e o **máximo**, para documentar a
  dispersão.

Por que mediana e não média: a média é puxada por uma execução atípica, que
em medição de desempenho aparece com frequência — basta o sistema
operacional escalonar outra tarefa no meio. A mediana ignora os extremos.

Por que 5 e não 3: o perfilamento mostrou dispersão abaixo de 2% entre
execuções, mas essa estabilidade foi observada no sequencial. As versões
paralelas tendem a variar mais, porque dependem de escalonamento de threads
e de comunicação entre processos. Cinco medições dão margem para a mediana
ser confiável também nelas.


---

## 3. Descarte de aquecimento

A primeira execução de cada configuração é **sempre descartada**.

A justificativa é concreta: na primeira execução, os vetores recém-alocados
ainda não têm páginas de memória efetivamente atribuídas pelo sistema
operacional. O primeiro acesso a cada página provoca uma falha de página, e
esse custo entra no tempo medido. Além disso, caches e preditores de desvio
começam vazios. A partir da segunda execução, o ambiente já está estabilizado.

---

## 4. Tamanhos de entrada e kernel

Três tamanhos, mantidos em todas as versões:

| Rótulo | N | Pixels | Memória dos dois buffers |
|---|---|---|---|
| Pequeno | 500 | 250 mil | ~2 MB |
| Médio | 1000 | 1 milhão | ~8 MB |
| Grande | 2000 | 4 milhões | ~32 MB |

**Número de passes: 30** em todos os tamanhos, como no baseline do Marco 1.

**Kernel: 3×3 fixo.** O kernel é definido pelo código de referência do
professor e não pode ser alterado — a lógica de cálculo é invariante do
projeto.

Os três tamanhos foram escolhidos para atravessar a hierarquia de memória da
máquina de medição, que tem 16 MiB de cache L3: o pequeno cabe
confortavelmente em cache, o médio cabe, e o grande não cabe. Isso permite
observar se o speedup se comporta de forma diferente quando o problema passa
a depender da memória principal.

---

## 5. Graus de paralelismo

Para cada tamanho de entrada, as versões paralelas são medidas com:

```
1, 2, 4, 8 e 16 unidades
```

onde "unidade" é thread no OpenMP (`OMP_NUM_THREADS`) e processo no MPI
(`mpirun -np`).

A máquina de medição tem 8 núcleos físicos e 16 CPUs lógicas. A medição vai
até 16 justamente para mostrar o que acontece além do número de núcleos
reais: espera-se que o ganho de 8 para 16 seja pequeno, porque as duas
threads de um mesmo núcleo físico disputam as mesmas unidades de execução.

Configurações com mais processos que CPUs disponíveis exigem
`--oversubscribe` no `mpirun`, e esse fato deve ser registrado junto da
medição.

---

## 6. Máquina e cuidados durante a medição

Todas as medições de uma mesma curva devem ser feitas **na mesma máquina, na
mesma sessão**. Tempos de máquinas diferentes não são comparáveis e não podem
ser misturados num mesmo gráfico.

A máquina e as versões de ferramentas estão registradas em
[ambiente.md](ambiente.md).

Cuidados a observar:

- Fechar navegador, editores e demais aplicativos antes de medir.
- Nenhum outro contêiner em execução.
- Notebook ligado na tomada e em modo de desempenho — em modo de economia de
  bateria a frequência do processador é reduzida e os tempos ficam
  inconsistentes entre execuções.
- Não usar a máquina durante a medição.
- Rodar a bateria completa de medições de uma vez, para que todas as
  configurações enfrentem as mesmas condições térmicas.

---

## 7. Formato dos dados

As medições são gravadas em CSV em `dados/medicoes/`, com uma linha por
execução válida:

```
versao,N,passes,unidades,repeticao,tempo_s,checksum
```

| Coluna | Conteúdo |
|---|---|
| `versao` | `sequencial`, `openmp` ou `mpi` |
| `N` | Lado da imagem |
| `passes` | Número de passes (30) |
| `unidades` | Threads ou processos; 1 para o sequencial |
| `repeticao` | 1 a 5 (a execução de aquecimento não é gravada) |
| `tempo_s` | Tempo medido em segundos |
| `checksum` | Checksum produzido, para rastrear corretude junto do tempo |

Gravar as execuções individuais, e não apenas a mediana, é deliberado: a
mediana pode ser recalculada a partir dos dados brutos, mas o contrário é
impossível. O CSV é a evidência; as tabelas e os gráficos são derivados dele.

Incluir o checksum em cada linha permite detectar, ao analisar os dados, uma
configuração que produziu tempo bom mas resultado errado.

---

## 8. Cálculo do speedup e da eficiência

O speedup de uma configuração com `n` unidades é:

```
S(n) = T_sequencial / T_paralelo(n)
```

A eficiência é:

```
E(n) = S(n) / n
```

### A armadilha a evitar

`T_sequencial` é o tempo da **versão sequencial medida**, e não o tempo da
versão paralela rodando com uma única unidade.

Usar a versão paralela com 1 thread como referência infla o speedup, porque
esse tempo já carrega o custo de criação de threads e das diretivas de
paralelização. O resultado é uma curva que parece melhor do que a realidade,
e que esconde justamente o overhead que a análise deveria expor.

Por isso o CSV registra a versão sequencial como linhas próprias, com
`versao=sequencial` e `unidades=1`: ela é o denominador de todas as contas.

---

## 9. Validação junto da medição

Toda execução registra o checksum. Uma configuração só é considerada válida
se o checksum conferir com o gabarito em `dados/gabarito/`.

A comparação deve ser feita com **tolerância numérica**, e não por igualdade
exata de texto. A soma em ponto flutuante não é associativa: paralelizar a
redução muda a ordem das parcelas e pode alterar os últimos dígitos do
resultado, mesmo numa implementação correta. O critério de tolerância é
definido na Issue #21.

Tempo de uma configuração cujo resultado está errado não é dado de
desempenho — é apenas um número. Medição e validação andam juntas.