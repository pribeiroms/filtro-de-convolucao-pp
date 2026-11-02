/* MPI 04 — Scatter, computacao local e Gather
 * O rank 0 gera um vetor, distribui pedacos iguais, cada processo eleva
 * seus elementos ao quadrado, e o rank 0 recolhe o resultado.
 * Compilar: mpicc -O2 -o 04_scatter_gather 04_scatter_gather.c
 * Executar: mpirun -np 4 ./04_scatter_gather
 */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N_POR_PROC 4

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = N_POR_PROC * size;
    int *global = NULL;
    int local[N_POR_PROC];

    if (rank == 0) {
        global = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) global[i] = i + 1;
    }

    MPI_Scatter(global, N_POR_PROC, MPI_INT, local, N_POR_PROC, MPI_INT, 0, MPI_COMM_WORLD);

    for (int i = 0; i < N_POR_PROC; i++) local[i] = local[i] * local[i];

    MPI_Gather(local, N_POR_PROC, MPI_INT, global, N_POR_PROC, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Resultado (quadrados de 1..%d):\n  ", n);
        for (int i = 0; i < n; i++) printf("%d ", global[i]);
        printf("\n");
        free(global);
    }

    MPI_Finalize();
    return 0;
}
