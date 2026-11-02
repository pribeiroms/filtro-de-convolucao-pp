/* MPI 03 — Operacoes coletivas: Bcast e Reduce
 * Calcula pi distribuindo o intervalo entre os processos.
 * Compilar: mpicc -O2 -o 03_coletivas_pi 03_coletivas_pi.c -lm
 * Executar: mpirun -np 4 ./03_coletivas_pi 100000000
 */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char **argv) {
    int rank, size;
    long n = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) n = (argc > 1) ? atol(argv[1]) : 100000000L;
    MPI_Bcast(&n, 1, MPI_LONG, 0, MPI_COMM_WORLD);

    double t0 = MPI_Wtime();
    double passo = 1.0 / (double)n, parcial = 0.0;

    /* decomposicao ciclica: cada rank pega 1 em cada 'size' iteracoes */
    for (long i = rank; i < n; i += size) {
        double x = (i + 0.5) * passo;
        parcial += 4.0 / (1.0 + x * x);
    }
    parcial *= passo;

    double total = 0.0;
    MPI_Reduce(&parcial, &total, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    double t1 = MPI_Wtime();

    if (rank == 0)
        printf("np=%d  n=%ld  pi=%.12f  tempo=%.6f s\n", size, n, total, t1 - t0);

    MPI_Finalize();
    return 0;
}
