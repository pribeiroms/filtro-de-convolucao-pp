#!/usr/bin/env bash
# Medicao da versao MPI conforme o protocolo definido em
# docs/protocolo-medicao.md (Issues #19 e #20).
#
# Uso:  bash scripts/medir_mpi.sh
#
# Por configuracao (N, processos): 6 execucoes, a primeira descartada como
# aquecimento, as 5 seguintes gravadas em dados/medicoes/mpi.csv. Alem das
# colunas do protocolo, cada linha traz o tempo de calculo, de halo e de
# gather, usados na analise de overhead de comunicacao (#19).
#
# Rodar na MESMA maquina e sessao do baseline sequencial: tempos de maquinas
# diferentes nao podem ser misturados num grafico (protocolo, secao 6).
#
# Variaveis opcionais:
#   PROCESSOS="1 2 4 8 16"   graus de paralelismo medidos
#   MPIRUN=mpirun            lancador MPI

set -euo pipefail

PASSES=30
TAMANHOS="500 1000 2000"
PROCESSOS=${PROCESSOS:-"1 2 4 8 16"}
REPETICOES=6          # 1 de aquecimento + 5 validas
DESCARTAR=1

SAIDA_DIR=dados/medicoes
GABARITO_DIR=dados/gabarito
SAIDA=$SAIDA_DIR/mpi.csv

BIN_MPI=bin/convolucao_mpi
MPIRUN=${MPIRUN:-mpirun}

mkdir -p "$SAIDA_DIR"

# ---------------------------------------------------------------
# Extrai um campo da saida do programa.
#   linha 1: "N=2000 passes=30  checksum=508325215.06  tempo=0.077238 s"
#   linha 2: "processos=8  calculo=0.050734 s  halo=0.026767 s  gather=0.009396 s"
# ---------------------------------------------------------------
extrai_checksum() { sed -nE 's/.*checksum=([0-9.]+).*/\1/p' <<< "$1"; }
extrai_tempo()    { sed -nE 's/.*tempo=([0-9.]+) s.*/\1/p'   <<< "$1"; }
extrai_campo()    { sed -nE "s/.*$2=([0-9.]+) s.*/\1/p"      <<< "$1"; }

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
# O OpenMPI recusa mais processos que CPUs sem --oversubscribe
# (protocolo, secao 5). Outros lancadores nao conhecem a opcao.
# ---------------------------------------------------------------
CPUS=$(nproc)
eh_openmpi() { "$MPIRUN" --version 2>&1 | grep -q "Open MPI"; }

opcoes_mpirun() {
    local np=$1
    if [ "$np" -gt "$CPUS" ] && eh_openmpi; then
        echo "--oversubscribe"
    fi
}

# ---------------------------------------------------------------
# Mede um tamanho com um numero de processos e grava as execucoes validas.
# ---------------------------------------------------------------
falhas=0

mede() {
    local N=$1 np=$2
    local esperado
    esperado=$(checksum_esperado "$N")

    local r valida=0
    for r in $(seq 1 "$REPETICOES"); do
        local saida tempo checksum calculo halo gather
        # shellcheck disable=SC2046
        saida=$("$MPIRUN" $(opcoes_mpirun "$np") -np "$np" "$BIN_MPI" "$N" "$PASSES")
        tempo=$(extrai_tempo "$saida")
        checksum=$(extrai_checksum "$saida")
        calculo=$(extrai_campo "$saida" calculo)
        halo=$(extrai_campo "$saida" halo)
        gather=$(extrai_campo "$saida" gather)

        if [ "$r" -le "$DESCARTAR" ]; then
            printf "  [aquecimento] %s s\n" "$tempo"
            continue
        fi

        valida=$((valida + 1))
        printf "  [%d/%d] %s s  checksum=%s  (calculo=%s halo=%s gather=%s)\n" \
            "$valida" "$((REPETICOES - DESCARTAR))" "$tempo" "$checksum" \
            "$calculo" "$halo" "$gather"

        # O MPI soma o checksum na mesma ordem do sequencial, entao o valor
        # tem de ser identico ao gabarito. Qualquer diferenca e erro (#20).
        if [ "$esperado" != "SEM_GABARITO" ] && [ "$checksum" != "$esperado" ]; then
            echo "  ERRO: checksum diverge do gabarito ($esperado)" >&2
            falhas=$((falhas + 1))
        fi

        echo "mpi,$N,$PASSES,$np,$valida,$tempo,$checksum,$calculo,$halo,$gather" \
            >> "$SAIDA"
    done
}

# ---------------------------------------------------------------
# MPI
# ---------------------------------------------------------------
[ -x "$BIN_MPI" ] || [ -x "$BIN_MPI.exe" ] || { echo "Compile primeiro: make mpi"; exit 1; }

echo "versao,N,passes,unidades,repeticao,tempo_s,checksum,calculo_s,halo_s,gather_s" \
    > "$SAIDA"

echo "=============================================="
echo " Medicao da versao MPI"
echo "=============================================="
echo "Data:       $(date -u '+%Y-%m-%d %H:%M:%S UTC')"
echo "Compilador: $(mpic++ --version 2>/dev/null | head -1)"
echo "Lancador:   $("$MPIRUN" --version 2>&1 | head -1)"
echo "CPUs:       $CPUS"
echo "Passes:     $PASSES"
echo "Processos:  $PROCESSOS"
echo "Protocolo:  $REPETICOES execucoes, $DESCARTAR descartada(s)"
echo

for N in $TAMANHOS; do
    for np in $PROCESSOS; do
        echo "--- N = $N, processos = $np $(opcoes_mpirun "$np") ---"
        mede "$N" "$np"
        echo
    done
done

echo "CSV gravado em $SAIDA"

if [ "$falhas" -gt 0 ]; then
    echo "FALHA: $falhas execucao(oes) com checksum diferente do gabarito." >&2
    exit 1
fi
