# ─────────────────────────────────────────────────────────────────────
# Filtro de Convolução 2D (blur 3x3) — CCO085 2026/2 — IESB
# Compila as três versões: sequencial, OpenMP e MPI.
#
#   make           compila tudo que já existe
#   make seq       só o sequencial (gabarito)
#   make instr     só o sequencial instrumentado (perfilamento)
#   make omp       só a versão OpenMP
#   make mpi       só a versão MPI
#   make clean     apaga os binários
# ─────────────────────────────────────────────────────────────────────

CXX      := g++
MPICXX   := mpic++
CXXFLAGS := -O2 -Wall
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

.PHONY: all seq instr omp mpi clean
all: $(ALVOS)

seq:   $(BIN)/convolucao_seq
instr: $(BIN)/convolucao_instr
omp:   $(BIN)/convolucao_omp
mpi:   $(BIN)/convolucao_mpi

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

clean:
	rm -rf $(BIN)