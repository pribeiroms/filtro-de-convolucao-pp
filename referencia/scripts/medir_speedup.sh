#!/usr/bin/env bash
# Coleta de dados de escalabilidade forte — CCO085 IESB 2026/2
# Gera resultados/openmp_escalabilidade.csv no formato exigido no projeto.
#
# Uso: ./scripts/medir_speedup.sh [N] [REPETICOES]

set -euo pipefail

N=${1:-100000000}
REP=${2:-5}
BIN=codigos/01_openmp/02_pi_openmp
SAIDA=resultados/openmp_escalabilidade.csv

[ -x "$BIN" ] || { echo "Compile primeiro: make"; exit 1; }
mkdir -p resultados

NUCLEOS=$(nproc)
echo "threads,repeticao,tempo_s" > "$SAIDA"

for T in 1 2 4 8 16; do
    [ "$T" -gt $((NUCLEOS * 2)) ] && continue
    for R in $(seq 1 "$REP"); do
        # a primeira execucao de cada configuracao e descartada (aquecimento)
        LINHA=$("$BIN" "$N" "$T" | grep '^paralelo')
        TEMPO=$(echo "$LINHA" | sed -E 's/.*t=([0-9.]+) s.*/\1/')
        if [ "$R" -gt 1 ]; then
            echo "$T,$((R-1)),$TEMPO" >> "$SAIDA"
        fi
    done
    echo "  threads=$T concluido"
done

echo
echo "Dados brutos em $SAIDA"
echo "Nucleos fisicos detectados: $NUCLEOS"
echo
echo "Lembre: o speedup deve ser calculado contra o tempo SEQUENCIAL medido,"
echo "nao contra o tempo da versao paralela rodando com 1 thread."
