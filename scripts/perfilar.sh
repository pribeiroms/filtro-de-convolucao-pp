#!/usr/bin/env bash
# Perfilamento do codigo sequencial: mede o tempo de cada trecho usando a
# copia instrumentada, para identificar o hotspot (Issue #6).
#
# Uso:  bash scripts/perfilar.sh
#
# Roda 4 vezes para cada tamanho: a primeira e descartada como aquecimento
# e as 3 seguintes sao as medicoes validas. A saida bruta vai para
# docs/perfilamento_bruto.txt, que e a evidencia exigida pela Issue.

set -euo pipefail

BIN=bin/convolucao_instr
SAIDA=docs/perfilamento_bruto.txt
PASSES=30
TAMANHOS="500 1000 2000"
REPETICOES=4

[ -x "$BIN" ] || { echo "Compile primeiro: make instrumentado"; exit 1; }

mkdir -p docs

{
    echo "========================================================"
    echo " Perfilamento do codigo sequencial"
    echo "========================================================"
    echo "Data da medicao: $(date -u '+%Y-%m-%d %H:%M:%S UTC')"
    echo "Compilador:      $(g++ --version | head -1)"
    echo "CPUs visiveis:   $(nproc)"
    echo "Binario:         $BIN"
    echo "Passes:          $PASSES"
    echo "Repeticoes:      $REPETICOES por tamanho (a 1a e aquecimento)"
    echo
    echo "A copia instrumentada mede o tempo de cada trecho. A logica de"
    echo "calculo e a entrada sao identicas as do codigo de referencia."
    echo

    for N in $TAMANHOS; do
        echo "--------------------------------------------------------"
        echo " N = $N"
        echo "--------------------------------------------------------"
        for R in $(seq 1 "$REPETICOES"); do
            if [ "$R" -eq 1 ]; then
                echo "[execucao 1 - aquecimento, descartada]"
            else
                echo "[execucao $((R-1)) de $((REPETICOES-1))]"
            fi
            "$BIN" "$N" "$PASSES"
            echo
        done
    done
} | tee "$SAIDA"

echo
echo "Evidencia bruta salva em $SAIDA"