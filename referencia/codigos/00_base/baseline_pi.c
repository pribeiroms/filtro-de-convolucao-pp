/* Roteiro 0 — Baseline sequencial e medicao de tempo
 * Programacao Paralela (CCO085) — IESB 2026/2 — Prof. Rodrigo Goncalves Pinto
 *
 * Calcula pi pela integral de 4/(1+x^2) no intervalo [0,1].
 * Objetivo do roteiro: estabelecer o tempo de referencia (baseline) e
 * praticar a metodologia de medicao que sera usada no projeto.
 *
 * Compilar:  gcc -O2 -o baseline_pi baseline_pi.c -lm
 * Executar:  ./baseline_pi 100000000
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double agora(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

int main(int argc, char **argv) {
    long n = (argc > 1) ? atol(argv[1]) : 100000000L;
    double passo = 1.0 / (double)n;
    double soma = 0.0;

    double t0 = agora();
    for (long i = 0; i < n; i++) {
        double x = (i + 0.5) * passo;
        soma += 4.0 / (1.0 + x * x);
    }
    double pi = soma * passo;
    double t1 = agora();

    printf("n=%ld  pi=%.12f  tempo=%.6f s\n", n, pi, t1 - t0);
    return 0;
}
