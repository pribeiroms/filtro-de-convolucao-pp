#!/usr/bin/env bash
set -euo pipefail
PASSES=${PASSES:-30}
TAMANHOS=${TAMANHOS:-"500 1000 2000"}
THREADS=${THREADS:-"1 2 4 8 16"}
if [[ -n "${THREADS_EXTRA:-}" ]]; then THREADS="$THREADS $THREADS_EXTRA"; fi
REPETICOES=6 # uma de aquecimento + cinco registradas
OUT=dados/medicoes
mkdir -p "$OUT" graficos
[[ -x bin/convolucao_seq && -x bin/convolucao_omp ]] || { echo "Execute make sequencial openmp" >&2; exit 1; }
printf 'versao,N,passes,unidades,schedule,chunk,repeticao,tempo_s,checksum\n' > "$OUT/openmp.csv"
printf 'N,passes,unidades,schedule,chunk,repeticao,tempo_s,checksum\n' > "$OUT/openmp_schedules.csv"
checksum() { sed -E 's/.*checksum=([0-9.]+).*/\1/' <<< "$1"; }
tempo() { sed -E 's/.*tempo=([0-9.]+) s.*/\1/' <<< "$1"; }
esperado() { sed -E 's/.*checksum=([0-9.]+).*/\1/' "dados/gabarito/seq_N${1}_P${PASSES}.txt"; }
mede() {
  local kind=$1 n=$2 t=$3 sch=$4 ch=$5 r out c tm exp rep
  exp=$(esperado "$n")
  for r in $(seq 1 $REPETICOES); do
        if [[ "$kind" == openmp || "$kind" == schedule ]]; then
      out=$(OMP_NUM_THREADS=$t bin/convolucao_omp "$n" "$PASSES" "$t" "$sch" "$ch")
    else
      out=$(bin/convolucao_seq "$n" "$PASSES")
    fi
    c=$(checksum "$out"); tm=$(tempo "$out")
    [[ "$c" == "$exp" ]] || { echo "Checksum divergente: $out (esperado $exp)" >&2; exit 1; }
    (( r == 1 )) && continue
    rep=$((r-1))
    if [[ "$kind" == openmp ]]; then
      echo "openmp,$n,$PASSES,$t,$sch,$ch,$rep,$tm,$c" >> "$OUT/openmp.csv"
    elif [[ "$kind" == schedule ]]; then
      echo "$n,$PASSES,$t,$sch,$ch,$rep,$tm,$c" >> "$OUT/openmp_schedules.csv"
    else
      echo "sequencial,$n,$PASSES,1,serial,0,$rep,$tm,$c" >> "$OUT/openmp.csv"
    fi
  done
}
for n in $TAMANHOS; do
  echo "Medição de escalabilidade N=$n"
  for t in $THREADS; do mede openmp "$n" "$t" static 1; done
  mede seq "$n" 1 serial 0
  echo "Medição de schedules N=$n"
  for spec in 'static 1' 'dynamic 1' 'dynamic 64' 'guided 1'; do
    read -r sch ch <<< "$spec"
    mede schedule "$n" 4 "$sch" "$ch"
  done
done
python3 scripts/graficos_openmp.py
echo "CSV: $OUT/openmp.csv e $OUT/openmp_schedules.csv"
