#!/usr/bin/env bash
# Medicao de desempenho conforme o protocolo definido em
# docs/protocolo-medicao.md (Issue #7).
#
# Uso:  bash scripts/medir.sh
#
# Por configuracao: 6 execucoes, a primeira descartada como aquecimento,
# as 5 seguintes gravadas no CSV. Uma linha por execucao valida.
#
# Os blocos de OpenMP e MPI entram na Issue #22, quando as versoes
# paralelas existirem.

set -euo pipefail

PASSES=30
TAMANHOS="500 1000 2000"
REPETICOES=6          # 1 de aquecimento + 5 validas
DESCARTAR=1

SAIDA_DIR=dados/medicoes
GABARITO_DIR=dados/gabarito

BIN_SEQ=bin/convolucao_seq

mkdir -p "$SAIDA_DIR"

# ---------------------------------------------------------------
# Extrai um campo da saida do programa.
#   saida: "N=2000 passes=30  checksum=508325215.06  tempo=0.347860 s"
# ---------------------------------------------------------------
extrai_checksum() { sed -E 's/.*checksum=([0-9.]+).*/\1/' <<< "$1"; }
extrai_tempo()    { sed -E 's/.*tempo=([0-9.]+) s.*/\1/'   <<< "$1"; }

# ---------------------------------------------------------------
# Checksum esperado, lido do gabarito gerado na Issue #3.
# ---------------------------------------------------------------
checksum_esperado() {
    local N=$1
    local arq="$GABARITO_DIR/seq_N${N}_P${PASSES}.txt"
    [ -f "$arq" ] || { echo "SEM_GABARITO"; return; }
    extrai_checksum "$(cat "$arq")"
}

# ---------------------------------------------------------------
# Mede uma versao para um tamanho e grava as execucoes validas.
#   $1 nome da versao  $2 unidades  $3 N  $4.. comando
# ---------------------------------------------------------------
mede() {
    local versao=$1 unidades=$2 N=$3
    shift 3
    local esperado
    esperado=$(checksum_esperado "$N")

    local r valida=0
    for r in $(seq 1 "$REPETICOES"); do
        local saida tempo checksum
        saida=$("$@")
        tempo=$(extrai_tempo "$saida")
        checksum=$(extrai_checksum "$saida")

        if [ "$r" -le "$DESCARTAR" ]; then
            printf "  [aquecimento] %s s\n" "$tempo"
            continue
        fi

        valida=$((valida + 1))
        printf "  [%d/%d] %s s  checksum=%s\n" \
            "$valida" "$((REPETICOES - DESCARTAR))" "$tempo" "$checksum"

        if [ "$esperado" != "SEM_GABARITO" ] && [ "$checksum" != "$esperado" ]; then
            echo "  AVISO: checksum diverge do gabarito ($esperado)" >&2
        fi

        echo "$versao,$N,$PASSES,$unidades,$valida,$tempo,$checksum" \
            >> "$SAIDA_DIR/${versao}.csv"
    done
}

# ---------------------------------------------------------------
# Sequencial
# ---------------------------------------------------------------
[ -x "$BIN_SEQ" ] || { echo "Compile primeiro: make sequencial"; exit 1; }

echo "versao,N,passes,unidades,repeticao,tempo_s,checksum" \
    > "$SAIDA_DIR/sequencial.csv"

echo "=============================================="
echo " Medicao do baseline sequencial"
echo "=============================================="
echo "Data:       $(date -u '+%Y-%m-%d %H:%M:%S UTC')"
echo "Compilador: $(g++ --version | head -1)"
echo "CPUs:       $(nproc)"
echo "Passes:     $PASSES"
echo "Protocolo:  $REPETICOES execucoes, $DESCARTAR descartada(s)"
echo

for N in $TAMANHOS; do
    echo "--- N = $N ---"
    mede sequencial 1 "$N" "$BIN_SEQ" "$N" "$PASSES"
    echo
done

echo "CSV gravado em $SAIDA_DIR/sequencial.csv"