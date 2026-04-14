#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    double min_nota;
    double max_nota;
    double mediana;
    double media;
    double dsv_pdr;
} Stats;

static size_t students_idx(int C, int A, int N, int i, int j, int k, int l);
static size_t media_idx(int C, int A, int i, int j, int k);
void geraTabela(int R, int C, int A, int N, int *students);
Stats calcula_stats(const double *v, int n);
void format_pt(double valor, char *out, size_t out_size);
void print_linha(const char *label, Stats s);
static int cmp_double(const void *a, const void *b);
void Avaliacao(float valor);

int main(void) {
    int R, C, A, N, T, seed;
    int NumRep = 1000;

    if (scanf("%d %d %d %d %d", &R, &C, &A, &N, &T) != 5 || scanf("%d", &seed) != 1) {
        fprintf(stderr, "Entrada invalida.\n");
        return 1;
    }
    if (R <= 0 || C <= 0 || A <= 0 || N <= 0 || T <= 0) {
        fprintf(stderr, "R, C, A, N e T devem ser positivos.\n");
        return 1;
    }

    size_t total_notas = (size_t)R * C * A * N;
    size_t total_alunos = (size_t)R * C * A;

    int *students = malloc(total_notas * sizeof(*students));
    double *media = malloc(total_alunos * sizeof(*media));
    Stats (*cidade_stats)[C] = malloc((size_t)R * sizeof(*cidade_stats));
    Stats *regiao_stats = malloc((size_t)R * sizeof(*regiao_stats));
    
    if (students == NULL || media == NULL || cidade_stats == NULL || regiao_stats == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente.\n");
        free(students);
        free(media);
        free(cidade_stats);
        free(regiao_stats);
        return 1;
    }

    srand((unsigned int)seed);
    geraTabela(R, C, A, N, students);

    Stats brasil_stats = {0.0, 0.0, 0.0, 0.0, 0.0};
    int best_r = 0;
    int best_city_r = 0;
    int best_city_c = 0;

    omp_set_num_threads(T);

    double start_time = omp_get_wtime();
    // FOR PARA REPETICOES
    for (int rep = 0; rep < NumRep; rep++) {
        // CALCULO MEDIAS
        #pragma omp parallel for collapse(3)
        for (int i = 0; i < R; i++) {
            for (int j = 0; j < C; j++) {
                for (int k = 0; k < A; k++) {
                    int soma = 0;
                    for (int l = 0; l < N; l++) {
                        soma += students[students_idx(C, A, N, i, j, k, l)];
                    }
                    media[media_idx(C, A, i, j, k)] = (double)soma / N;
                }
            }
        }

        #pragma omp parallel for collapse(2)
        for (int i = 0; i < R; i++) {
            for (int j = 0; j < C; j++) {
                cidade_stats[i][j] = calcula_stats(&media[media_idx(C, A, i, j, 0)], A);
            }
        }

        #pragma omp parallel for
        for (int i = 0; i < R; i++) {
            regiao_stats[i] = calcula_stats(&media[media_idx(C, A, i, 0, 0)], C * A);
        }

        brasil_stats = calcula_stats(media, (int)total_alunos);

        // MELHOR REGIAO E MELHOR CIDADE
        best_r = 0;
        best_city_r = 0;
        best_city_c = 0;
        for (int i = 1; i < R; i++) {
            if (regiao_stats[i].media > regiao_stats[best_r].media) {
                best_r = i;
            }
        }
        for (int i = 0; i < R; i++) {
            for (int j = 0; j < C; j++) {
                if (cidade_stats[i][j].media > cidade_stats[best_city_r][best_city_c].media) {
                    best_city_r = i;
                    best_city_c = j;
                }
            }
        }
    }
    double end_time = omp_get_wtime();

    printf("Tabelas para a saida: consideram as notas medias dos alunos.\n\n");

    printf("+-----------------+----------+----------+----------+----------+----------+\n");
    printf("| %-15s | %-8s | %-8s | %-8s | %-8s | %-8s |\n",
           "Cidades", "Min Nota", "Max Nota", "Mediana", "Media", "DsvPdr");
    printf("+-----------------+----------+----------+----------+----------+----------+\n");
    for (int i = 0; i < R; i++) {
        for (int j = 0; j < C; j++) {
            char label[32];
            snprintf(label, sizeof(label), "R=%d, C=%d", i, j);
            print_linha(label, cidade_stats[i][j]);
        }
    }
    printf("+-----------------+----------+----------+----------+----------+----------+\n\n");

    printf("+-----------------+----------+----------+----------+----------+----------+\n");
    printf("| %-15s | %-8s | %-8s | %-8s | %-8s | %-8s |\n",
           "Regioes", "Min Nota", "Max Nota", "Mediana", "Media", "DsvPdr");
    printf("+-----------------+----------+----------+----------+----------+----------+\n");
    for (int i = 0; i < R; i++) {
        char label[32];
        snprintf(label, sizeof(label), "R=%d", i);
        print_linha(label, regiao_stats[i]);
    }
    printf("+-----------------+----------+----------+----------+----------+----------+\n\n");

    printf("+-----------------+----------+----------+----------+----------+----------+\n");
    printf("| %-15s | %-8s | %-8s | %-8s | %-8s | %-8s |\n",
           "Brasil", "Min Nota", "Max Nota", "Mediana", "Media", "DsvPdr");
    printf("+-----------------+----------+----------+----------+----------+----------+\n");
    print_linha("Total", brasil_stats);
    printf("+-----------------+----------+----------+----------+----------+----------+\n\n");

    char best_reg_media[32];
    char best_city_media[32];
    format_pt(regiao_stats[best_r].media, best_reg_media, sizeof(best_reg_media));
    format_pt(cidade_stats[best_city_r][best_city_c].media, best_city_media, sizeof(best_city_media));
    printf("Premiacao\n");
    printf("Melhor regiao: R%d (%s)\n", best_r, best_reg_media);
    printf("Melhor cidade: R%d-C%d (%s)\n", best_city_r, best_city_c, best_city_media);

    printf("Tempo de execucao: %.6f segundos medio\n", (end_time - start_time) / NumRep);
    Avaliacao((float)((end_time - start_time)/NumRep * 1000000)); 

    free(students);
    free(media);
    free(cidade_stats);
    free(regiao_stats);
    return 0;
}

