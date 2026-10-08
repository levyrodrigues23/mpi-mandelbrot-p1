#include "X11Display.h"
#include "Palette.h"
#include "Mandel.h"
#include <iostream>
#include <string>
#include <string>

using namespace std;

// valor default para a largura da tela
const unsigned int DefaultWidth = 800;

// valor default para a altura da tela
const unsigned int DefaultHeight = 600;

// valor default (ponto no plano complexo) para o canto inferior esquerdo da figura a ser gerada 
const dcomplex DefaultZMin = dcomplex(-1.5545173217428, -1.55451670625497);

// valor default (ponto no plano complexo) para o canto superior direito da figura a ser gerada  
const dcomplex DefaultZMax = dcomplex( 0.29958615131552137, 0.39958615593168024); 


// valor default para o numero maximo de iteracoes para o calculo de cada ponto da figura
const int DefaultMaxIter = 20000;

                                         
int 
main(int argc, char *argv[])
{
   string command;
   
   unsigned int largura = DefaultWidth;
   unsigned int altura = DefaultHeight;
   
   int maxiter = DefaultMaxIter;
   
   // Por default, zmin e zmax estão configurados para permitir a visualização
   // da figura de Mandelbrot "por inteiro" (sem zoom).
   dcomplex zmin = DefaultZMin;
   dcomplex zmax = DefaultZMax;

   // Para ver um exemplo de figura com zoom, descomente as linhas abaixo.   
   zmin = dcomplex(-0.482536, -0.635201);
   zmax = dcomplex(-0.565928, -0.579588);
   
   X11Display d;
   d.open(largura, altura, "Mandelbrot");
   
   Mandel m;

   RGBPalette p(0.4, 0.95, 0.65, 0.45, 0.5, 0.5, 0.5);
   //GrayPalette p(0.4, 0.9, 0.5);
     
   m.calcAndShow(d, p, zmin, zmax, maxiter); 
   
   d.flush();
   
   std::cout << "Digite fim para encerrar o programa" << endl;
   while (std::cin >> command)
   {
      if (command == "fim")
         break;
   };
   
   d.close();
   
   return 0;

}
