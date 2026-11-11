// Versao OpenMP do baseline sequencial. O kernel e as bordas sao identicos.
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <cstring>
#include <limits>
#include <cstddef>
#include <omp.h>
using namespace std;

static double agora() {
    using relogio = chrono::steady_clock;
    return chrono::duration<double>(relogio::now().time_since_epoch()).count();
}

int main(int argc, char** argv) {
    int N = (argc > 1) ? atoi(argv[1]) : 2000;
    int P = (argc > 2) ? atoi(argv[2]) : 30;
    int threads = (argc > 3) ? atoi(argv[3]) : omp_get_max_threads();
    const char* schedule = (argc > 4) ? argv[4] : "static";
    // chunk 0 (ou negativo) = usar o tamanho padrao do runtime. Para static,
    // esse padrao divide as linhas em blocos contiguos, um por thread, que e
    // o que preserva a localidade de cache. Um chunk pequeno espalha linhas
    // vizinhas entre threads diferentes e derruba o desempenho.
    int chunk = (argc > 5) ? atoi(argv[5]) : 0;
    if (N < 3 || P < 0 || threads < 1 ||
        static_cast<size_t>(N) > numeric_limits<size_t>::max() / static_cast<size_t>(N) ||
        static_cast<size_t>(N) > static_cast<size_t>(numeric_limits<long long>::max())) {
        fprintf(stderr, "Uso: %s [N>=3] [passes>=0] [threads] [static|dynamic|guided] [chunk>=0, 0=padrao]\n", argv[0]);
        return 1;
    }
    const size_t lado = static_cast<size_t>(N);
    const size_t pixels = lado * lado;
    omp_sched_t sched;
    if (!strcmp(schedule, "static")) sched = omp_sched_static;
    else if (!strcmp(schedule, "dynamic")) sched = omp_sched_dynamic;
    else if (!strcmp(schedule, "guided")) sched = omp_sched_guided;
    else { fprintf(stderr, "Schedule invalido: %s\n", schedule); return 1; }
    omp_set_num_threads(threads);
    omp_set_schedule(sched, chunk);
    vector<float> a(pixels), b(pixels);
    for (size_t i = 0; i < pixels; i++) a[i] = static_cast<float>((i * 131) % 256);

    double t0 = agora();
    for (int p = 0; p < P; p++) {
        // a e b sao buffers distintos. Cada iteracao escreve um unico pixel
        // interno de b; os pixels de borda permanecem como no baseline.
        //
        // A paralelizacao e so sobre as linhas (y), sem collapse: cada thread
        // fica com um bloco de linhas inteiras e contiguas na memoria. Com
        // collapse(2) o espaco de iteracao vira pixel a pixel, threads passam
        // a dividir a mesma linha e a escrever na mesma linha de cache
        // (false sharing), o que torna a versao paralela mais lenta que a
        // sequencial.
        //
        // 'lado' entra no shared porque default(none) exige que toda variavel
        // usada na regiao esteja declarada, inclusive as const.
        #pragma omp parallel for schedule(runtime) default(none) shared(a, b, N, lado)
        for (int y = 1; y < N - 1; y++)
            for (int x = 1; x < N - 1; x++) {
                float s = 0;
                for (int dy = -1; dy <= 1; dy++)
                    for (int dx = -1; dx <= 1; dx++)
                        s += a[static_cast<size_t>(y + dy) * lado + static_cast<size_t>(x + dx)];
                b[static_cast<size_t>(y) * lado + static_cast<size_t>(x)] = s / 9.0f;
            }
        a.swap(b);
    }
    double t1 = agora();
    double soma = 0;
    for (size_t i = 0; i < pixels; i++) soma += a[i];
    printf("N=%d passes=%d  checksum=%.2f  tempo=%.6f s\n", N, P, soma, t1 - t0);
    return 0;
}