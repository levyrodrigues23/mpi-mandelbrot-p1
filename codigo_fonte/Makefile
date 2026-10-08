CXX=g++
CXXFLAGS=-Wall -O2 -I.
X11PATH=/usr/X11R6
LDFLAGS=-L$(X11PATH)/lib -lX11

CC_MPI=mpicc
CFLAGS_MPI=-Wall -O2

SERIAL_TARGET=mandelbrot
SERIAL_OBJS=X11Display.o Palette.o mandelbrot.o

MPI_TARGET=mandelbrot_mpi
MPI_OBJS=mandelbrot_mpi.o mandelbrot_calc.o

.PHONY: all serial mpi clean

all: serial mpi

# Versao serial original (C++, com janela X11) -- baseline de comparacao,
# codigo intocado em relacao ao que o professor forneceu.
serial: $(SERIAL_TARGET)

$(SERIAL_TARGET): $(SERIAL_OBJS)
	$(CXX) $(CXXFLAGS) $(SERIAL_OBJS) -o $@ $(LDFLAGS)

# Versao distribuida (C puro, MPI, sem X11) -- a entrega da P1.
mpi: $(MPI_TARGET)

$(MPI_TARGET): $(MPI_OBJS)
	$(CC_MPI) $(CFLAGS_MPI) $(MPI_OBJS) -lm -o $@

mandelbrot_mpi.o: mandelbrot_mpi.c mandelbrot_calc.h
	$(CC_MPI) $(CFLAGS_MPI) -c mandelbrot_mpi.c

mandelbrot_calc.o: mandelbrot_calc.c mandelbrot_calc.h
	$(CC_MPI) $(CFLAGS_MPI) -c mandelbrot_calc.c

clean:
	rm -f $(SERIAL_OBJS) $(SERIAL_TARGET) $(MPI_OBJS) $(MPI_TARGET) saida.ppm saida.png
