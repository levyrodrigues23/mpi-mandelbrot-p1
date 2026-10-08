#ifndef MANDELBROT_CALC_H
#define MANDELBROT_CALC_H

/*
 * Porta em C puro (sem std::complex/classes) das duas formulas do
 * codigo-base em C++ (Mandel.h e Palette.cpp), usadas pelo programa MPI
 * (mandelbrot_mpi.c), que precisa ser C puro (compilado com mpicc) para
 * seguir o mesmo estilo do exemplo mestre/escravo do professor.
 */

/* Porta de Mandel::calcPoint: numero de iteracoes ate o ponto (cr,ci)
   escapar (|z|^2 >= 4) ou atingir maxiter. */
int calc_point(double cr, double ci, int maxiter);

/* Porta de RGBPalette::getColor + X11Display::color: converte o numero
   de iteracoes (0..maxiter) na cor RGB (0..255 cada componente). */
void get_color(int iter, int maxiter, unsigned char *r, unsigned char *g, unsigned char *b);

#endif /* MANDELBROT_CALC_H */
