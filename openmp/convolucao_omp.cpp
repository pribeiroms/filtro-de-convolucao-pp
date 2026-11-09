// Versao OpenMP do baseline sequencial. O kernel e as bordas sao identicos.
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <cstring>
#include <omp.h>
using namespace std;

static double agora() {
    timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

int main(int argc, char** argv) {
    int N = (argc > 1) ? atoi(argv[1]) : 2000;
    int P = (argc > 2) ? atoi(argv[2]) : 30;
    int threads = (argc > 3) ? atoi(argv[3]) : omp_get_max_threads();
    const char* schedule = (argc > 4) ? argv[4] : "static";
    int chunk = (argc > 5) ? atoi(argv[5]) : 1;
    if (N < 3 || P < 0 || threads < 1 || chunk < 1) {
        fprintf(stderr, "Uso: %s [N>=3] [passes>=0] [threads] [static|dynamic|guided] [chunk]\n", argv[0]);
        return 1;
    }
    omp_sched_t sched;
    if (!strcmp(schedule, "static")) sched = omp_sched_static;
    else if (!strcmp(schedule, "dynamic")) sched = omp_sched_dynamic;
    else if (!strcmp(schedule, "guided")) sched = omp_sched_guided;
    else { fprintf(stderr, "Schedule invalido: %s\n", schedule); return 1; }
    omp_set_num_threads(threads);
    omp_set_schedule(sched, chunk);
    vector<float> a(N * N), b(N * N);
    for (int i = 0; i < N * N; i++) a[i] = (float)((i * 131) % 256);

    double t0 = agora();
    for (int p = 0; p < P; p++) {
        // a e b sao buffers distintos. Cada iteracao escreve um unico pixel
        // interno de b; os pixels de borda permanecem como no baseline.
        #pragma omp parallel for collapse(2) schedule(runtime) default(none) shared(a, b, N)
        for (int y = 1; y < N - 1; y++)
            for (int x = 1; x < N - 1; x++) {
                float s = 0;
                for (int dy = -1; dy <= 1; dy++)
                    for (int dx = -1; dx <= 1; dx++)
                        s += a[(y + dy) * N + (x + dx)];
                b[y * N + x] = s / 9.0f;
            }
        a.swap(b);
    }
    double t1 = agora();
    double soma = 0;
    for (int i = 0; i < N * N; i++) soma += a[i];
    printf("N=%d passes=%d  checksum=%.2f  tempo=%.6f s\n", N, P, soma, t1 - t0);
    return 0;
}
