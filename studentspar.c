/******************************************************************************
Universidade de São Paulo - ICMC/EESC - Curso de Eng. de Computação
SSC0903 - Computação de Alto Desempenho (2026/1) – Trab01

Integrantes do Grupo 5:

- Diogo Barboza de Souza - 12745657
- Gabriela Passos de Andrade - 12625142
- João Marcelo Moreira Trovão Filho - 13676332
- Lucas Fernando Witzel Camara Ferreira - 11800650
- Matheus dos Santos Ines - 12546784
- Rafael Cunha Bejes Learth - 13676367

Codigo Paralelo: studentspar.c
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

// Numero de repeticoes para o calculo do tempo medio
#define NUM_REP 33

// Optamos por tratar os arrays contendo os dados dos estudantes como estruturas 
// unidimensionais para melhor desempenho. Por isso, foram criadas macros que facilitam 
// a leitura do codigo no momento da indexacao de posicoes.

// Macros para indexacao de arrays multidimensionais
#define INDEX_1DIM(r) ((size_t)(r))
#define INDEX_2DIM(r, c) ((size_t)(r)*(C) + (c))
#define INDEX_3DIM(r,c,a) ((size_t)(r)*(C)*(A) + (size_t)(c)*(A) + (a))
#define INDEX_4DIM(r,c,a,n) ((size_t)(r)*(C)*(A)*(N) + (size_t)(c)*(A)*(N) + (size_t)(a)*(N) + (n))

// Estrutura para armazenar os dados estatisticos
typedef struct {
    double min_nota;
    double max_nota;
    double mediana;
    double media;
    double desvio_padrao;
} Dados;

/*
 * Funcao para gerar a tabela com todas as notas de cada estudante
 */
void gerarTabela(int R, int C, int A, int N, double* estudantes) {
    for (int r = 0; r < R; r++) {
        for (int c = 0; c < C; c++) {
            for (int a = 0; a < A; a++) {
                for (int n = 0; n < N; n++) {
                    estudantes[INDEX_4DIM(r, c, a, n)] = (double)(rand() % 1001) / 10.0;
                }
            }
        }
    }
}

/*
 * Funcao para comparar dois valores do tipo double
 */
