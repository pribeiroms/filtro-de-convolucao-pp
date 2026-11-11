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
| `scripts/` | Geração do gabarito, medição e gráficos |
| `dados/gabarito/` | Saídas de referência do sequencial |
| `dados/medicoes/` | CSVs produzidos pelas medições |
| `graficos/` | Gráficos de speedup, eficiência e escalabilidade |
| `docs/` | Documentos de entrega e registro do ambiente |
| `referencia/` | Material fornecido pelo professor (não é trabalho do grupo) |
| `bin/` | Binários compilados (não versionado) |

---

## Como rodar do zero

O repositório é autossuficiente: basta clonar e subir o contêiner. Nada
fora deste repositório é necessário.

### 1. Clonar

```bash
git clone https://github.com/pribeiroms/filtro-de-convolucao-pp.git
cd filtro-de-convolucao-pp
```

### 2. Subir o ambiente

Requisito: Docker e Docker Compose instalados.

```bash
docker compose build                   # só na primeira vez
docker compose run --rm projeto        # abre um shell em /lab
```

O ambiente é Ubuntu 24.04 com `g++`, OpenMPI, `matplotlib` e `pandas` — o
mesmo conjunto do ambiente oficial da disciplina, para que os resultados
sejam comparáveis aos do laboratório.

No VS Code, a extensão Dev Containers abre o projeto direto no contêiner
pela configuração em `.devcontainer/`.

Alternativa sem Docker: em qualquer Linux com `g++` e OpenMPI instalados,
os comandos abaixo funcionam sem alteração.

### 3. Compilar

```bash
make
```

Os binários vão para `bin/`. Para compilar só uma versão: `make sequencial`,
`make openmp` ou `make mpi`.

### 4. Executar

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

### 5. Gerar o gabarito

O gabarito são as saídas de referência do sequencial, usadas para validar
as versões paralelas. Já está versionado em `dados/gabarito/`; só precisa
ser regerado se o código sequencial fornecido mudar.

```bash
./scripts/gerar_gabarito.sh
```

O script roda o sequencial para N = 500, 1000 e 2000 com 30 passes e grava
um arquivo por tamanho. O tempo de execução é descartado de propósito: ele
varia a cada execução e não faz parte do resultado.

### 6. Medir e gerar gráficos

```bash
./scripts/medir.sh                 # produz dados/medicoes/*.csv
python3 scripts/gerar_graficos.py  # produz graficos/*.png
```

---

## Perfilamento

A pasta `sequencial/` traz também `04_convolucao_instrumentado.cpp`, uma
cópia com medição de tempo por trecho — **não é o gabarito** e não deve
ser usada para validação. Ela serviu para identificar o hotspot:

```bash
make instrumentado
./bin/convolucao_instr 2000 30
```

## OpenMP: compilacao, medicao e corretude

```bash
make openmp
OMP_NUM_THREADS=8 ./bin/convolucao_omp 2000 30
# Argumentos opcionais: N passes threads schedule chunk
./bin/convolucao_omp 2000 30 8 dynamic 64
```

O codigo, a analise de compartilhamento e corridas, e a interpretacao dos
experimentos estao em [docs/openmp.md](docs/openmp.md). Na maquina do
laboratorio, execute `bash scripts/medir_openmp.sh`: o script segue as seis
execucoes por configuracao do protocolo, valida cada checksum com o gabarito,
grava os dados brutos em `dados/medicoes/openmp*.csv` e gera
`graficos/openmp_speedup.svg`. Para medir threads adicionais, defina
`THREADS_EXTRA="12 20"`. Os artefatos dependem da maquina onde a medicao roda.
O script tambem gera a tabela de medianas dos schedules em
`docs/openmp_resultados.md`.

---

## Material de referência

A pasta `referencia/` contém os códigos de exemplo e o script de medição
**fornecidos pelo professor**. Nada ali é de autoria do grupo. Para
compilá-los:

```bash
make referencia
```

---

## Observações sobre o ambiente

- Dentro do contêiner o `mpirun` roda como root sem flag extra, porque a
  imagem define `OMPI_ALLOW_RUN_AS_ROOT`.
- Tempos medidos em máquinas diferentes não são comparáveis entre si. Cada
  curva de speedup precisa ser construída com todas as medições feitas na
  mesma máquina. Veja [docs/ambiente.md](docs/ambiente.md).

---

## Regra inviolável

O arquivo `sequencial/04_convolucao.cpp` é o código entregue pelo
professor. **A lógica de cálculo e a entrada não podem ser alteradas**
em hipótese alguma — ele é a referência contra a qual tudo é medido e
validado.