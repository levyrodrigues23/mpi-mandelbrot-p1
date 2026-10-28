#include "mandelbrot_calc.h"
#include <math.h>

/*
 * calc_point: porta de Mandel::calcPoint (Mandel.h).
 *
 * O original usa std::complex<double> e a funcao generica sqr(). Aqui a
 * mesma iteracao z = z^2 + c e feita manualmente com duas variaveis
 * double (parte real/imaginaria), sem precisar de nenhuma biblioteca de
 * numeros complexos.
 */
int calc_point(double cr, double ci, int maxiter)
{
    double zr = 0.0, zi = 0.0;
    double novo_zr, novo_zi;
    double m2;
    int iter = 0;

    do {
        novo_zr = zr * zr - zi * zi + cr;
        novo_zi = 2.0 * zr * zi + ci;
        zr = novo_zr;
        zi = novo_zi;
        m2 = zr * zr + zi * zi;
        iter++;
    } while (m2 < 4.0 && iter < maxiter);

    return iter;
}

/*
 * get_color: porta de RGBPalette::getColor (Palette.cpp) + da conversao
 * de float (0..1) para byte (0..255) que o X11Display::color ja fazia.
 *
 * Parametros da paleta identicos aos usados em mandelbrot.cpp:
 *   RGBPalette p(0.4, 0.95, 0.65, 0.45, 0.5, 0.5, 0.5)
 * ou seja: exp=0.4, freq=(0.95,0.65,0.45), fase=(0.5,0.5,0.5) nos 3
 * canais (R,G,B).
 */
#define PALETTE_EXP     0.4
#define PALETTE_FREQ_R  0.95
#define PALETTE_FREQ_G  0.65
#define PALETTE_FREQ_B  0.45
#define PALETTE_FASE_R  0.5
#define PALETTE_FASE_G  0.5
#define PALETTE_FASE_B  0.5

static unsigned char componente_para_byte(double v)
{
    if (v < 0.0) v = 0.0;
    if (v > 1.0) v = 1.0;
    return (unsigned char)(v * 255.0);
}

void get_color(int iter, int maxiter, unsigned char *r, unsigned char *g, unsigned char *b)
{
    double n = (double) iter / (double) maxiter;
    double ne = pow(n, PALETTE_EXP);

    double fr = pow(sin(ne * 5 * 3.14 * PALETTE_FREQ_R + PALETTE_FASE_R * 10 * 3.14 / 2), 2);
    double fg = pow(sin(ne * 5 * 3.14 * PALETTE_FREQ_G + PALETTE_FASE_G * 10 * 3.14 / 2), 2);
    double fb = pow(sin(ne * 5 * 3.14 * PALETTE_FREQ_B + PALETTE_FASE_B * 10 * 3.14 / 2), 2);

    *r = componente_para_byte(fr);
    *g = componente_para_byte(fg);
    *b = componente_para_byte(fb);
}
