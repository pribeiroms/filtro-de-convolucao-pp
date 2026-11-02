# ─────────────────────────────────────────────────────────────────────
# Filtro de Convolução 2D (blur 3x3) — CCO085 2026/2 — IESB
#
#   make                 compila as versões do projeto
#   make sequencial      só o sequencial (gabarito)
#   make instrumentado   só o sequencial instrumentado (perfilamento)
#   make openmp          só a versão OpenMP
#   make mpi             só a versão MPI
#   make referencia      compila os exemplos do professor em referencia/
#   make clean           apaga os binários do projeto
#   make clean-tudo      apaga também os binários de referencia/
# ─────────────────────────────────────────────────────────────────────

# Projeto (C++)
CXX      := g++
MPICXX   := mpic++
CXXFLAGS := -O2 -Wall

# Exemplos de referência (C)
CC       := gcc
MPICC    := mpicc
CFLAGS   := -O2 -Wall
LDLIBS   := -lm

OMPFLAGS := -fopenmp

BIN := bin

SEQ_SRC   := sequencial/04_convolucao.cpp
INSTR_SRC := sequencial/04_convolucao_instrumentado.cpp
OMP_SRC   := openmp/convolucao_omp.cpp
MPI_SRC   := mpi/convolucao_mpi.cpp

# "make" compila apenas o que já foi escrito. Enquanto as versões
# paralelas não existirem, o alvo simplesmente não entra na lista —
# assim ninguém esbarra em erro de arquivo inexistente.
ALVOS :=
ifneq ($(wildcard $(SEQ_SRC)),)
  ALVOS += $(BIN)/convolucao_seq
endif
ifneq ($(wildcard $(INSTR_SRC)),)
  ALVOS += $(BIN)/convolucao_instr
endif
ifneq ($(wildcard $(OMP_SRC)),)
  ALVOS += $(BIN)/convolucao_omp
endif
ifneq ($(wildcard $(MPI_SRC)),)
  ALVOS += $(BIN)/convolucao_mpi
endif

.PHONY: all sequencial instrumentado openmp mpi referencia clean clean-tudo

all: $(ALVOS)

sequencial:    $(BIN)/convolucao_seq
instrumentado: $(BIN)/convolucao_instr
openmp:        $(BIN)/convolucao_omp
mpi:           $(BIN)/convolucao_mpi

$(BIN):
	mkdir -p $(BIN)

$(BIN)/convolucao_seq: $(SEQ_SRC) | $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BIN)/convolucao_instr: $(INSTR_SRC) | $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BIN)/convolucao_omp: $(OMP_SRC) | $(BIN)
	$(CXX) $(CXXFLAGS) $(OMPFLAGS) -o $@ $<

$(BIN)/convolucao_mpi: $(MPI_SRC) | $(BIN)
	$(MPICXX) $(CXXFLAGS) -o $@ $<

# ─────────────────────────────────────────────────────────────────────
# Exemplos fornecidos pelo professor (referencia/)
#
# São arquivos .c, compilados com gcc/mpicc e ligados à libm. Os binários
# ficam ao lado dos fontes, e o .gitignore cuida de não versioná-los.
# Os códigos de referencia/codigos/aulas/ são .cpp avulsos e não entram
# aqui: compile-os individualmente com g++ quando precisar.
# ─────────────────────────────────────────────────────────────────────

REF      := referencia/codigos
REF_BASE := $(REF)/00_base
REF_OMP  := $(REF)/01_openmp
REF_MPI  := $(REF)/02_mpi

REF_BIN := $(patsubst %.c,%,$(wildcard $(REF_BASE)/*.c)) \
           $(patsubst %.c,%,$(wildcard $(REF_OMP)/*.c)) \
           $(patsubst %.c,%,$(wildcard $(REF_MPI)/*.c))

referencia: $(REF_BIN)

$(REF_BASE)/%: $(REF_BASE)/%.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

$(REF_OMP)/%: $(REF_OMP)/%.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ $< $(LDLIBS)

$(REF_MPI)/%: $(REF_MPI)/%.c
	$(MPICC) $(CFLAGS) -o $@ $< $(LDLIBS)

# ─────────────────────────────────────────────────────────────────────

clean:
	rm -rf $(BIN)

clean-tudo: clean
	rm -f $(REF_BIN)