/* OpenMP 03 — Condicao de corrida e as tres formas de corrigi-la
 * Mostra o resultado ERRADO (sem protecao) e tres versoes corretas,
 * com o custo de cada uma. O valor correto e sempre n*(n-1)/2.
 * Compilar: gcc -O2 -fopenmp -o 03_condicao_corrida 03_condicao_corrida.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv) {
    long n = (argc > 1) ? atol(argv[1]) : 20000000L;
    long esperado = n * (n - 1) / 2;
    double t0;

    long s1 = 0;
    t0 = omp_get_wtime();
    #pragma omp parallel for
    for (long i = 0; i < n; i++) s1 += i;          /* ERRADO de proposito */
    printf("sem protecao : %ld  (esperado %ld)  t=%.4f s\n", s1, esperado, omp_get_wtime() - t0);

    long s2 = 0;
    t0 = omp_get_wtime();
    #pragma omp parallel for
    for (long i = 0; i < n; i++) {
        #pragma omp critical
        s2 += i;
    }
    printf("critical     : %ld  t=%.4f s\n", s2, omp_get_wtime() - t0);

    long s3 = 0;
    t0 = omp_get_wtime();
    #pragma omp parallel for
    for (long i = 0; i < n; i++) {
        #pragma omp atomic
        s3 += i;
    }
    printf("atomic       : %ld  t=%.4f s\n", s3, omp_get_wtime() - t0);

    long s4 = 0;
    t0 = omp_get_wtime();
    #pragma omp parallel for reduction(+:s4)
    for (long i = 0; i < n; i++) s4 += i;
    printf("reduction    : %ld  t=%.4f s\n", s4, omp_get_wtime() - t0);

    puts("\nDiscussao: as quatro versoes fazem o mesmo trabalho aritmetico.");
    puts("A diferenca de tempo e inteiramente custo de sincronizacao.");
    return 0;
}
