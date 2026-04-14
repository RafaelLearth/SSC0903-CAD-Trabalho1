#include <stdio.h>

int main() {
    FILE *arquivo = fopen("avaliacao.txt", "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    float primeiro, segundo;

    while (fscanf(arquivo, "%f", &primeiro) == 1) {
        if (fscanf(arquivo, "%f", &segundo) != 1) {
            printf("Arquivo com quantidade ímpar de números.\n");
            break;
        }

        if (segundo == 0) {
            printf("Nao e possivel dividir por zero.\n");
        } else if(primeiro / segundo <1){
            printf("Speedup calculado(tempo seq/tempo par):%f / %f = %.2f SPEEDUP RUIM, TEMPO PARALELO MAIOR\n", (float)primeiro/1000000, (float)segundo/1000000, (float)primeiro / segundo);
        }else{
            printf("Speedup calculado(tempo seq/tempo par):%f / %f = %.2f\n", (float)primeiro/1000000, (float)segundo/1000000, (float)primeiro / segundo);
        }
    }

    fclose(arquivo);
    return 0;
}