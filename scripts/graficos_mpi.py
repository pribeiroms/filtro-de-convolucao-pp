#!/usr/bin/env python3
"""Graficos e tabelas da versao MPI (Issues #19 e #20).

Le dados/medicoes/mpi.csv (gerado por scripts/medir_mpi.sh) e
dados/medicoes/sequencial.csv (gerado por scripts/medir.sh), calcula as
medianas conforme o protocolo (Issue #7) e grava em graficos/:

  mpi_speedup.png       speedup por numero de processos, um traco por N
  mpi_eficiencia.png    eficiencia por numero de processos
  mpi_comunicacao.png   fracao do tempo em calculo, halo e gather (#19)

Tambem imprime as tabelas em Markdown, prontas para docs/mpi.md.

Speedup usa o tempo do SEQUENCIAL medido, nao o do MPI com 1 processo
(protocolo, secao 8). Os dois CSV precisam vir da mesma maquina e sessao.

Uso:  python3 scripts/graficos_mpi.py [diretorio_das_medicoes]
"""

import sys
from pathlib import Path

import matplotlib

matplotlib.use("Agg")  # gera arquivos sem precisar de tela
import matplotlib.pyplot as plt
import pandas as pd

DIR = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("dados/medicoes")
SAIDA = Path("graficos")


def medianas(arq: Path, colunas: list[str]) -> pd.DataFrame:
    bruto = pd.read_csv(arq)
    return bruto.groupby(["N", "unidades"], as_index=False)[colunas].median()


def tabela_markdown(df: pd.DataFrame) -> str:
    cab = "| " + " | ".join(df.columns) + " |"
    sep = "|" + "---|" * len(df.columns)
    linhas = ["| " + " | ".join(str(v) for v in r) + " |" for r in df.values]
    return "\n".join([cab, sep, *linhas])


def main() -> int:
    arq_mpi = DIR / "mpi.csv"
    arq_seq = DIR / "sequencial.csv"
    for arq in (arq_mpi, arq_seq):
        if not arq.is_file():
            print(f"arquivo nao encontrado: {arq}", file=sys.stderr)
            return 1

    mpi = medianas(arq_mpi, ["tempo_s", "calculo_s", "halo_s", "gather_s"])
    seq = medianas(arq_seq, ["tempo_s"]).rename(columns={"tempo_s": "t_seq"})

    # O resultado nao pode mudar com o numero de processos (#20)
    bruto = pd.read_csv(arq_mpi)
    for N, g in bruto.groupby("N"):
        if g["checksum"].nunique() > 1:
            print(f"ERRO: N={N} tem checksums diferentes entre configuracoes: "
                  f"{sorted(g['checksum'].unique())}", file=sys.stderr)
            return 1

    df = mpi.merge(seq[["N", "t_seq"]], on="N")
    df["speedup"] = df["t_seq"] / df["tempo_s"]
    df["eficiencia"] = df["speedup"] / df["unidades"]
    comunic = df["halo_s"] + df["gather_s"]
    df["comunicacao_pct"] = 100 * comunic / df["tempo_s"]

    SAIDA.mkdir(exist_ok=True)
    tamanhos = sorted(df["N"].unique())
    procs = sorted(df["unidades"].unique())

    # ---- Speedup ----
    fig, ax = plt.subplots(figsize=(7, 5))
    ax.plot(procs, procs, "k--", lw=1, label="ideal (S = p)")
    for N in tamanhos:
        d = df[df["N"] == N]
        ax.plot(d["unidades"], d["speedup"], "o-", label=f"N = {N}")
    ax.set_xscale("log", base=2)
    ax.set_xticks(procs, [str(p) for p in procs])
    ax.set_xlabel("processos MPI")
    ax.set_ylabel("speedup  (T_sequencial / T_mpi)")
    ax.set_title("MPI — speedup por número de processos (30 passes)")
    ax.grid(alpha=0.3)
    ax.legend()
    fig.tight_layout()
    fig.savefig(SAIDA / "mpi_speedup.png", dpi=150)
    plt.close(fig)

    # ---- Eficiencia ----
    fig, ax = plt.subplots(figsize=(7, 5))
    ax.axhline(1.0, color="k", ls="--", lw=1, label="ideal (E = 1)")
    for N in tamanhos:
        d = df[df["N"] == N]
        ax.plot(d["unidades"], d["eficiencia"], "o-", label=f"N = {N}")
    ax.set_xscale("log", base=2)
    ax.set_xticks(procs, [str(p) for p in procs])
    ax.set_ylim(bottom=0)
    ax.set_xlabel("processos MPI")
    ax.set_ylabel("eficiência  (speedup / processos)")
    ax.set_title("MPI — eficiência por número de processos (30 passes)")
    ax.grid(alpha=0.3)
    ax.legend()
    fig.tight_layout()
    fig.savefig(SAIDA / "mpi_eficiencia.png", dpi=150)
    plt.close(fig)

    # ---- Comunicacao vs calculo (#19) ----
    # Barras empilhadas em fracao do tempo total de cada configuracao. O que
    # sobra alem de calculo + halo + gather (laco, swap) fica em "outros".
    fig, axes = plt.subplots(1, len(tamanhos), figsize=(4 * len(tamanhos), 4.5),
                             sharey=True)
    if len(tamanhos) == 1:
        axes = [axes]
    for ax, N in zip(axes, tamanhos):
        d = df[df["N"] == N]
        rot = [str(p) for p in d["unidades"]]
        calc = 100 * d["calculo_s"] / d["tempo_s"]
        halo = 100 * d["halo_s"] / d["tempo_s"]
        gath = 100 * d["gather_s"] / d["tempo_s"]
        outros = (100 - calc - halo - gath).clip(lower=0)
        ax.bar(rot, calc, label="cálculo")
        ax.bar(rot, halo, bottom=calc, label="troca de halo")
        ax.bar(rot, gath, bottom=calc + halo, label="gather")
        ax.bar(rot, outros, bottom=calc + halo + gath, label="outros", color="0.8")
        ax.set_title(f"N = {N}")
        ax.set_xlabel("processos MPI")
    axes[0].set_ylabel("% do tempo total")
    axes[-1].legend(loc="upper left", fontsize=8)
    fig.suptitle("MPI — cálculo vs. comunicação (processo mais lento)")
    fig.tight_layout()
    fig.savefig(SAIDA / "mpi_comunicacao.png", dpi=150)
    plt.close(fig)

    # ---- Tabelas ----
    tab = df[["N", "unidades", "tempo_s", "speedup", "eficiencia",
              "calculo_s", "halo_s", "gather_s", "comunicacao_pct"]].copy()
    tab = tab.rename(columns={"unidades": "processos", "tempo_s": "mediana_s"})
    tab = tab.round({"mediana_s": 6, "speedup": 2, "eficiencia": 2,
                     "calculo_s": 6, "halo_s": 6, "gather_s": 6,
                     "comunicacao_pct": 1})
    print(tabela_markdown(tab))
    print(f"\ngraficos gravados em {SAIDA}/mpi_*.png")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
