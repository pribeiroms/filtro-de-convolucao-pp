# Filtro de Convolução 2D (blur 3×3) — OpenMP e MPI

Projeto da disciplina **Programação Paralela (CCO085)** — IESB, 2026/2.
Paralelização de um filtro de convolução 2D fornecido pelo professor,
usando OpenMP (memória compartilhada) e MPI (memória distribuída),
preservando exatamente o resultado da versão sequencial.

Veja [AUTORES.md](AUTORES.md) para os integrantes e suas frentes.

---

## O problema

O programa aplica `P` passes de um filtro de média 3×3 sobre uma imagem
sintética de `N × N` pixels. A cada passe, todo pixel interno recebe a
média dos seus 9 vizinhos; as bordas não são recalculadas. Ao final,
soma-se toda a matriz para obter um **checksum**, que é o critério de
corretude: as versões paralelas têm de produzir o mesmo checksum do
sequencial.

---

## Estrutura do repositório

| Pasta | Conteúdo |
|---|---|
| `sequencial/` | Código de referência do professor, sem alteração |
| `openmp/` | Versão paralela com threads |
| `mpi/` | Versão paralela com processos |
| `scripts/` | Medição (gera CSV) e geração dos gráficos |
| `dados/gabarito/` | Saídas de referência do sequencial |
| `dados/medicoes/` | CSVs produzidos pelas medições |
| `graficos/` | Gráficos de speedup, eficiência e escalabilidade |
| `docs/` | Documentos de entrega |
| `bin/` | Binários compilados (não versionado) |

---

## Como rodar do zero

### 1. Ambiente

O ambiente de referência é o do laboratório: Linux, `g++` e OpenMPI.
Em casa, usamos o contêiner Docker da disciplina (Roteiro B).

Com o contêiner, esta pasta fica visível em `/lab/projeto`. Isso exige
que o `docker-compose.yml` do ambiente monte o volume correspondente:

```yaml
volumes:
  - ../projeto:/lab/projeto
```

Confira que as ferramentas estão presentes:

```bash
g++ --version
mpic++ --version
mpirun --version
```

### 2. Compilar

```bash
make
```

Os binários vão para `bin/`. Para compilar só uma versão: `make seq`,
`make omp` ou `make mpi`.

### 3. Executar

Os dois argumentos são `N` (lado da imagem) e `P` (número de passes).

```bash
# Sequencial
./bin/convolucao_seq 2000 30

# OpenMP — o número de threads vem da variável de ambiente
OMP_NUM_THREADS=8 ./bin/convolucao_omp 2000 30

# MPI — o número de processos vem do mpirun
mpirun -np 4 ./bin/convolucao_mpi 2000 30
```

As três execuções devem imprimir **o mesmo checksum**.

### 4. Gerar o gabarito

O gabarito são as saídas do sequencial, usadas para validar as versões
paralelas. Gere uma vez e não altere mais:

```bash
mkdir -p dados/gabarito
for N in 500 1000 2000; do
  ./bin/convolucao_seq $N 30 > dados/gabarito/seq_${N}_30.txt
done
```

### 5. Medir e gerar gráficos

```bash
./scripts/medir.sh                 # produz dados/medicoes/*.csv
python3 scripts/gerar_graficos.py  # produz graficos/*.png
```

---

## Perfilamento

A pasta `sequencial/` traz também `04_convolucao_instrumentado.cpp`, uma
cópia com medição de tempo por trecho — não deve
ser usada para validação. Ela serviu para identificar o hotspot:

```bash
make instr
./bin/convolucao_instr 2000 30
```

---

## Regra inviolável

O arquivo `sequencial/04_convolucao.cpp` é o código base. **A lógica de cálculo e a entrada não podem ser alteradas**
