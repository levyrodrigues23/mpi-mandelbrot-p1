#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "mandelbrot_calc.h"

/*
 * mandelbrot_mpi.c - Mandelbrot distribuido com MPI (mestre/escravo).
 *
 * Baseado no mestre_escravo_exemplo.c do professor. Aqui cada tarefa e'
 * "calcular uma linha da imagem". O mestre (rank 0) so distribui as
 * linhas, recebe os resultados e monta a figura; os outros processos
 * (escravos) fazem o calculo.
 *
 * As linhas sao entregues uma de cada vez, para quem ficar livre primeiro.
 * Assim, uma maquina mais rapida (ou que pegou linhas mais faceis) acaba
 * calculando mais linhas, e ninguem fica parado esperando.
 */

#define IMG_WIDTH   800
#define IMG_HEIGHT  600
#define MAXITER     20000

/* Mesma regiao (com zoom) usada no mandelbrot.cpp do professor. */
#define ZMIN_R (-0.482536)
#define ZMIN_I (-0.635201)
#define ZMAX_R (-0.565928)
#define ZMAX_I (-0.579588)

#define TAG_TAREFA     110
#define TAG_RESULTADO  111
#define TAG_NOME       112
#define END            -1

/* Nome do computador do escravo de rank r. */
#define NOME(r) (nomes + (size_t) (r) * MPI_MAX_PROCESSOR_NAME)

/* Resultado de uma tarefa: numero da linha + os pixels RGB dela. */
typedef struct {
    int linha;
    unsigned char pixels[IMG_WIDTH * 3];
} LinhaResultado;

/* Calcula a cor de todos os pixels da linha y. */
static void calcula_linha(int y, unsigned char *buf_linha)
{
    int x;
    double dx = (ZMAX_R - ZMIN_R) / (double) IMG_WIDTH;
    double dy = (ZMAX_I - ZMIN_I) / (double) IMG_HEIGHT;

    for (x = 0; x < IMG_WIDTH; x++) {
        double cr = ZMIN_R + (double) x * dx;
        double ci = ZMIN_I + (double) y * dy;
        int iter = calc_point(cr, ci, MAXITER);
        get_color(iter, MAXITER,
                  &buf_linha[x * 3], &buf_linha[x * 3 + 1], &buf_linha[x * 3 + 2]);
    }
}

/* Grava a imagem no formato PPM (cabecalho em texto + bytes RGB). */
static void salvar_ppm(const char *nome_arquivo, unsigned char *imagem,
                       int largura, int altura)
{
    FILE *f = fopen(nome_arquivo, "wb");
    if (f == NULL) {
        perror("Erro ao abrir arquivo de saida");
        return;
    }
    fprintf(f, "P6\n%d %d\n255\n", largura, altura);
    fwrite(imagem, 1, (size_t) largura * altura * 3, f);
    fclose(f);
    printf("Imagem gravada em %s (%dx%d)\n", nome_arquivo, largura, altura);
}

