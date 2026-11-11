# Validação de corretude

Verificação de que as versões OpenMP e MPI produzem exatamente o mesmo
resultado do código sequencial de referência, conforme a Issue #21.

Script: `scripts/validar.sh`.

---

## 1. Critério

O programa de referência imprime um **checksum**: a soma de todos os
elementos da matriz após os passes. É o único valor observável do resultado,
e por isso o critério de corretude.

A comparação é por **igualdade exata**, não por tolerância numérica.

Essa escolha merece justificativa, porque a soma em ponto flutuante não é
associativa: mudar a ordem das parcelas pode alterar os últimos dígitos do
resultado. Paralelizar uma redução costuma mudar exatamente isso, e nesse
caso a comparação exata falharia mesmo com a implementação correta.

Aqui ela não falha, porque **nenhuma das duas versões paraleliza a soma do
checksum**:

- Na versão OpenMP, o checksum é somado depois do laço de passes, fora de
  qualquer região paralela, percorrendo a matriz na mesma ordem do
  sequencial.
- Na versão MPI, o processo 0 reúne a imagem inteira com `MPI_Gatherv` e só
  então soma, também na mesma ordem.

As duas decisões são deliberadas. Paralelizar a redução daria um ganho
desprezível — o checksum é 0,8% do tempo, conforme o
[perfilamento](perfilamento.md) — e custaria a possibilidade de comparar o
resultado por igualdade exata.

---

## 2. Duas baterias

### P = 30, contra o gabarito versionado

Compara com os arquivos de `dados/gabarito/`, gerados uma única vez pelo
sequencial na Issue #3.

### P = 31, contra o sequencial executado na hora

Esta bateria existe por causa das bordas.

O código de referência não recalcula a moldura da imagem: os laços vão de 1
a `N−2`. Como ele troca os dois buffers a cada passe (`a.swap(b)`) e o
buffer `b` nasce zerado, a borda do resultado **alterna** conforme a paridade
do número de passes — valores originais da imagem com `P` par, zeros com `P`
ímpar. A análise completa está em [algoritmo.md](algoritmo.md), seção 2.

A consequência para a validação é direta: uma versão paralela que trate a
borda de outra forma — zerando-a sempre, replicando-a ou recalculando-a —
pode passar despercebida com `P = 30` e falhar com `P = 31`. Validar apenas
com um número par de passes deixaria essa classe de erro invisível.

As bordas pesam de 0,2% a 0,8% do checksum, dependendo do tamanho, o que é
mais do que suficiente para alterar o valor impresso.

---

## 3. Cobertura

Para cada uma das duas baterias:

| Dimensão | Valores |
|---|---|
| Tamanhos | N = 500, 1000, 2000 |
| OpenMP | 1, 2, 4, 8 e 16 threads |
| MPI | 1, 2, 4, 8 e 16 processos |

São 33 configurações com `P = 30` e 30 com `P = 31`, totalizando **63
verificações**.

Configurações com mais processos que núcleos físicos recebem
`--oversubscribe` automaticamente.

---

## 4. Resultado

Todas as 63 configurações produziram checksum idêntico à referência.

O mesmo resultado se repetiu nas três sessões completas de medição descritas
na seção 7 da [análise experimental](analise-experimental.md): nenhuma
execução, em nenhuma configuração, divergiu.

Isso vale também para as configurações com divisão não exata do domínio no
MPI, em que os primeiros processos recebem uma linha a mais que os demais.

---

## 5. Validação durante a medição

Além deste script, os três scripts de medição conferem o checksum de **cada
execução** contra o gabarito enquanto medem, e interrompem com erro se algum
divergir. O `scripts/resumir.py` avisa quando uma mesma configuração produziu
checksums diferentes entre repetições, o que seria sintoma de não
determinismo.

Tempo de uma configuração cujo resultado está errado não é dado de
desempenho, e essa verificação impede que um número assim chegue às tabelas.

---

## 6. Como reproduzir

```bash
make all
bash scripts/validar.sh
```

O script termina com código de saída 1 se alguma configuração divergir, o
que permite usá-lo como verificação automática.