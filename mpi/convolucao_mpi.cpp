/* Versao MPI — Filtro de convolucao 2D (blur 3x3) sobre imagem sintetica
 * IESB 2026/2 — CCO085 — Programacao Paralela
 *
 * Compilar: make mpi   (ou: mpic++ -O2 -Wall -o bin/convolucao_mpi mpi/convolucao_mpi.cpp)
 * Executar: mpirun -np 4 ./bin/convolucao_mpi 2000 30   (lado da imagem; num. de passes)
 *
 * Ideia geral (memoria distribuida, decomposicao de dominio):
 *   - A imagem N x N e dividida em FAIXAS DE LINHAS, uma faixa por processo.
 *   - Cada processo guarda so a sua faixa, mais 1 linha extra em cima e 1 embaixo
 *     (o HALO), porque o filtro 3x3 de uma linha precisa da linha de cima e da
 *     de baixo, que pertencem ao processo vizinho.
 *   - A cada passe, os vizinhos trocam essas linhas de borda (MPI_Sendrecv) e
 *     depois cada processo calcula a sua faixa sozinho.
 *   - No final, o processo 0 junta a imagem inteira (MPI_Gatherv) e calcula o
 *     checksum na mesma ordem do sequencial, para dar exatamente o mesmo valor.
 *
 * Por que faixas de linhas (1D) e nao blocos 2D: na memoria a imagem e guardada
 * linha por linha, entao uma linha inteira e um bloco continuo e pode ser enviada
 * numa unica mensagem, sem montar tipos derivados. Com poucos processos (ate 8
 * ou 16), o halo de 2 linhas por processo e pequeno perto do calculo da faixa.
 */