static void mestre(void)
{
    int tarefa = 0;
    int nb_escravos, i;
    char *nomes;
    unsigned char *imagem;
    LinhaResultado resultado;
    MPI_Status stat;

    MPI_Comm_size(MPI_COMM_WORLD, &nb_escravos);
    nb_escravos--;

    nomes = malloc((size_t) (nb_escravos + 1) * MPI_MAX_PROCESSOR_NAME);
    imagem = malloc((size_t) IMG_WIDTH * IMG_HEIGHT * 3);
    if (nomes == NULL || imagem == NULL) {
        fprintf(stderr, "Erro ao alocar memoria\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    /* Cada escravo comeca mandando o nome do computador onde esta. */
    for (i = 1; i <= nb_escravos; i++) {
        MPI_Recv(NOME(i), MPI_MAX_PROCESSOR_NAME, MPI_CHAR, i, TAG_NOME,
                 MPI_COMM_WORLD, &stat);
        printf("Maquina %d esta no computador \"%s\"\n", i, NOME(i));
    }

    /* Primeiro, manda uma linha para cada escravo. */
    for (tarefa = 0; tarefa < nb_escravos && tarefa < IMG_HEIGHT; tarefa++) {
        printf("Mestre manda linha %d para a maquina %d (%s)\n",
               tarefa, tarefa + 1, NOME(tarefa + 1));
        MPI_Send(&tarefa, 1, MPI_INT, tarefa + 1, TAG_TAREFA, MPI_COMM_WORLD);
    }

    /* Depois, a cada resultado recebido, guarda a linha na imagem e manda
       a proxima linha para quem acabou de terminar. */
    while (tarefa < IMG_HEIGHT) {
        MPI_Recv(&resultado, sizeof(LinhaResultado), MPI_BYTE, MPI_ANY_SOURCE,
                 TAG_RESULTADO, MPI_COMM_WORLD, &stat);
        memcpy(&imagem[(size_t) resultado.linha * IMG_WIDTH * 3],
               resultado.pixels, IMG_WIDTH * 3);
        printf("Mestre recebeu linha %d da maquina %d (%s); manda linha %d\n",
               resultado.linha, stat.MPI_SOURCE, NOME(stat.MPI_SOURCE), tarefa);
        MPI_Send(&tarefa, 1, MPI_INT, stat.MPI_SOURCE, TAG_TAREFA, MPI_COMM_WORLD);
        tarefa++;
    }

    /* Sobra um resultado pendente por escravo; depois dele, manda o fim. */
    for (i = 0; i < nb_escravos; i++) {
        int sinal = END;
        MPI_Recv(&resultado, sizeof(LinhaResultado), MPI_BYTE, MPI_ANY_SOURCE,
                 TAG_RESULTADO, MPI_COMM_WORLD, &stat);
        memcpy(&imagem[(size_t) resultado.linha * IMG_WIDTH * 3],
               resultado.pixels, IMG_WIDTH * 3);
        printf("Mestre recebeu linha %d da maquina %d (%s) (ultima); mandando fim\n",
               resultado.linha, stat.MPI_SOURCE, NOME(stat.MPI_SOURCE));
        MPI_Send(&sinal, 1, MPI_INT, stat.MPI_SOURCE, TAG_TAREFA, MPI_COMM_WORLD);
    }

    salvar_ppm("saida.ppm", imagem, IMG_WIDTH, IMG_HEIGHT);
    free(imagem);
    free(nomes);
}

static void escravo(void)
{
    int tarefa = 0;
    int rank, tam_nome;
    char nome[MPI_MAX_PROCESSOR_NAME];
    LinhaResultado resultado;
    MPI_Status stat;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* Usa o apelido da maquina (ex.: "diego") se o rodar.sh passou um;
       senao, usa o nome do computador. */
    memset(nome, 0, sizeof(nome));
    if (getenv("APELIDO") != NULL && getenv("APELIDO")[0] != '\0') {
        strncpy(nome, getenv("APELIDO"), sizeof(nome) - 1);
    } else {
        MPI_Get_processor_name(nome, &tam_nome);
    }
    MPI_Send(nome, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, TAG_NOME, MPI_COMM_WORLD);

    /* Recebe uma linha, calcula, devolve; repete ate receber END. */
    while (1) {
        MPI_Recv(&tarefa, 1, MPI_INT, 0, TAG_TAREFA, MPI_COMM_WORLD, &stat);
        if (tarefa == END) {
            break;
        }
        printf("Maquina %d (%s): calculando linha %d\n", rank, nome, tarefa);
        calcula_linha(tarefa, resultado.pixels);
        resultado.linha = tarefa;
        MPI_Send(&resultado, sizeof(LinhaResultado), MPI_BYTE, 0, TAG_RESULTADO, MPI_COMM_WORLD);
    }
    printf("Maquina %d (%s) terminando...\n", rank, nome);
}

int main(int argc, char *argv[])
{
    int rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        printf("Processo 0 eh o mestre.\n");
        mestre();
    } else {
        printf("Processo %d eh uma maquina escrava.\n", rank);
        escravo();
    }

    MPI_Finalize();
    return 0;
}
