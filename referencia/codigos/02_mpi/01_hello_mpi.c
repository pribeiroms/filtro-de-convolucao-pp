/* MPI 01 — Modelo SPMD: rank, size e comunicador
 * Compilar: mpicc -O2 -o 01_hello_mpi 01_hello_mpi.c
 * Executar: mpirun -np 4 ./01_hello_mpi
 */
#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv) {
    int rank, size, len;
    char host[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Get_processor_name(host, &len);

    printf("Processo %d de %d executando em %s\n", rank, size, host);

    MPI_Finalize();
    return 0;
}
