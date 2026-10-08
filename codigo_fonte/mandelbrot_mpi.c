#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "mandelbrot_calc.h"

/*
 * mandelbrot_mpi.c
 *
 * Versao distribuida (mestre/escravo) do gerador do conjunto de
 * Mandelbrot, adaptada diretamente do exemplo mestre_escravo_exemplo.c
 * fornecido pelo professor: em vez de "tarefa = um numero qualquer",
 * cada tarefa e' "calcule a linha Y inteira da imagem". O mestre (rank 0)
 * nao calcula pixels, so distribui linhas, recebe os resultados e monta
 * a figura final; as outras maquinas (ranks 1..N) calculam as linhas.
 *
 * Essa distribuicao e' dinamica (uma linha de cada vez, sob demanda),
 * igual ao exemplo original, porque o custo de cada linha do Mandelbrot
 * varia muito (pontos "dentro" do conjunto gastam o maxiter inteiro,
 * pontos "fora" escapam rapido) -- uma divisao estatica (ex.: 1/3 fixo
 * da imagem para cada maquina) correria o risco de sobrecarregar uma
 * maquina enquanto as outras ficam ociosas.
 */

#define IMG_WIDTH   800
#define IMG_HEIGHT  600
#define MAXITER     20000

/* Mesma regiao do plano complexo (com zoom) usada em mandelbrot.cpp. */
#define ZMIN_R (-0.482536)
#define ZMIN_I (-0.635201)
#define ZMAX_R (-0.565928)
#define ZMAX_I (-0.579588)

#define TAG_TAREFA     110
#define TAG_RESULTADO  111
#define END            -1

/* Uma "tarefa" concluida: o indice da linha + os bytes RGB da linha
   inteira. Tamanho fixo (sabemos IMG_WIDTH em tempo de compilacao), o
   que permite mandar tudo num unico MPI_Send/MPI_Recv de bytes. */
typedef struct {
    int linha;
    unsigned char pixels[IMG_WIDTH * 3];
} LinhaResultado;

/* Calcula uma linha inteira da imagem (mesma conta de Mandel::calcAndShow
   para mapear pixel -> ponto no plano complexo), usando as funcoes
   portadas para C puro em mandelbrot_calc.c. */
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

/* Grava a imagem completa num arquivo PPM (formato P6: cabecalho texto +
   bytes RGB binarios), sem precisar de nenhuma biblioteca externa. */
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

/* Mestre: distribui linhas dinamicamente, recebe os resultados, monta a
   imagem completa e grava o arquivo final. Adaptado diretamente de
   mestre() em mestre_escravo_exemplo.c. */
static void mestre(void)
{
    int tarefa = 0;
    int nb_escravos;
    unsigned char *imagem;
    LinhaResultado resultado;
    MPI_Status stat;

    MPI_Comm_size(MPI_COMM_WORLD, &nb_escravos);
    nb_escravos--;

    imagem = malloc((size_t) IMG_WIDTH * IMG_HEIGHT * 3);
    if (imagem == NULL) {
        fprintf(stderr, "Erro ao alocar buffer da imagem\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    /* Primeiro, manda uma linha para cada maquina escrava. */
    for (tarefa = 0; tarefa < nb_escravos && tarefa < IMG_HEIGHT; tarefa++) {
        printf("Mestre manda linha %d para a maquina %d\n", tarefa, tarefa + 1);
        MPI_Send(&tarefa, 1, MPI_INT, tarefa + 1, TAG_TAREFA, MPI_COMM_WORLD);
    }

    /* Recebe resultados e vai mandando a proxima linha pendente para
       quem terminou (balanceamento dinamico de carga). */
    while (tarefa < IMG_HEIGHT) {
        MPI_Recv(&resultado, sizeof(LinhaResultado), MPI_BYTE, MPI_ANY_SOURCE,
                 TAG_RESULTADO, MPI_COMM_WORLD, &stat);
        memcpy(&imagem[(size_t) resultado.linha * IMG_WIDTH * 3],
               resultado.pixels, IMG_WIDTH * 3);
        printf("Mestre recebeu linha %d da maquina %d; manda linha %d\n",
               resultado.linha, stat.MPI_SOURCE, tarefa);
        MPI_Send(&tarefa, 1, MPI_INT, stat.MPI_SOURCE, TAG_TAREFA, MPI_COMM_WORLD);
        tarefa++;
    }

    /* Ainda faltam receber os resultados que estavam "em andamento". */
    for (tarefa = 0; tarefa < nb_escravos; tarefa++) {
        int sinal = END;
        MPI_Recv(&resultado, sizeof(LinhaResultado), MPI_BYTE, MPI_ANY_SOURCE,
                 TAG_RESULTADO, MPI_COMM_WORLD, &stat);
        memcpy(&imagem[(size_t) resultado.linha * IMG_WIDTH * 3],
               resultado.pixels, IMG_WIDTH * 3);
        printf("Mestre recebeu linha %d da maquina %d (ultima); mandando fim\n",
               resultado.linha, stat.MPI_SOURCE);
        MPI_Send(&sinal, 1, MPI_INT, stat.MPI_SOURCE, TAG_TAREFA, MPI_COMM_WORLD);
    }

    salvar_ppm("saida.ppm", imagem, IMG_WIDTH, IMG_HEIGHT);
    free(imagem);
}

/* Escravo (uma das "maquinas"): recebe o indice de uma linha, calcula, e
   manda o resultado de volta; repete ate receber o sinal de fim (END).
   Adaptado diretamente de escravo() em mestre_escravo_exemplo.c. */
static void escravo(void)
{
    int tarefa = 0;
    int rank;
    LinhaResultado resultado;
    MPI_Status stat;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    while (1) {
        MPI_Recv(&tarefa, 1, MPI_INT, 0, TAG_TAREFA, MPI_COMM_WORLD, &stat);
        if (tarefa == END) {
            break;
        }
        printf("Maquina %d: calculando linha %d\n", rank, tarefa);
        calcula_linha(tarefa, resultado.pixels);
        resultado.linha = tarefa;
        MPI_Send(&resultado, sizeof(LinhaResultado), MPI_BYTE, 0, TAG_RESULTADO, MPI_COMM_WORLD);
    }
    printf("Maquina %d terminando...\n", rank);
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
