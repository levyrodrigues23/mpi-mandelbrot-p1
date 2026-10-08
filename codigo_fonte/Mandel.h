#ifndef _MANDEL_H
#define _MANDEL_H

#include <complex> // classes de números complexos fazem parte da biblioteca padrão C++

#define dcomplex std::complex<double>

/**
 Função genérica que calcula o quadrado de um número complexo.
 */
template <typename Tc> 
Tc sqr(const Tc& a) 
{ 
   return Tc (a.real() * a.real() - a.imag() * a.imag(), 
              2 * a.real() * a.imag()); 
} 

/**
 Classe que encapsula a geração de fractais de Mandelbrot.
 */
class Mandel
{

   public:
   
      /**
        Calcula o valor de um ponto no plano complexo.
        */
      unsigned int calcPoint(dcomplex& c, int maxiter);
      
      /**
        Calcula o fractal de Mandelbrot para todos os pontos da tela.
        @param[in]   d      Tela.
        @param[in]   p      Paleta.
        @param[in]   zmin   Ponto que representa o canto inferior esquerdo do fractal.
        @param[in]   zmax   Ponto que representa o canto superior direito do fractal.
        @param[in]   maxiter   Número máximo de iterações para o cálculo de cada ponto.     
        */        
      template<typename Td, typename Tp>
      void calcAndShow(Td& d, const Tp& p, 
         const dcomplex& zmin, const dcomplex& zmax, int maxiter);
               
   private:
      // Quadrado do raio do plano complexo de Mandelbrot
      static const double Raio2 = 4.0;   
};

/**
 Calcula o valor de um ponto no plano complexo.
 */
unsigned int
Mandel::calcPoint(dcomplex& c, int maxiter)
{
   int iter = 0;
   dcomplex z(0.0, 0.0);
   dcomplex z2; // quadrado de z
   double m2; // quadrado da magnitude de z

   do {
      z = sqr(z) + c;
      m2 = norm(z);
      iter++;
   } while ((m2 < Raio2) && (iter < maxiter));
   return iter;
}


/**
 Calcula o fractal de Mandelbrot para todos os pontos da tela.
 Usou-se templates (polimorfismo estático) para permitir que esta função possa
 ser reutilizada para diferentes classes que representam telas e paletas.
 A vinculação de tipos aos argumentos da função abaixo ocorre no momento da compilação
 (note que o código abaixo precisa ser compilado para cada combinação de argumentos, por
 isso a implementação está num arquivo com extensão ".h").
 */   

template<typename Td, typename Tp>
void
Mandel::calcAndShow(Td& d, const Tp& p, const dcomplex& zmin, const dcomplex& zmax, int maxiter)
{
   unsigned int x, y;
   dcomplex c;
   int iter;
   unsigned int nx = d.width();
   unsigned int ny = d.height();
   
   double dx = (zmax.real() - zmin.real()) / (double) nx;
   double dy = (zmax.imag() - zmin.imag()) / (double) ny;

   for (x = 0; x < nx; x++) 
   {
      for (y = 0; y < ny; y++) 
      {
         // mapeia (x,y) no plano complexo
         c = dcomplex((double) (zmin.real() + ((double) x * dx)), 
                      (double) (zmin.imag() + ((double) y * dy)));
     
         // calcula valor do fractal no ponto c
         iter = calcPoint(c, maxiter);

         // mostra ponto na tela
         d.drawPoint(x, y, p.getColor((float)iter/maxiter));
      }
   }
}



#endif // _MANDEL_H
