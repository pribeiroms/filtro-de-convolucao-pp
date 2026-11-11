#!/usr/bin/env python3
"""Gera SVG de speedup a partir dos CSVs brutos, sem dependencias externas."""
import csv
from collections import defaultdict
from pathlib import Path

src = Path('dados/medicoes/openmp.csv')
if not src.exists():
    raise SystemExit('Execute scripts/medir_openmp.sh para criar os dados.')
rows = defaultdict(list)
with src.open(newline='') as f:
    for row in csv.DictReader(f):
        rows[(row['versao'], int(row['N']), int(row['unidades']))].append(float(row['tempo_s']))
# O baseline vem apenas de sequencial.csv. O medir_openmp.sh tambem grava
# linhas "sequencial" dentro de openmp.csv; usar as duas fontes daria dois
# baselines diferentes no mesmo repositorio, e os graficos discordariam dos
# documentos. Descarta as de openmp.csv e le as de sequencial.csv.
rows = defaultdict(list, {k: v for k, v in rows.items() if k[0] != 'sequencial'})
base = Path('dados/medicoes/sequencial.csv')
if not base.exists():
    raise SystemExit('Execute scripts/medir.sh para criar o baseline.')
with base.open(newline='') as f:
    for row in csv.DictReader(f):
        rows[('sequencial', int(row['N']), 1)].append(float(row['tempo_s']))
def median(v):
    v = sorted(v); n = len(v)
    return v[n//2] if n % 2 else (v[n//2-1]+v[n//2])/2
ns = sorted({k[1] for k in rows})
threads = sorted({k[2] for k in rows if k[0] == 'openmp'})
W,H=820,520; L,R,T,B=80,30,50,65
colors=['#1769aa','#d1495b','#2a9d8f','#e9c46a','#6a4c93']  # uma por contagem de threads
maxv=max((median(rows[('sequencial',n,1)])/median(rows[('openmp',n,t)]) for n in ns for t in threads if ('openmp',n,t) in rows), default=1)
maxv=max(1,maxv*1.1); plotw=W-L-R; ploth=H-T-B
svg=[f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">','<rect width="100%" height="100%" fill="white"/>','<text x="410" y="28" text-anchor="middle" font-family="sans-serif" font-size="18">Speedup OpenMP por threads</text>']
for j in range(6):
    y=T+ploth*j/5; val=maxv*(5-j)/5
    svg += [f'<line x1="{L}" y1="{y:.1f}" x2="{W-R}" y2="{y:.1f}" stroke="#ddd"/>',f'<text x="{L-10}" y="{y+4:.1f}" text-anchor="end" font-family="sans-serif" font-size="12">{val:.1f}</text>']
svg += [f'<text x="20" y="{T+ploth/2}" transform="rotate(-90 20 {T+ploth/2})" text-anchor="middle" font-family="sans-serif" font-size="14">Speedup</text>',f'<line x1="{L}" y1="{T}" x2="{L}" y2="{H-B}" stroke="#333"/>',f'<line x1="{L}" y1="{H-B}" x2="{W-R}" y2="{H-B}" stroke="#333"/>']
for ni,n in enumerate(ns):
    x=L+(ni+0.5)*plotw/max(1,len(ns)); svg.append(f'<text x="{x}" y="{H-B+25}" text-anchor="middle" font-family="sans-serif" font-size="13">N={n}</text>')
    for ti,t in enumerate(threads):
        key=('openmp',n,t)
        if key not in rows or ('sequencial',n,1) not in rows: continue
        val=median(rows[('sequencial',n,1)])/median(rows[key]); y=T+ploth*(1-val/maxv); px=x+(ti-(len(threads)-1)/2)*12
        svg.append(f'<circle cx="{px:.1f}" cy="{y:.1f}" r="4" fill="{colors[ti%len(colors)]}"/>')
for i,t in enumerate(threads):
    x=L+15+i*115; svg += [f'<circle cx="{x}" cy="{H-20}" r="4" fill="{colors[i%len(colors)]}"/>',f'<text x="{x+9}" y="{H-16}" font-family="sans-serif" font-size="12">{t} threads</text>']
Path('graficos/openmp_speedup.svg').write_text('\n'.join(svg+['</svg>']), encoding='utf-8')

# Relatorio de schedules com medianas calculadas das cinco repeticoes brutas.
schedule_file = Path('dados/medicoes/openmp_schedules.csv')
if schedule_file.exists():
    sched_rows = defaultdict(list)
    with schedule_file.open(newline='') as f:
        for row in csv.DictReader(f):
            key = (int(row['N']), row['schedule'], int(row['chunk']))
            sched_rows[key].append(float(row['tempo_s']))
    lines = ['# Comparacao de schedules OpenMP', '',
             'Medianas dos tempos em segundos; quatro threads, cinco repeticoes apos aquecimento.', '',
             '| N | Schedule | Chunk | Mediana (s) |', '|---:|---|---:|---:|']
    for (n, sch, chunk), values in sorted(sched_rows.items()):
        lines.append(f'| {n} | {sch} | {chunk} | {median(values):.6f} |')
    lines += ['', 'Interprete os resultados junto de [docs/openmp.md](openmp.md) e da secao 5 de',
              '[docs/analise-experimental.md](analise-experimental.md). O custo por pixel e uniforme,',
              'entao nao ha desbalanceamento para as politicas dinamicas corrigirem. O que separa as',
              'politicas neste problema e o tamanho do pedaco: com chunk pequeno o static perde a',
              'localidade dos blocos contiguos, que e sua principal vantagem, e o guided -- que comeca',
              'com pedacos grandes e contiguos -- leva vantagem nos tres tamanhos medidos.']
    Path('docs/openmp_resultados.md').write_text('\n'.join(lines)+'\n', encoding='utf-8')