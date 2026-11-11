#!/usr/bin/env bash
# Validacao de corretude das versoes paralelas (Issue #21).
#
# Uso:  bash scripts/validar.sh
#
# Compara o checksum de cada versao com a referencia, em todos os tamanhos e
# em todos os graus de paralelismo. Termina com codigo 1 se alguma
# configuracao divergir.
#
# Duas baterias:
#   1. P = 30, contra o gabarito versionado em dados/gabarito/ (Issue #3).
#   2. P = 31, contra o sequencial executado na hora.
#
# A segunda existe por causa das bordas. O codigo de referencia nao recalcula
# a moldura da imagem e troca os dois buffers a cada passe, de modo que a
# borda do resultado alterna entre os valores originais e zero conforme a
# paridade do numero de passes. Uma versao paralela que trate a borda de
# outra forma passa despercebida com P par e falha com P impar.
#
# A comparacao e por igualdade exata, nao por tolerancia. As duas versoes
# paralelas somam o checksum na mesma ordem do sequencial -- o OpenMP fora da
# regiao paralela, o MPI no processo 0 sobre a imagem ja reunida --, entao o
# resultado tem de ser identico, bit a bit.

set -uo pipefail

PASSES_PAR=30
PASSES_IMPAR=31
TAMANHOS="500 1000 2000"
UNIDADES="1 2 4 8 16"

GABARITO_DIR=dados/gabarito
BIN_SEQ=bin/convolucao_seq
BIN_OMP=bin/convolucao_omp
BIN_MPI=bin/convolucao_mpi

for b in "$BIN_SEQ" "$BIN_OMP" "$BIN_MPI"; do
    [ -x "$b" ] || { echo "Compile primeiro: make all"; exit 1; }
done

extrai_checksum() { sed -nE 's/.*checksum=([0-9.]+).*/\1/p' <<< "$1" | head -1; }

CPUS=$(lscpu -p=Core,Socket | grep -v '^#' | sort -u | wc -l)

ok=0
falhas=0

# $1 rotulo  $2 esperado  $3.. comando
confere() {
    local rotulo=$1 esperado=$2
    shift 2
    local obtido
    obtido=$(extrai_checksum "$("$@" 2>/dev/null)")
    if [ "$obtido" = "$esperado" ]; then
        printf "  OK     %-34s %s\n" "$rotulo" "$obtido"
        ok=$((ok + 1))
    else
        printf "  FALHA  %-34s %s (esperado %s)\n" "$rotulo" "${obtido:-sem saida}" "$esperado"
        falhas=$((falhas + 1))
    fi
}

roda_mpi() {
    local np=$1 N=$2 P=$3
    if [ "$np" -gt "$CPUS" ]; then
        mpirun --oversubscribe -np "$np" "$BIN_MPI" "$N" "$P"
    else
        mpirun -np "$np" "$BIN_MPI" "$N" "$P"
    fi
}

echo "=============================================="
echo " Validacao de corretude"
echo "=============================================="
echo "Nucleos fisicos: $CPUS"
echo

# ---------------------------------------------------------------
# Bateria 1: P par, contra o gabarito versionado
# ---------------------------------------------------------------
echo "--- P = $PASSES_PAR, referencia: dados/gabarito/ ---"
for N in $TAMANHOS; do
    arq="$GABARITO_DIR/seq_N${N}_P${PASSES_PAR}.txt"
    if [ ! -f "$arq" ]; then
        echo "  gabarito ausente para N=$N; gere com scripts/gerar_gabarito.sh" >&2
        falhas=$((falhas + 1))
        continue
    fi
    esperado=$(extrai_checksum "$(cat "$arq")")
    echo "  N = $N  (referencia $esperado)"

    confere "sequencial" "$esperado" "$BIN_SEQ" "$N" "$PASSES_PAR"
    for u in $UNIDADES; do
        confere "openmp, $u thread(s)" "$esperado" \
            env OMP_NUM_THREADS="$u" "$BIN_OMP" "$N" "$PASSES_PAR" "$u"
    done
    for u in $UNIDADES; do
        confere "mpi, $u processo(s)" "$esperado" roda_mpi "$u" "$N" "$PASSES_PAR"
    done
    echo
done

# ---------------------------------------------------------------
# Bateria 2: P impar, referencia calculada na hora
# ---------------------------------------------------------------
echo "--- P = $PASSES_IMPAR, referencia: sequencial executado agora ---"
echo "    (verifica a alternancia das bordas)"
for N in $TAMANHOS; do
    esperado=$(extrai_checksum "$("$BIN_SEQ" "$N" "$PASSES_IMPAR")")
    echo "  N = $N  (referencia $esperado)"

    for u in $UNIDADES; do
        confere "openmp, $u thread(s)" "$esperado" \
            env OMP_NUM_THREADS="$u" "$BIN_OMP" "$N" "$PASSES_IMPAR" "$u"
    done
    for u in $UNIDADES; do
        confere "mpi, $u processo(s)" "$esperado" roda_mpi "$u" "$N" "$PASSES_IMPAR"
    done
    echo
done

echo "=============================================="
echo " $ok configuracao(oes) correta(s), $falhas divergente(s)"
echo "=============================================="

[ "$falhas" -eq 0 ] || exit 1