static inline int compararDouble(const void *a, const void *b) {
    double da = *(const double*) a;
    double db = *(const double*) b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

/*
 * Funcao para trocar o valor de duas variaveis double
 */
static inline void trocarDouble(double *a, double *b) {
    double aux = *a;
    *a = *b;
    *b = aux;
}

/*
 * Implementacao do algoritmo Quickselect para encontrar o valor de uma posicao especifica
 * caso o vetor estivesse ordenado. Utilizada para otimizar o calculo da mediana: O(n)
 */
double quickselect(double *vetor, int tam, int indice) {
    int i, j;
    double pivo;
    int esq = 0;
    int dir = tam - 1;

    while (esq < dir) {
        pivo = vetor[esq + (dir - esq) / 2];
        i = esq;
        j = dir;

        while (i <= j) {
            while (vetor[i] < pivo) i++;
            while (vetor[j] > pivo) j--;
            if (i <= j) {
                trocarDouble(&vetor[i], &vetor[j]);
                i++;
                j--;
            }
        }

        if (indice <= j) {
            dir = j;
        } 
        else if (indice >= i) {
            esq = i;
        }
        else {
            return vetor[indice];
        }
    }

    return vetor[esq];
}

/*
 * Funcao para calcular os dados estatisticos: min, max, mediana, media e desvio padrao
 */
Dados calcularDados(const double *valores, double *aux_vetor, int lim) {
    Dados d = {0};
    double soma = 0.0, soma_quadrados = 0.0;
    double atual_valor;
    double min_valor = valores[0];
    double max_valor = valores[0];

    // =====================================================
    // Paralelisacao dos calculos de notas minimas e maximas
    // =====================================================
    #pragma omp simd reduction(+:soma, soma_quadrados) private(atual_valor) reduction(min:min_valor) reduction(max:max_valor)
    for (int i = 0; i < lim; i++) {
        atual_valor = valores[i];

        // Ternários sao usados para evitar desvios condicionais complexos dentro do loop SIMD
        min_valor = (atual_valor < min_valor) ? atual_valor : min_valor;
        max_valor = (atual_valor > max_valor) ? atual_valor : max_valor;
        
        soma += atual_valor;
        soma_quadrados += (atual_valor * atual_valor);
        aux_vetor[i] = atual_valor; 
    }

    // Calculo da media e do desvio padrao
    d.min_nota = min_valor;
    d.max_nota = max_valor;
    d.media = soma / lim;

    double variancia = (soma_quadrados / lim) - (d.media * d.media);
    d.desvio_padrao = sqrt(variancia < 0 ? 0 : variancia);

    // Calculo da mediana
    if (lim % 2 == 0) {
        double med1 = quickselect(aux_vetor, lim, lim / 2 - 1);
        double med2 = quickselect(aux_vetor, lim, lim / 2);
        d.mediana = (med1 + med2) / 2.0;
    }
    else {
        d.mediana = quickselect(aux_vetor, lim, lim / 2);
    }

    return d;
}

/*
 * Funcao para formatar um valor double no formato brasileiro (com virgula)
 */
void formatarPT(double valor_original, char *saida, size_t tam_saida) {
    snprintf(saida, tam_saida, "%.1f", valor_original);
    for (char *c = saida; *c != '\0'; c++) {
        if (*c == '.') *c = ',';
    }
}

/*
 * Funcao para escrever resultados dos tempos num arquivo texto
 */
int escreveArquivo(const double *tempos_exec, int num_rep, int T) {
    FILE *arq = fopen("saidas/tempospar.txt", "a");
    if (arq == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo de saida.\n");
        return 1;
    }

    fprintf(arq, "threads(T)= %d\n", T);

    for (int i = 0; i < num_rep; i++) {
        fprintf(arq, "%.10f\n", tempos_exec[i]);
    }

    fprintf(arq, "\n"); // linha em branco entre execucoes
    fclose(arq);
    return 0;
}

/*
 * Funcao para imprimir uma linha da tabela com os dados estatisticos
 */
 void printLinha(const char *texto, Dados d) {
    char min_pt[32], max_pt[32], mediana_pt[32], media_pt[32], desvio_pt[32];
    formatarPT(d.min_nota, min_pt, sizeof(min_pt));
    formatarPT(d.max_nota, max_pt, sizeof(max_pt));
    formatarPT(d.mediana, mediana_pt, sizeof(mediana_pt));
    formatarPT(d.media, media_pt, sizeof(media_pt));
    formatarPT(d.desvio_padrao, desvio_pt, sizeof(desvio_pt));

    printf("| %-15s | %8s | %8s | %8s | %8s | %8s |\n", texto, min_pt, max_pt, mediana_pt, media_pt, desvio_pt);
}

/**
 * Funcao para imprimir as tabelas com os resultado dos dados estatisticos
 */
void printTabelas(int R, int C, Dados *cidade_Dados, Dados *regiao_Dados, Dados brasil_Dados, int melhor_regiao, int melhor_cidade_regiao, int melhor_cidade, double tempo_total) {
    printf("Tabelas para a saida: consideram as notas medias dos alunos.\n\n");

    printf("+-----------------+----------+----------+----------+----------+----------+\n");
    printf("| %-15s | %-8s | %-8s | %-8s | %-8s | %-8s |\n", "Cidades", "Min Nota", "Max Nota", "Mediana", "Media", "DsvPad");
    printf("+-----------------+----------+----------+----------+----------+----------+\n");
    for (int r = 0; r < R; r++) {
        for (int c = 0; c < C; c++) {
            char label[32];
            snprintf(label, sizeof(label), "R=%d, C=%d", r, c);
            printLinha(label, cidade_Dados[INDEX_2DIM(r, c)]);
        }
    }
    printf("+-----------------+----------+----------+----------+----------+----------+\n\n");

    printf("+-----------------+----------+----------+----------+----------+----------+\n");
    printf("| %-15s | %-8s | %-8s | %-8s | %-8s | %-8s |\n", "Regioes", "Min Nota", "Max Nota", "Mediana", "Media", "DsvPad");
    printf("+-----------------+----------+----------+----------+----------+----------+\n");
    for (int r = 0; r < R; r++) {
        char label[32];
        snprintf(label, sizeof(label), "R=%d", r);
        printLinha(label, regiao_Dados[INDEX_1DIM(r)]);
    }
    printf("+-----------------+----------+----------+----------+----------+----------+\n\n");

    printf("+-----------------+----------+----------+----------+----------+----------+\n");
    printf("| %-15s | %-8s | %-8s | %-8s | %-8s | %-8s |\n", "Brasil", "Min Nota", "Max Nota", "Mediana", "Media", "DsvPad");
    printf("+-----------------+----------+----------+----------+----------+----------+\n");
    printLinha("", brasil_Dados);
    printf("+-----------------+----------+----------+----------+----------+----------+\n\n");

    printf("+----------------------+----------------------+------------------+\n");
    printf("| %-20s | %-20s | %-16s |\n", "Premiacao", "Regicao/Cidade", "Media Aritmetica");
    printf("+----------------------+----------------------+------------------+\n");

    char media_regiao[32], media_cidade[32];
    formatarPT(regiao_Dados[INDEX_1DIM(melhor_regiao)].media, media_regiao, sizeof(media_regiao));
    formatarPT(cidade_Dados[INDEX_2DIM(melhor_cidade_regiao, melhor_cidade)].media, media_cidade, sizeof(media_cidade));

    char id_regiao[32], id_cidade[32];
    snprintf(id_regiao, sizeof(id_regiao), "R=%d", melhor_regiao);
    snprintf(id_cidade, sizeof(id_cidade), "R=%d, C=%d", melhor_cidade_regiao, melhor_cidade);

    printf("| %-20s | %-20s | %16s |\n", "Melhor Regiao", id_regiao, media_regiao);
    printf("| %-20s | %-20s | %16s |\n", "Melhor Cidade", id_cidade, media_cidade);
    printf("+----------------------+----------------------+------------------+\n");

    printf("\nTempo medio de execucao: %.6f (s)\n\n", tempo_total / NUM_REP);
}

/*
 * Funcao principal
 */
int main(int argc, char *argv[]) {
    (void) argc;
    if (argv[1] == NULL) {
        fprintf(stderr, "Erro: arquivo de entrada nao indicado.\n");
        return 1;
    }
    
    // ===================================================
    // Lendo os parametros de entrada do arquivo input.txt
    // ===================================================
    int R, C, A, N, T, seed;
    char *arquivo_entrada = argv[1];

    FILE* fp = fopen(arquivo_entrada, "r");
    if (fp == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo de entrada.\n");
        return 1;
    }
    if (fscanf(fp, "%d %d %d %d %d %d", &R, &C, &A, &N, &T, &seed) != 6) {
        fprintf(stderr, "Entrada invalida.\n");
        fclose(fp);
        return 1;
    }
    fclose(fp);

    if (R <= 0 || C <= 0 || A <= 0 || N <= 0 || T <= 0 || seed <= 0) {
        fprintf(stderr, "R, C, A, N, T e seed devem ser positivos.\n");
        return 1;
    }

    srand((unsigned int)seed);

    omp_set_num_threads(T);

    // =====================================================
    // Alocacao de memoria dinamica para as tabelas de dados
    // =====================================================
    Dados brasil_Dados;

    Dados *cidade_Dados = malloc((size_t) R * C * sizeof(*cidade_Dados));
    if (cidade_Dados == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente para criar a tabela de dados das cidades.\n");
        return 1;
    }

    Dados *regiao_Dados = malloc((size_t) R * sizeof(*regiao_Dados));
    if (regiao_Dados == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente para criar a tabela de dados das regioes.\n");
        free(cidade_Dados);
        return 1;
    }

    double* estudantes = malloc((size_t) R * C * A * N * sizeof(*estudantes));
    if (estudantes == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente para criar a tabela de estudantes.\n");
        free(cidade_Dados);
        free(regiao_Dados);
        return 1;
    }
    
    double *media = malloc((size_t) R * C * A * sizeof(*media));
    if (media == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente para criar a tabela de medias.\n");
        free(cidade_Dados);
        free(regiao_Dados);
        free(estudantes);
        return 1;
    }

    double *aux_vetor_brasil = malloc((size_t) R * C * A * sizeof(*aux_vetor_brasil));
    if (aux_vetor_brasil == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente para criar o vetor auxiliar de dados do Brasil.\n");
        free(cidade_Dados);
        free(regiao_Dados);
        free(estudantes);
        free(media);
        return 1;
    }

    double **aux_vetor_cidade_threads = malloc((size_t) T * sizeof(*aux_vetor_cidade_threads));
    if (aux_vetor_cidade_threads == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente para criar o vetor auxiliar de cidade para as threads.\n");
        free(cidade_Dados);
        free(regiao_Dados);
        free(estudantes);
        free(media);
        free(aux_vetor_brasil);
        return 1;
    }

    for (int i = 0; i < T; i++) {
        aux_vetor_cidade_threads[i] = malloc((size_t) A * sizeof(**aux_vetor_cidade_threads));
        if (aux_vetor_cidade_threads[i] == NULL) {
            fprintf(stderr, "Erro: memoria insuficiente para criar o vetor de cidade para uma thread especifica.\n");
            free(cidade_Dados);
            free(regiao_Dados);
            free(estudantes);
            free(media);
            free(aux_vetor_brasil);
            for (int j = 0; j < i; j++) free(aux_vetor_cidade_threads[j]);
            free(aux_vetor_cidade_threads);
            return 1;
        }
    }

    double **aux_vetor_regiao_threads = malloc((size_t) T * sizeof(*aux_vetor_regiao_threads));
    if (aux_vetor_regiao_threads == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente para criar o vetor auxiliar de regiao para as threads.\n");
        free(cidade_Dados);
        free(regiao_Dados);
        free(estudantes);
        free(media);
        free(aux_vetor_brasil);
        for (int i = 0; i < T; i++) free(aux_vetor_cidade_threads[i]);
        free(aux_vetor_cidade_threads);
        return 1;
    }

    for (int i = 0; i < T; i++) {
        aux_vetor_regiao_threads[i] = malloc((size_t) C * A * sizeof(**aux_vetor_regiao_threads));
        if (aux_vetor_regiao_threads[i] == NULL) {
            fprintf(stderr, "Erro: memoria insuficiente para criar o vetor de regiao para uma thread especifica.\n");
            free(cidade_Dados);
            free(regiao_Dados);
            free(estudantes);
            free(media);
            free(aux_vetor_brasil);
            for (int j = 0; j < T; j++) free(aux_vetor_cidade_threads[j]);
            free(aux_vetor_cidade_threads);
            for (int k = 0; k < i; k++) free(aux_vetor_regiao_threads[k]);
            free(aux_vetor_regiao_threads);
            return 1;
        }
    }

    double *tempos_exec = malloc((size_t) NUM_REP* sizeof(*tempos_exec));
    if (tempos_exec == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente para criar o vetor tempos de exec de dados do Brasil.\n");
        free(cidade_Dados);
        free(regiao_Dados);
        free(estudantes);
        free(media);
        return 1;
    }

    // Preenchendo a tabela de estudantes com notas pseudo-aleatorias
    gerarTabela(R, C, A, N, estudantes);

    double tempo_total = 0.0;
    int melhor_regiao, melhor_cidade, melhor_cidade_regiao;
    
    // ===================================================================================
    // Loop principal para o calculo dos dados estatisticos e medicao do tempo de execucao
    // ===================================================================================
    for (int rep = 0; rep < NUM_REP; rep++) {
        // Iniciando medicao de tempo
        double inicio_tempo = omp_get_wtime();

        // ===============================================
        // Paralelisacao dos calculos das medias por aluno
        // ===============================================
        double soma;
        #pragma omp parallel for simd collapse(3) private(soma)
        for (int r=0; r<R; r++) {
            for (int c=0; c<C; c++) {
                for (int a=0; a<A; a++) {
                    soma = 0;
                    for (int n=0; n<N; n++) {
                        soma += estudantes[INDEX_4DIM(r, c, a, n)];
                    }
                    media[INDEX_3DIM(r, c, a)] = (double) soma / N;
                }
            }
        }
        
        // ===================================================================
        // Paralelisacao dos calculos das estatisticas por cidade e por regiao
        // ===================================================================
        #pragma omp parallel
        {
            int id_thread = omp_get_thread_num();
            double *aux_cidade = aux_vetor_cidade_threads[id_thread];
            double *aux_regiao = aux_vetor_regiao_threads[id_thread];
            
            #pragma omp for simd collapse(2) schedule(dynamic) nowait
            for (int r = 0; r < R; r++) {
                for (int c = 0; c < C; c++) {
                    cidade_Dados[INDEX_2DIM(r, c)] = calcularDados(&media[INDEX_3DIM(r, c, 0)], aux_cidade, A);
                }
            }

            #pragma omp for simd schedule(dynamic)
            for (int r = 0; r < R; r++) {
                regiao_Dados[INDEX_1DIM(r)] = calcularDados(&media[(size_t) r * (C * A)], aux_regiao, C * A);
            }
        }

        // ========================================================================================
        // Paralelisacao dos calculos das estatisticas do Brasil e definindo Melhor Regiao e Cidade
        // ========================================================================================
        #pragma omp parallel
        {
            #pragma omp sections
            {
                #pragma omp section
                {
                    brasil_Dados = calcularDados(media, aux_vetor_brasil, R * C * A);
                }

                #pragma omp section
                {
                    melhor_regiao = 0; melhor_cidade = 0; melhor_cidade_regiao = 0;

                    for (int r=0; r<R; r++) {
                        if (regiao_Dados[r].media > regiao_Dados[melhor_regiao].media) {
                            melhor_regiao = r;
                        }
                        for (int c=0; c<C; c++) {
                            if (cidade_Dados[INDEX_2DIM(r, c)].media > cidade_Dados[INDEX_2DIM(melhor_cidade_regiao, melhor_cidade)].media) {
                                melhor_cidade_regiao = r;
                                melhor_cidade = c;
                            }
                        }
                    }
                }
            }
        }

        // ================
        // METODO COM TASKS (por enquanto ignorar, vai ser uma analise posterior so pra comparar SECTION X TASK)
        // ================
        /*        
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task
                {
                    brasil_Dados = calcularDados(media, aux_vetor_brasil, R * C * A);
                }

                #pragma omp task
                {
                    melhor_regiao = 0; melhor_cidade = 0; melhor_cidade_regiao = 0;

                    for (int r=0; r<R; r++) {
                        if (regiao_Dados[r].media > regiao_Dados[melhor_regiao].media) {
                            melhor_regiao = r;
                        }
                        for (int c=0; c<C; c++) {
                            if (cidade_Dados[INDEX_2DIM(r, c)].media > cidade_Dados[INDEX_2DIM(melhor_cidade_regiao, melhor_cidade)].media) {
                                melhor_cidade_regiao = r;
                                melhor_cidade = c;
                            }
                        }
                    }
                }
            }
        }
        */

        // Finalizando medicao de tempo e calculando tempo total
        double final_tempo = omp_get_wtime();
        tempos_exec[rep] = final_tempo - inicio_tempo;
    }

    // ===============================================================================
    // Impressao dos resultados e liberacao da memoria alocada. Finalizacao do codigo.
    // ===============================================================================
    //printTabelas(R, C, cidade_Dados, regiao_Dados, brasil_Dados, melhor_regiao, melhor_cidade_regiao, melhor_cidade, tempo_total);
    escreveArquivo(tempos_exec, NUM_REP, T);

    free(cidade_Dados);
    free(regiao_Dados);
    free(estudantes);
    free(media);
    free(aux_vetor_brasil);
    for (int i = 0; i < T; i++) free(aux_vetor_cidade_threads[i]);
    free(aux_vetor_cidade_threads);
    for (int i = 0; i < T; i++) free(aux_vetor_regiao_threads[i]);
    free(aux_vetor_regiao_threads);

    return 0;
}
