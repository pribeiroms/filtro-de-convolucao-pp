#!/usr/bin/env bash
# Medicao de escalabilidade fraca (Issue #23).
#
# Uso:  bash scripts/medir_fraca.sh
#
# Na escalabilidade fraca a carga por unidade fica constante: como o custo e
# O(N^2), o lado da imagem cresce com a raiz do numero de unidades.
#
#   1 unidade  -> N =  500     4 unidades -> N = 1000    16 unidades -> N = 2000
#   2 unidades -> N =  707     8 unidades -> N = 1414
#
# Cada par (unidades, N) da 250 mil pixels por unidade.
#
# Protocolo (docs/protocolo-medicao.md): 6 execucoes por configuracao, a
# primeira descartada, as 5 seguintes gravadas.
#
# Grava dados/medicoes/escalabilidade_fraca.csv.

set -euo pipefail

PASSES=30
REPETICOES=6
DESCARTAR=1

# pares "unidades:N"
PARES="1:500 2:707 4:1000 8:1414 16:2000"

SAIDA_DIR=dados/medicoes
SAIDA=$SAIDA_DIR/escalabilidade_fraca.csv

BIN_SEQ=bin/convolucao_seq
BIN_OMP=bin/convolucao_omp
BIN_MPI=bin/convolucao_mpi

for b in "$BIN_SEQ" "$BIN_OMP" "$BIN_MPI"; do
    [ -x "$b" ] || { echo "Compile primeiro: make all"; exit 1; }
done

mkdir -p "$SAIDA_DIR"

# A saida do MPI tem duas linhas; o -n com /p imprime so a que casa.
extrai_checksum() { sed -nE 's/.*checksum=([0-9.]+).*/\1/p' <<< "$1" | head -1; }
extrai_tempo()    { sed -nE 's/.*tempo=([0-9.]+) s.*/\1/p'   <<< "$1" | head -1; }

CPUS=$(lscpu -p=Core,Socket | grep -v '^#' | sort -u | wc -l)

echo "versao,N,passes,unidades,repeticao,tempo_s,checksum" > "$SAIDA"

echo "=============================================="
echo " Escalabilidade fraca"
echo "=============================================="
echo "Data:       $(date -u '+%Y-%m-%d %H:%M:%S UTC')"
echo "Passes:     $PASSES"
echo "Nucleos:    $CPUS fisicos"
echo "Protocolo:  $REPETICOES execucoes, $DESCARTAR descartada(s)"
echo

falhas=0

# Referencia de corretude: o sequencial define o checksum correto para cada
# N. Os tamanhos 707 e 1414 nao estao no gabarito da Issue #3, que cobre
# apenas 500, 1000 e 2000, entao a referencia e calculada aqui.
mede() {
    local versao=$1 unidades=$2 N=$3 esperado=$4
    shift 4

    local r valida=0
    for r in $(seq 1 "$REPETICOES"); do
        local saida tempo checksum
        saida=$("$@")
        tempo=$(extrai_tempo "$saida")
        checksum=$(extrai_checksum "$saida")

        if [ "$r" -le "$DESCARTAR" ]; then
            printf "    [aquecimento] %s s\n" "$tempo"
            continue
        fi

        valida=$((valida + 1))
        printf "    [%d/%d] %s s  checksum=%s\n" \
            "$valida" "$((REPETICOES - DESCARTAR))" "$tempo" "$checksum"

        if [ "$checksum" != "$esperado" ]; then
            echo "    ERRO: checksum diverge do sequencial ($esperado)" >&2
            falhas=$((falhas + 1))
        fi

        echo "$versao,$N,$PASSES,$unidades,$valida,$tempo,$checksum" >> "$SAIDA"
    done
}

for par in $PARES; do
    n=${par%%:*}
    N=${par##*:}
    echo "--- $n unidade(s), N = $N ---"

    # checksum de referencia para este N
    ref=$(extrai_checksum "$($BIN_SEQ "$N" "$PASSES")")
    echo "  referencia do sequencial: $ref"

    echo "  sequencial"
    mede sequencial 1 "$N" "$ref" "$BIN_SEQ" "$N" "$PASSES"

    echo "  openmp ($n threads)"
    mede openmp "$n" "$N" "$ref" \
        env OMP_NUM_THREADS="$n" "$BIN_OMP" "$N" "$PASSES" "$n" static 1

    echo "  mpi ($n processos)"
    if [ "$n" -gt "$CPUS" ]; then
        mede mpi "$n" "$N" "$ref" \
            mpirun --oversubscribe -np "$n" "$BIN_MPI" "$N" "$PASSES"
    else
        mede mpi "$n" "$N" "$ref" \
            mpirun -np "$n" "$BIN_MPI" "$N" "$PASSES"
    fi
    echo
done

echo "CSV gravado em $SAIDA"

if [ "$falhas" -gt 0 ]; then
    echo "FALHA: $falhas execucao(oes) com checksum divergente." >&2
    exit 1
fi