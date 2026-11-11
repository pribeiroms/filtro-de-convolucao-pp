/* OpenMP 04 — Desbalanceamento de carga e politicas de schedule
 * O conjunto de Mandelbrot tem custo muito desigual por linha, o que torna
 * visivel a diferenca entre static, dynamic e guided.
 * Compilar: gcc -O2 -fopenmp -o 04_schedule_mandelbrot 04_schedule_mandelbrot.c -lm
 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define LARGURA 1200
#define ALTURA  1200
#define MAXIT   2000

static long linha(int y) {
    long conta = 0;
    for (int x = 0; x < LARGURA; x++) {
        double cr = -2.0 + 3.0 * x / LARGURA;
        double ci = -1.5 + 3.0 * y / ALTURA;
        double zr = 0.0, zi = 0.0;
        int it = 0;
        while (zr * zr + zi * zi <= 4.0 && it < MAXIT) {
            double tmp = zr * zr - zi * zi + cr;
            zi = 2.0 * zr * zi + ci;
            zr = tmp;
            it++;
        }
        conta += it;
    }
    return conta;
}

static double mede(const char *nome, int modo) {
    long total = 0;
    double t0 = omp_get_wtime();
    if (modo == 0) {
        #pragma omp parallel for reduction(+:total) schedule(static)
        for (int y = 0; y < ALTURA; y++) total += linha(y);
    } else if (modo == 1) {
        #pragma omp parallel for reduction(+:total) schedule(dynamic, 8)
        for (int y = 0; y < ALTURA; y++) total += linha(y);
    } else {
        #pragma omp parallel for reduction(+:total) schedule(guided)
        for (int y = 0; y < ALTURA; y++) total += linha(y);
    }
    double t = omp_get_wtime() - t0;
    printf("%-16s soma=%ld  t=%.4f s\n", nome, total, t);
    return t;
}

int main(void) {
    printf("Threads: %d\n", omp_get_max_threads());
    mede("static", 0);
    mede("dynamic,8", 1);
    mede("guided", 2);
    return 0;
}
