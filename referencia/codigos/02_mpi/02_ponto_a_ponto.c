/* MPI 02 — Comunicacao ponto a ponto (anel de mensagens)
 * Cada processo recebe do anterior, soma seu rank e envia ao proximo.
 * Compilar: mpicc -O2 -o 02_ponto_a_ponto 02_ponto_a_ponto.c
 * Executar: mpirun -np 4 ./02_ponto_a_ponto
 */
#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv) {
    int rank, size, valor = 0;
    MPI_Status st;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        valor = 0;
        MPI_Send(&valor, 1, MPI_INT, 1 % size, 0, MPI_COMM_WORLD);
        MPI_Recv(&valor, 1, MPI_INT, size - 1, 0, MPI_COMM_WORLD, &st);
        printf("Anel completo. Valor final no rank 0: %d\n", valor);
    } else {
        MPI_Recv(&valor, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &st);
        valor += rank;
        MPI_Send(&valor, 1, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD);
        printf("Rank %d repassou o valor %d\n", rank, valor);
    }

    MPI_Finalize();
    return 0;
}
