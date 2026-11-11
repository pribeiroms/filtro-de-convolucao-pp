#!/usr/bin/env python3
"""Resume as medicoes brutas em medianas, conforme o protocolo (Issue #7).

Gera dois arquivos derivados em dados/medicoes/:

    resumo.csv        escalabilidade forte (sequencial, openmp, mpi)
    resumo_fraca.csv  escalabilidade fraca

Os dois sao separados de proposito. Na escalabilidade fraca o tamanho da
imagem cresce junto com o numero de unidades, e algumas configuracoes
coincidem com as da escalabilidade forte -- por exemplo "openmp, N=2000, 16
unidades" existe nas duas. Somar as duas medicoes num grupo so misturaria
experimentos diferentes.

O baseline vem apenas de sequencial.csv. O medir_openmp.sh tambem grava
linhas "sequencial" dentro de openmp.csv; elas sao ignoradas aqui para que
exista um unico baseline no repositorio, que e o denominador de todas as
curvas de speedup.

O openmp_schedules.csv nao entra: ele nao tem coluna "versao" e tem duas
dimensoes a mais (schedule e chunk). Seu resumo e gerado pelo
scripts/graficos_openmp.py, em docs/openmp_resultados.md.

Uso:  python3 scripts/resumir.py
"""

import sys
from pathlib import Path

import pandas as pd

DIR = Path("dados/medicoes")

FORTE = ["sequencial.csv", "openmp.csv", "mpi.csv"]
FRACA = ["escalabilidade_fraca.csv"]

CHAVE = ["versao", "N", "passes", "unidades"]


def resume(arquivos: list[str], saida: Path, ignorar_sequencial_duplicado: bool) -> bool:
    """Le os arquivos, agrupa por configuracao e grava as medianas."""
    partes = []
    for nome in arquivos:
        caminho = DIR / nome
        if not caminho.exists():
            print(f"  {nome}: ausente, ignorado", file=sys.stderr)
            continue
        df = pd.read_csv(caminho)
        if ignorar_sequencial_duplicado and nome != "sequencial.csv":
            # So sequencial.csv define o baseline.
            df = df[df["versao"] != "sequencial"]
        partes.append(df)

    if not partes:
        return False

    bruto = pd.concat(partes, ignore_index=True)

    linhas = []
    for (versao, N, passes, unidades), g in bruto.groupby(CHAVE, sort=True):
        tempos = g["tempo_s"]
        mediana, minimo, maximo = tempos.median(), tempos.min(), tempos.max()

        checksums = g["checksum"].unique()
        if len(checksums) > 1:
            print(
                f"AVISO: {versao} N={N} unidades={unidades} produziu "
                f"checksums diferentes: {list(checksums)}",
                file=sys.stderr,
            )

        linhas.append(
            {
                "versao": versao,
                "N": N,
                "passes": passes,
                "unidades": unidades,
                "execucoes": len(g),
                "mediana_s": round(mediana, 6),
                "min_s": round(minimo, 6),
                "max_s": round(maximo, 6),
                # dispersao = (max - min) / mediana, em %
                "dispersao_pct": round(100 * (maximo - minimo) / mediana, 2),
                "checksum": checksums[0],
            }
        )

    resumo = pd.DataFrame(linhas)
    resumo.to_csv(saida, index=False)
    print(resumo.to_string(index=False))
    print(f"\ngravado em {saida}\n")
    return True


def main() -> int:
    if not DIR.is_dir():
        print(f"diretorio nao encontrado: {DIR}", file=sys.stderr)
        return 1

    print("=== Escalabilidade forte ===")
    ok = resume(FORTE, DIR / "resumo.csv", ignorar_sequencial_duplicado=True)

    print("=== Escalabilidade fraca ===")
    resume(FRACA, DIR / "resumo_fraca.csv", ignorar_sequencial_duplicado=False)

    if (DIR / "openmp_schedules.csv").exists():
        print("openmp_schedules.csv: resumido em docs/openmp_resultados.md "
              "pelo scripts/graficos_openmp.py")

    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())