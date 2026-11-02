/* OpenMP 02 — Paralelizacao de laco com reduction
 * Compara o baseline sequencial com a versao paralela e calcula o speedup.
 * Compilar: gcc -O2 -fopenmp -o 02_pi_openmp 02_pi_openmp.c -lm
 * Executar: ./02_pi_openmp 100000000 4
 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

static double pi_seq(long n) {
    double passo = 1.0 / (double)n, soma = 0.0;
    for (long i = 0; i < n; i++) {
        double x = (i + 0.5) * passo;
        soma += 4.0 / (1.0 + x * x);
    }
    return soma * passo;
}

static double pi_par(long n, int nthreads) {
    double passo = 1.0 / (double)n, soma = 0.0;
    #pragma omp parallel for reduction(+:soma) num_threads(nthreads)
    for (long i = 0; i < n; i++) {
        double x = (i + 0.5) * passo;
        soma += 4.0 / (1.0 + x * x);
    }
    return soma * passo;
}

int main(int argc, char **argv) {
    long n = (argc > 1) ? atol(argv[1]) : 100000000L;
    int  t = (argc > 2) ? atoi(argv[2]) : omp_get_max_threads();

    double t0 = omp_get_wtime();
    double a = pi_seq(n);
    double ts = omp_get_wtime() - t0;

    t0 = omp_get_wtime();
    double b = pi_par(n, t);
    double tp = omp_get_wtime() - t0;

    printf("sequencial : pi=%.12f  t=%.6f s\n", a, ts);
    printf("paralelo   : pi=%.12f  t=%.6f s  (%d threads)\n", b, tp, t);
    printf("speedup    : %.3f\n", ts / tp);
    printf("eficiencia : %.3f\n", (ts / tp) / t);
    return 0;
}
