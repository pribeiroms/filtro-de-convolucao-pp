#!/usr/bin/env bash
# Gera todos os dados de medicao do projeto, do zero (Issue #22).
#
# Uso:  bash scripts/medir_tudo.sh
#
# Executa, nesta ordem:
#   1. medir.sh          baseline sequencial
#   2. medir_openmp.sh   speedup por threads e politicas de escalonamento
#   3. medir_mpi.sh      speedup por processos, com calculo, halo e gather
#   4. medir_fraca.sh    escalabilidade fraca
#   5. resumir.py        medianas em resumo.csv e resumo_fraca.csv
#   6. graficos_*.py     graficos em graficos/
#
# A ordem importa: o baseline e o denominador das curvas das duas versoes
# paralelas, e o protocolo (docs/protocolo-medicao.md, secao 6) exige que
# todas as medicoes de uma curva saiam da mesma maquina e sessao.
#
# Antes de rodar: feche os demais aplicativos, pause servicos de
# sincronizacao de arquivos e deixe a maquina sem outras cargas. Leva cerca
# de dez minutos.

set -euo pipefail

echo "=============================================="
echo " Bateria completa de medicao"
echo "=============================================="
echo "Inicio: $(date -u '+%Y-%m-%d %H:%M:%S UTC')"
echo

[ -x bin/convolucao_seq ] && [ -x bin/convolucao_omp ] && [ -x bin/convolucao_mpi ] || {
    echo "Compile primeiro: make all" >&2
    exit 1
}

etapa() {
    echo
    echo "----------------------------------------------"
    echo " $1"
    echo "----------------------------------------------"
}

etapa "1/6  Baseline sequencial"
bash scripts/medir.sh

etapa "2/6  OpenMP"
bash scripts/medir_openmp.sh

etapa "3/6  MPI"
bash scripts/medir_mpi.sh

etapa "4/6  Escalabilidade fraca"
bash scripts/medir_fraca.sh

etapa "5/6  Resumo das medianas"
python3 scripts/resumir.py

etapa "6/6  Graficos"
python3 scripts/graficos_mpi.py
python3 scripts/graficos_openmp.py

echo
echo "=============================================="
echo " Concluido: $(date -u '+%Y-%m-%d %H:%M:%S UTC')"
echo "=============================================="
echo
echo "Dados em dados/medicoes/, graficos em graficos/."
echo
echo "Configuracoes com dispersao acima de 25% entre execucoes:"
awk -F, 'NR>1 && $9+0 > 25 {printf "  %-11s N=%-5s unidades=%-3s dispersao=%s%%\n", $1,$2,$4,$9}' \
    dados/medicoes/resumo.csv
echo
echo "Muitas linhas acima indicam que a maquina estava ocupada durante a"
echo "medicao. Nesse caso, feche o que estiver rodando e repita a bateria."