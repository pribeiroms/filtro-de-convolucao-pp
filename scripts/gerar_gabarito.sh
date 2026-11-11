#!/usr/bin/env bash
# Gera o gabarito: a saída de referência do código sequencial para cada
# tamanho de entrada. É contra estes valores que as versões paralelas são
# validadas (Issue #21).
#
# Uso:  ./scripts/gerar_gabarito.sh
#
# Gere uma vez e não altere mais. O gabarito só mudaria se o código
# sequencial fornecido mudasse — o que não pode acontecer.

set -euo pipefail

BIN=bin/convolucao_seq
SAIDA=dados/gabarito
PASSES=30
TAMANHOS="500 1000 2000"

[ -x "$BIN" ] || { echo "Compile primeiro: make sequencial"; exit 1; }

mkdir -p "$SAIDA"

for N in $TAMANHOS; do
    ARQ="$SAIDA/seq_N${N}_P${PASSES}.txt"
    # O tempo de execução é removido de propósito: ele varia a cada
    # execução e não faz parte do resultado. O gabarito guarda apenas o
    # que as versões paralelas têm de reproduzir.
    "$BIN" "$N" "$PASSES" \
        | sed -E 's/[[:space:]]+tempo=[0-9.]+ s//' \
        > "$ARQ"
    echo "gerado: $ARQ"
    cat "$ARQ"
done

echo
echo "Gabarito completo em $SAIDA/"