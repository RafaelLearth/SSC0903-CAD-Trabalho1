# Trabalho de Computação de Alto Desempenho

Este projeto foi desenvolvido para comparar o desempenho de duas versões de uma mesma solução computacional:

- uma implementação sequencial;
- uma implementação paralela utilizando OpenMP.

Além disso, o projeto inclui um código de avaliação que lê os tempos de execução registrados em um arquivo `.txt` para realizar a análise de desempenho.

## Estrutura do projeto

Os principais arquivos do projeto são:

- `studentsseq.c` — implementação sequencial;
- `studentspar.c` ou `studentspar2.c` — implementação paralela com OpenMP;
- `avaliacao_cod.c` — código responsável por ler o arquivo de tempos e calcular a análise de desempenho;
- `Makefile` — automatiza a compilação e a execução do projeto;
- `avaliacao.txt` — arquivo onde os tempos de execução são registrados.

## Objetivo

O objetivo deste trabalho é analisar o impacto da paralelização no tempo de execução do programa, comparando a versão sequencial com a versão paralela e avaliando o ganho de desempenho obtido.

## Requisitos

Para compilar e executar o projeto, é necessário ter instalado:

- `gcc`
- `make`
- suporte a OpenMP

Em sistemas Linux, normalmente basta ter o GCC instalado com suporte à flag `-fopenmp`.

## Compilação

Para compilar o projeto, utilize:

```bash
make
```

## Execução

Para executar o projeto, utilize:

```bash
make run
```

Esses comandos utilizam o `Makefile` para compilar e rodar os programas automaticamente.

## Funcionamento

As versões sequencial e paralela executam o processamento principal e registram o tempo de execução em um arquivo de texto.

Depois disso, o código de avaliação lê esse arquivo para realizar a análise dos tempos obtidos.

De forma geral, o fluxo do projeto é:

1. compilar os programas com `make`;
2. executar com `make run`;
3. armazenar os tempos no arquivo `avaliacao.txt`;
4. usar o código de avaliação para ler os tempos e comparar o desempenho.

## Entrada

Os programas recebem como entrada valores que representam a configuração da execução, como quantidade de regiões, cidades, alunos, notas, número de threads e semente aleatória.

De forma geral, a entrada segue o formato:

```text
R C A N T
seed
```

Onde:

- `R` = número de regiões;
- `C` = número de cidades por região;
- `A` = número de alunos por cidade;
- `N` = número de notas por aluno;
- `T` = número de threads;
- `seed` = semente para geração aleatória.

## Saída

Os programas exibem estatísticas calculadas a partir das médias dos alunos, incluindo:

- nota mínima;
- nota máxima;
- mediana;
- média;
- desvio padrão.

Essas estatísticas são apresentadas por cidade, por região e no total geral.

Além disso, o programa informa:

- a melhor região;
- a melhor cidade;
- o tempo médio de execução.

## Avaliação de desempenho

O arquivo `avaliacao.txt` armazena os tempos de execução das versões sequencial e paralela.

O programa `avaliacao_cod.c` lê esses valores e realiza a comparação entre eles, calculando o speedup da versão paralela em relação à sequencial.

A fórmula utilizada é:

```text
speedup = tempo_sequencial / tempo_paralelo
```

Se o valor do speedup for maior que 1, a versão paralela apresentou melhor desempenho.

Se o valor for menor que 1, a versão paralela foi mais lenta que a versão sequencial.

## Observações

- O arquivo `avaliacao.txt` é aberto em modo de adição, então os tempos de execuções anteriores continuam armazenados no arquivo.
- Para reiniciar a análise do zero, recomenda-se apagar o arquivo antes de rodar novos testes.

Exemplo:

```bash
rm -f avaliacao.txt
make run
```

## Autor

Trabalho desenvolvido para a disciplina de Computação de Alto Desempenho.
