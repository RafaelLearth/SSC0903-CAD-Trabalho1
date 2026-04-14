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
Stats calcula_stats(const double *v, int n, double *tmp);
void format_pt(double valor, char *out, size_t out_size);
void print_linha(const char *label, Stats s);
void Avaliacao(float valor);
static double quickselect_double(double *v, int n, int k);
static void swap_double(double *a, double *b);

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
    int max_stats_n = C * A > A ? C * A : A;
    double *stats_workspace = malloc((size_t)T * (size_t)max_stats_n * sizeof(*stats_workspace));
    double *brasil_workspace = malloc(total_alunos * sizeof(*brasil_workspace));
    
    if (students == NULL || media == NULL || cidade_stats == NULL || regiao_stats == NULL ||
        stats_workspace == NULL || brasil_workspace == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente.\n");
        free(students);
        free(media);
        free(cidade_stats);
        free(regiao_stats);
        free(stats_workspace);
        free(brasil_workspace);
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
    for (int rep = 0; rep < NumRep; rep++) {
        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            double *tmp = stats_workspace + (size_t)tid * (size_t)max_stats_n;

            #pragma omp for collapse(3) schedule(static)
            for (int i = 0; i < R; i++) {
                for (int j = 0; j < C; j++) {
                    for (int k = 0; k < A; k++) {
                        int soma = 0;
                        size_t base = students_idx(C, A, N, i, j, k, 0);
                        for (int l = 0; l < N; l++) {
                            soma += students[base + (size_t)l];
                        }
                        media[media_idx(C, A, i, j, k)] = (double)soma / N;
                    }
                }
            }

            #pragma omp for collapse(2) schedule(static)
            for (int i = 0; i < R; i++) {
                for (int j = 0; j < C; j++) {
                    cidade_stats[i][j] = calcula_stats(&media[media_idx(C, A, i, j, 0)], A, tmp);
                }
            }

            #pragma omp for schedule(static)
            for (int i = 0; i < R; i++) {
                regiao_stats[i] = calcula_stats(&media[media_idx(C, A, i, 0, 0)], C * A, tmp);
            }

            #pragma omp single
            {
                brasil_stats = calcula_stats(media, (int)total_alunos, brasil_workspace);
            }
        }

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
    free(stats_workspace);
    free(brasil_workspace);
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

Stats calcula_stats(const double *v, int n, double *tmp) {
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
        tmp[i] = v[i];
        soma += v[i];
    }

    s.media = soma / n;

    double soma_quadrados = 0.0;
    for (int i = 0; i < n; i++) {
        double d = v[i] - s.media;
        soma_quadrados += d * d;
    }
    s.dsv_pdr = sqrt(soma_quadrados / n);

    if (n % 2 == 0) {
        double med1 = quickselect_double(tmp, n, n / 2 - 1);
        double med2 = quickselect_double(tmp, n, n / 2);
        s.mediana = (med1 + med2) / 2.0;
    } else {
        s.mediana = quickselect_double(tmp, n, n / 2);
    }

    return s;
}

static double quickselect_double(double *v, int n, int k) {
    int left = 0;
    int right = n - 1;

    while (left < right) {
        double pivot = v[left + (right - left) / 2];
        int i = left;
        int j = right;

        while (i <= j) {
            while (v[i] < pivot) {
                i++;
            }
            while (v[j] > pivot) {
                j--;
            }
            if (i <= j) {
                swap_double(&v[i], &v[j]);
                i++;
                j--;
            }
        }

        if (k <= j) {
            right = j;
        } else if (k >= i) {
            left = i;
        } else {
            return v[k];
        }
    }

    return v[left];
}

static void swap_double(double *a, double *b) {
    double tmp = *a;
    *a = *b;
    *b = tmp;
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