#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace std;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);    // quem sou eu (0 .. nprocs-1)
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);  // quantos processos existem

    /* Parametros de entrada. O processo 0 le da linha de comando e manda para
     * todos com MPI_Bcast (operacao coletiva: um envia, todos recebem). O padrao
     * MPI so garante os argumentos no processo 0, por isso nao confiamos que os
     * outros tenham o argv. Os valores padrao sao os mesmos do sequencial. */
    int param[2];
    if (rank == 0) {
        param[0] = (argc > 1) ? atoi(argv[1]) : 2000;  // N: lado da imagem
        param[1] = (argc > 2) ? atoi(argv[2]) : 30;    // P: numero de passes
    }
    MPI_Bcast(param, 2, MPI_INT, 0, MPI_COMM_WORLD);
    int N = param[0];
    int P = param[1];

    if (N < nprocs) {
        if (rank == 0)
            fprintf(stderr, "Erro: N (%d) precisa ser >= numero de processos (%d).\n", N, nprocs);
        MPI_Finalize();
        return 1;
    }

    /* ----- Decomposicao de dominio (#16) -----
     * N linhas divididas entre nprocs processos. Se a divisao nao for exata,
     * os 'resto' primeiros processos ficam com 1 linha a mais, para a carga
     * ficar o mais igual possivel (diferenca de no maximo 1 linha). */
    int base   = N / nprocs;
    int resto  = N % nprocs;
    int linhas = base + (rank < resto ? 1 : 0);                 // linhas desta faixa
    int inicio = rank * base + (rank < resto ? rank : resto);   // 1a linha global da faixa

    /* Vetores locais com (linhas + 2) linhas de N colunas:
     *   linha local 0           -> halo de cima   (copia da ultima linha do vizinho de cima)
     *   linhas locais 1..linhas -> a faixa deste processo
     *   linha local linhas+1    -> halo de baixo  (copia da 1a linha do vizinho de baixo)
     *
     * 'b' comeca zerado, IGUAL ao sequencial. Isso importa: as bordas da imagem
     * (linhas 0 e N-1, colunas 0 e N-1) nunca sao recalculadas, e com o swap
     * elas alternam entre os valores originais e zero a cada passe. Para o
     * checksum bater, cada processo precisa reproduzir exatamente esse efeito. */
    vector<float> a((size_t)(linhas + 2) * N), b((size_t)(linhas + 2) * N);

    /* Cada processo gera so as suas linhas, com a mesma formula do sequencial,
     * usando o indice GLOBAL do pixel. Assim nao precisamos distribuir a imagem
     * com Scatterv: o resultado e identico e evitamos uma comunicacao inteira. */
    for (int l = 0; l < linhas; l++)
        for (int x = 0; x < N; x++) {
            int i = (inicio + l) * N + x;                  // indice global do pixel
            a[(size_t)(l + 1) * N + x] = (float)((i * 131) % 256);
        }

    /* Vizinhos na "fila" de faixas. MPI_PROC_NULL e um destino vazio: enviar ou
     * receber dele nao faz nada. Assim o primeiro e o ultimo processo usam o
     * mesmo codigo dos outros, sem if especial para quem nao tem vizinho. */
    int cima  = (rank > 0)          ? rank - 1 : MPI_PROC_NULL;
    int baixo = (rank < nprocs - 1) ? rank + 1 : MPI_PROC_NULL;

    double t_comunicacao = 0.0;

    // [MEDICAO] Barreira para todos comecarem juntos; mede so os passes do filtro,
    // o mesmo trecho que o sequencial cronometra (sem inicializacao nem checksum).
    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    for (int p = 0; p < P; p++) {
        /* ----- Troca de halo (#17) -----
         * Feita a CADA passe, porque o passe anterior mudou as linhas de borda
         * dos vizinhos. MPI_Sendrecv envia e recebe na mesma chamada: se cada
         * processo fizesse primeiro um Send bloqueante e depois um Recv, todos
         * poderiam ficar esperando o outro receber (deadlock).
         *
         * 1) mando minha 1a linha para cima e recebo, de baixo, o meu halo de baixo;
         * 2) mando minha ultima linha para baixo e recebo, de cima, o meu halo de cima. */
        // [MEDICAO] tempo gasto em comunicacao, para a analise de overhead (#19)
        double tc = MPI_Wtime();
        MPI_Sendrecv(&a[(size_t)1 * N],            N, MPI_FLOAT, cima,  0,
                     &a[(size_t)(linhas + 1) * N], N, MPI_FLOAT, baixo, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&a[(size_t)linhas * N],       N, MPI_FLOAT, baixo, 1,
                     &a[0],                        N, MPI_FLOAT, cima,  1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        t_comunicacao += MPI_Wtime() - tc;

        /* ----- Calculo local -----
         * Mesmo laco do sequencial, so que sobre as linhas desta faixa. As linhas
         * 0 e N-1 da imagem inteira nao sao calculadas (no sequencial o laco vai
         * de 1 a N-2), entao pulamos quando a linha global for uma delas.
         * A soma percorre dy e dx na mesma ordem do sequencial, para cada pixel
         * dar exatamente o mesmo float (soma de float depende da ordem). */
        for (int l = 1; l <= linhas; l++) {
            int y = inicio + l - 1;                  // linha global
            if (y == 0 || y == N - 1) continue;
            for (int x = 1; x < N - 1; x++) {
                float s = 0;
                for (int dy = -1; dy <= 1; dy++)
                    for (int dx = -1; dx <= 1; dx++)
                        s += a[(size_t)(l + dy) * N + (x + dx)];
                b[(size_t)l * N + x] = s / 9.0f;
            }
        }
        a.swap(b);  // o resultado deste passe vira a entrada do proximo
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double t1 = MPI_Wtime();

    /* ----- Reuniao do resultado (#18) -----
     * MPI_Gatherv (coletiva) junta as faixas no processo 0. E a versao "v"
     * porque as faixas podem ter tamanhos diferentes quando N nao e multiplo de
     * nprocs; cada processo informa quantos floats envia (contagens) e onde eles
     * entram na imagem final (deslocamentos). So o processo 0 precisa desses vetores. */
    // [MEDICAO] o gather fica fora do tempo principal, como o checksum no sequencial
    double tg = MPI_Wtime();
    vector<float> imagem;
    vector<int> contagens, deslocamentos;
    if (rank == 0) {
        imagem.resize((size_t)N * N);
        contagens.resize(nprocs);
        deslocamentos.resize(nprocs);
        for (int r = 0; r < nprocs; r++) {
            int lr = base + (r < resto ? 1 : 0);
            int ir = r * base + (r < resto ? r : resto);
            contagens[r]     = lr * N;
            deslocamentos[r] = ir * N;
        }
    }
    MPI_Gatherv(&a[(size_t)1 * N], linhas * N, MPI_FLOAT,
                rank == 0 ? imagem.data() : nullptr,
                rank == 0 ? contagens.data() : nullptr,
                rank == 0 ? deslocamentos.data() : nullptr,
                MPI_FLOAT, 0, MPI_COMM_WORLD);
    double t_gather = MPI_Wtime() - tg;

    /* Maior tempo de comunicacao entre os processos (coletiva MPI_Reduce com
     * MPI_MAX): o processo mais lento e o que define o tempo total. */
    double t_com_max = 0.0;
    MPI_Reduce(&t_comunicacao, &t_com_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    /* Checksum no processo 0, somando a imagem inteira na MESMA ORDEM do
     * sequencial. Nao usamos MPI_Reduce de somas parciais aqui de proposito:
     * somar double em outra ordem muda as ultimas casas decimais, e o
     * checksum deixaria de bater com o gabarito. */
    if (rank == 0) {
        double soma = 0;
        for (size_t i = 0; i < (size_t)N * N; i++) soma += imagem[i];
        printf("N=%d passes=%d  checksum=%.2f  tempo=%.6f s\n", N, P, soma, t1 - t0);
        printf("processos=%d  comunicacao_halo=%.6f s  gather=%.6f s\n",
               nprocs, t_com_max, t_gather);
    }

    MPI_Finalize();
    return 0;
}
