#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

void *mallocSafe(size_t nbytes)
{
    void *ponteiro = malloc(nbytes);
    if (ponteiro == NULL)
    {
        printf("Socorro! malloc devolveu NULL!\n");
        exit(EXIT_FAILURE);
    }
    return ponteiro;
}

/*A funcao troca o valor dos indices pro sort, pta deixar o codigo menos poluido*/
void troca(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

int separa(int d[], int id[], int p, int r)
{
    int x = d[id[r]];
    int i = p - 1;
    for (int j = p; j < r; j++)
    {
        if (d[id[j]] >= x)
        {
            i++;
            troca(&id[i], &id[j]);
        }
    }
    troca(&id[i + 1], &id[r]);
    return i + 1;
}

void quicksort(int d[], int id[], int p, int r)
{
    if (p < r)
    {
        int q = separa(d, id, p, r);
        quicksort(d, id, p, q - 1);
        quicksort(d, id, q + 1, r);
    }
}

/*Usa o quicksort para ordenacao indireta do vetor ID*/
void sortInd(int n, int d[], int id[])
{
    for (int i = 0; i < n; i++)
    {
        id[i] = i;
    }
    quicksort(d, id, 0, n - 1);
}

/*funcao recursiva para gerar e testar escalonamentos pras maquinas*/
void escalona(int m, int n, int d[], int tarefaAtual, int cargas[],
              int escalonamento[], int escalOtimo[], int *tarefasOrdenadas,
              int maiorCarga, int *melhorEscalonamento)
{
    if (tarefaAtual == n)
    {
        /*Se o escalonamento testado atual for melhor que o melhor escalonamento
        (a maquina mais ocupada tem um tempo menor que a duracao do escalonamento), salvamos ele*/
        if (maiorCarga < *melhorEscalonamento)
        {
            *melhorEscalonamento = maiorCarga;
            memcpy(escalOtimo, escalonamento, n * sizeof(int));
        }
        return;
    }

    // Calculamos o quanto falta para escalonar e qual a maior tarefa restante
    int tempoRestanteParaEscalonar = 0;
    int maiorTarefaRestante = 0;
    for (int i = tarefaAtual; i < n; i++)
    {
        int tempoTarefa = d[tarefasOrdenadas[i]];
        tempoRestanteParaEscalonar += tempoTarefa;
        if (tempoTarefa > maiorTarefaRestante)
            maiorTarefaRestante = tempoTarefa;
    }

    // Calculamos o tempo total ja escalonado de tarefas
    int tempoEscalonado = 0;
    for (int i = 0; i < m; i++)
        tempoEscalonado += cargas[i];

    /*Calculamos a carga media dividindo o tempo total das tarefas pelas maquinas*/
    double cargaMedia = (tempoRestanteParaEscalonar + tempoEscalonado) / (double)m;
    /*Colocamos um limite superior para cortarmos solucoes do escalonamento, mas sem cortar demais.
    Para isso, escolhemos primeiro o maior numero entre a carga media e a maior tarefa ainda nao escalonada.
    Depois, escolhemos entre isso e a duracao do escalonamento anterior.
    Por exemplo, podemos ter tarefas de duracao 2 e 3 escalonadas em 2 maquinas, faltando a de 7,
    sendo a maiorCarga do escalonamento anterior 7
    Entao escolheremos como limite entre 12/2 = 6 e 7; e depois escolhemos entre 7 e 7; ficando 7*/
    int limiteSuperior = (int)ceil(fmax(maiorCarga, fmax(cargaMedia, maiorTarefaRestante)));

    if (limiteSuperior >= *melhorEscalonamento)
    {
        // Desistimos do escalonamento que estamos tentando se esse limite for maior que o melhor escalonamento atual
        return;
    }

    int tempoTarefaAtual = d[tarefasOrdenadas[tarefaAtual]];

    for (int i = 0; i < m; i++)
    {
        // Se nossa atribuicao de carga a tarefa atual exceder o melhor escalonamento, tentamos a proxima
        if (cargas[i] + tempoTarefaAtual >= *melhorEscalonamento)
        {
            continue;
        }

        // Atribuimos a tarefa a maquina
        escalonamento[tarefasOrdenadas[tarefaAtual]] = i;
        cargas[i] += tempoTarefaAtual;

        int novoMaximo;
        if (cargas[i] > maiorCarga)
        {
            novoMaximo = cargas[i];
        }
        else
        {
            novoMaximo = maiorCarga;
        }

        // Usamos a recursao pra tentar mais :)
        escalona(m, n, d, tarefaAtual + 1, cargas, escalonamento,
                 escalOtimo, tarefasOrdenadas, novoMaximo, melhorEscalonamento);

        // Se ela retornar, desfazemos a atribuicao (backtracking)
        cargas[i] -= tempoTarefaAtual;
    }
}

int SolucaoOtima(int m, int n, int d[], int escalOtimo[])
{
    /*inicializamos uma variavel que guarda a duracao do melhor escalonamento com int_max,
    porque inicializar com um escalonamento de uma heuristica tipo o sorted graham estava fazendo
    com que meu algoritmo nao encontrasse a solucao otima em alguns casos fora dos exemplos do EP*/
    /*Por isso, atribuo inicialmente um numero arbitrariamente grande, e ele vai ser atualizado logo
    na primeira chamada do escalonamento, isso faz o escalonamento funcionar.*/
    int melhorEscalonamento = INT_MAX;

    int *cargas = mallocSafe(m * sizeof(int));
    int *escalonamento = mallocSafe(n * sizeof(int));
    int *tarefasOrdenadas = mallocSafe(n * sizeof(int));

    // Inicializamos o vetor de cargas pra 0
    for (int i = 0; i < m; i++)
        cargas[i] = 0;

    sortInd(n, d, tarefasOrdenadas);

    escalona(m, n, d, 0, cargas, escalonamento, escalOtimo, tarefasOrdenadas, 0, &melhorEscalonamento);
    free(cargas);
    free(escalonamento);
    free(tarefasOrdenadas);

    return melhorEscalonamento;
}

int main(int argc, char *argv[])
{
    /* argc = numero de argumentos na linha de comando */
    /* argv = vetor de apontadores para strings contendo esses argumentos */

    FILE *entrada; /* declaracao da variavel para o arquivo de entrada */
    if (argc == 1)
    {
        printf("Uso: %s <arquivo de entrada>\n", argv[0]);
        return -1;
    }

    if ((entrada = fopen(argv[1], "r")) == NULL)
    {
        printf("%s: arquivo de entrada %s nao pode ser aberto.\n", argv[0], argv[1]);
        return -1;
    }

    int m, n;
    fscanf(entrada, "%d %d", &m, &n);

    int *duracao = mallocSafe(n * sizeof(int));
    for (int i = 0; i < n; i++)
    {
        fscanf(entrada, "%d", &duracao[i]);
    }
    fclose(entrada);

    printf("m = %d      n = %d", m, n);

    printf("\n\nTarefas: ");
    for (int i = 0; i < n; i++)
        printf("%d  ", i);

    printf("\nDuracao: ");
    for (int i = 0; i < n; i++)
        printf("%d  ", duracao[i]);

    int *escalOtimo = mallocSafe(n * sizeof(int));
    int duracaoOtimo = SolucaoOtima(m, n, duracao, escalOtimo);

    printf("\n\nEscalonamento otimo:\n");
    printf("Tarefa Maquina\n");
    for (int i = 0; i < n; i++)
    {
        printf("%d      %d\n", i, escalOtimo[i]);
    }

    printf("\nDuracao do escalonamento: %d", duracaoOtimo);
    free(escalOtimo);
    return 0;
}