static size_t students_idx(int C, int A, int N, int i, int j, int k, int l) {
    return (((size_t)i * C + j) * A + k) * N + l;
}

static size_t media_idx(int C, int A, int i, int j, int k) {
    return ((size_t)i * C + j) * A + k;
}

void geraTabela(int R, int C, int A, int N, int *students) {
    for (int i = 0; i < R; i++) {
        for (int j = 0; j < C; j++) {
            for (int k = 0; k < A; k++) {
                for (int l = 0; l < N; l++) {
                    students[students_idx(C, A, N, i, j, k, l)] = rand() % 10 + 1;
                }
            }
        }
    }
}

static int cmp_double(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

Stats calcula_stats(const double *v, int n) {
    Stats s = {0.0, 0.0, 0.0, 0.0, 0.0};
    if (n <= 0) {
        return s;
    }

    s.min_nota = v[0];
    s.max_nota = v[0];
    double soma = 0.0;
    for (int i = 0; i < n; i++) {
        if (v[i] < s.min_nota) s.min_nota = v[i];
        if (v[i] > s.max_nota) s.max_nota = v[i];
        soma += v[i];
    }
    s.media = soma / n;

    double soma_quadrados = 0.0;
    for (int i = 0; i < n; i++) {
        double d = v[i] - s.media;
        soma_quadrados += d * d;
    }
    s.dsv_pdr = sqrt(soma_quadrados / n);

    double *tmp = malloc((size_t)n * sizeof(*tmp));
    if (tmp == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente para calcular mediana.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++) {
        tmp[i] = v[i];
    }
    qsort(tmp, (size_t)n, sizeof(*tmp), cmp_double);
    if (n % 2 == 0) {
        s.mediana = (tmp[n / 2 - 1] + tmp[n / 2]) / 2.0;
    } else {
        s.mediana = tmp[n / 2];
    }
    free(tmp);

    return s;
}

void format_pt(double valor, char *out, size_t out_size) {
    snprintf(out, out_size, "%.1f", valor);
    for (char *p = out; *p != '\0'; p++) {
        if (*p == '.') {
            *p = ',';
        }
    }
}

void print_linha(const char *label, Stats s) {
    char min_s[32];
    char max_s[32];
    char med_s[32];
    char mean_s[32];
    char std_s[32];

    format_pt(s.min_nota, min_s, sizeof(min_s));
    format_pt(s.max_nota, max_s, sizeof(max_s));
    format_pt(s.mediana, med_s, sizeof(med_s));
    format_pt(s.media, mean_s, sizeof(mean_s));
    format_pt(s.dsv_pdr, std_s, sizeof(std_s));

    printf("| %-15s | %8s | %8s | %8s | %8s | %8s |\n",
           label, min_s, max_s, med_s, mean_s, std_s);
}
void Avaliacao(float valor) {
    FILE *arquivo = fopen("avaliacao.txt", "a");  // "a" = adiciona no final do arquivo
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }

    fprintf(arquivo, "%f\n", valor);
    fclose(arquivo);
}