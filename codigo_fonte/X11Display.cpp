#include "X11Display.h"
#include <cstdlib> // a função abs é declarada neste cabeçalho

// Construtor da tela.
void X11Display::open(unsigned int width, unsigned int height, char *name)
{
   createDisplay();
   initColors();
   initWindow(width, height, name);
   initGC();

   displayWindow();
   flush();
}

void X11Display::close()
{
   XCloseDisplay(display);
}

// Inicializa display, estabelecendo conexão com servidor X.
void X11Display::createDisplay()
{
   // Estabelece conexão com servidor X.
   // O argumento indica o nome do display (se for NULL, 
   // utiliza-se o valor da variável de ambiente DISPLAY).
   display = XOpenDisplay(NULL);
       
   // Obtém o número do screen default deste display.  
   screen = DefaultScreen(display);
}

// Calcula o número máximo de cores que podem ser mostradas na tela.
void X11Display::initColors()
{
   unsigned long white = WhitePixel(display, screen);
   unsigned long black = BlackPixel(display, screen);

   _colors = abs(black - white);
}

// Inicializa janela, definindo seus atributos.
void X11Display::initWindow(int width, int height, char *name)
{
   unsigned long mask = 0L;
   XSetWindowAttributes attr;
    
   _width = width;
   _height = height;

   // Define atributos da janela a ser criada.
   attr.background_pixel = WhitePixel(display, screen);
   attr.backing_store = Always;
   attr.save_under = True;
   mask = CWBackPixel | CWBackingStore | CWSaveUnder;

   // Cria janela.    
   window = XCreateWindow(display, RootWindow(display, screen), 
                          0, 0,   // coordenadas do canto superior esquerdo
                          width, height,   // dimensões internas da janela 
                          0,   // número de pixels da borda da janela
                          0,   // profundidade da janela
                          InputOutput,   // classe da janela
                          (Visual *) CopyFromParent,   // tipo de visual da janela
                          mask,   // máscara que indica os atributos definidos em attr
                          &attr); // conjunto de atributos da janela

   // Define nome para a janela.
   XStoreName(display, window, name);
}

// Inicializa o contexto gráfico.
void X11Display::initGC()
{
   gc = XCreateGC(display, window, 0, NULL);
   XSetBackground(display, gc, WhitePixel(display, screen));
   XSetForeground(display, gc, BlackPixel(display, screen));
   XSetLineAttributes(display, gc, 1, LineSolid, CapRound, JoinRound);
}

// Mostra janela.
void X11Display::displayWindow()
{
   // Mapeia a janela no display para torná-la visível.
   XMapWindow(display, window);
}

// Atualiza tela.
void X11Display::flush()
{
   XFlush(display);
}

// Mostra na tela o ponto de coordenadas (x,y), pintando-o com color.
void X11Display::drawPoint(int x, int y, unsigned long color)
{
   XSetForeground(display, gc, color);
   XDrawPoint(display, window, gc, x, y);
}

// Mostra na tela o ponto de coordenadas (x,y), pintando-o com color (RGB).
void X11Display::drawPoint(int x, int y, Color color)
{
   unsigned long c = X11Display::color(color.r(), color.g(), color.b());
   XSetForeground(display, gc, c);
   XDrawPoint(display, window, gc, x, y);
}

// Número máximo de cores na tela.
unsigned long X11Display::colors()
{
   return _colors;
}

// Largura da tela.
unsigned int X11Display::width()
{
   return _width;
}

// Altura da tela.
unsigned int X11Display::height()
{
   return _height;
}

#define AJEITA(x) (x < 0 ? 0 : (x > 1 ? 1 : x))
#define COMP(x, X) ((int)(AJEITA(x) * ((1<<X)-1)))
#define COR(r, g, b, R, G, B) ((COMP(r,R)<<(G+B)) | (COMP(g,G)<<B) | COMP(b,B))

// Calcula cor a partir de componentes vermelho, verde e azul
// (r, g e b devem estar entre 0 e 1).
unsigned long X11Display::color(float r, float g, float b)
{
    return COR(r, g, b, B_R, B_G, B_B);
}




