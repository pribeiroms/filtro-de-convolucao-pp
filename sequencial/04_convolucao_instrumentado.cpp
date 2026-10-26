/* 04_convolucao_instrumentado.cpp — cópia para perfilamento, NAO é o gabarito */
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <ctime>
using namespace std;

static double agora() {
    timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

int main(int argc, char** argv) {
    int N = (argc > 1) ? atoi(argv[1]) : 2000;
    int P = (argc > 2) ? atoi(argv[2]) : 30;
    vector<float> a(N*N), b(N*N);

    double t_init0 = agora();
    for (int i = 0; i < N*N; i++) a[i] = (float)((i * 131) % 256);
    double t_init1 = agora();

    double t_conv0 = agora();
    for (int p = 0; p < P; p++) {
        for (int y = 1; y < N-1; y++)
            for (int x = 1; x < N-1; x++) {
                float s = 0;
                for (int dy = -1; dy <= 1; dy++)
                    for (int dx = -1; dx <= 1; dx++)
                        s += a[(y+dy)*N + (x+dx)];
                b[y*N + x] = s / 9.0f;
            }
        a.swap(b);
    }
    double t_conv1 = agora();

    double t_sum0 = agora();
    double soma = 0;
    for (int i = 0; i < N*N; i++) soma += a[i];
    double t_sum1 = agora();

    double total = t_sum1 - t_init0;
    printf("N=%d passes=%d checksum=%.2f\n", N, P, soma);
    printf("inicializacao: %.6f s (%.2f%%)\n", t_init1-t_init0, 100.0*(t_init1-t_init0)/total);
    printf("convolucao:    %.6f s (%.2f%%)\n", t_conv1-t_conv0, 100.0*(t_conv1-t_conv0)/total);
    printf("checksum:      %.6f s (%.2f%%)\n", t_sum1-t_sum0, 100.0*(t_sum1-t_sum0)/total);
    printf("total:         %.6f s\n", total);
    return 0;
}