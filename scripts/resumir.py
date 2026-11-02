#!/usr/bin/env python3
"""Resume as medicoes brutas em medianas, conforme o protocolo (Issue #7).

Le todos os CSV de dados/medicoes/ (exceto o proprio resumo), agrupa por
versao, tamanho e numero de unidades, e grava dados/medicoes/resumo.csv.

As medicoes brutas continuam sendo a evidencia; este arquivo e derivado e
pode ser regerado a qualquer momento sem repetir as medicoes.

Uso:  python3 scripts/resumir.py
"""

import sys
from pathlib import Path

import pandas as pd

DIR = Path("dados/medicoes")
RESUMO = DIR / "resumo.csv"


def main() -> int:
    if not DIR.is_dir():
        print(f"diretorio nao encontrado: {DIR}", file=sys.stderr)
        return 1

    arquivos = sorted(p for p in DIR.glob("*.csv") if p != RESUMO)
    if not arquivos:
        print(f"nenhuma medicao em {DIR}", file=sys.stderr)
        return 1

    bruto = pd.concat([pd.read_csv(p) for p in arquivos], ignore_index=True)

    linhas = []
    for (versao, N, passes, unidades), g in bruto.groupby(
        ["versao", "N", "passes", "unidades"], sort=True
    ):
        tempos = g["tempo_s"]
        mediana = tempos.median()
        minimo = tempos.min()
        maximo = tempos.max()

        checksums = g["checksum"].unique()
        if len(checksums) > 1:
            print(
                f"AVISO: {versao} N={N} unidades={unidades} produziu "
                f"checksums diferentes: {list(checksums)}",
                file=sys.stderr,
            )
        checksum = checksums[0]

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
                "checksum": checksum,
            }
        )

    resumo = pd.DataFrame(linhas)
    resumo.to_csv(RESUMO, index=False)

    print(resumo.to_string(index=False))
    print(f"\nresumo gravado em {RESUMO}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())