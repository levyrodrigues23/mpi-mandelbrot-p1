# P1 — Mandelbrot distribuído com MPI

Disciplina Sistemas Distribuídos (UFC, Campus Itapajé, Prof. Dr. Julio Anjos).
Autores: José Levy Rodrigues da Silva e Diego Silva Nascimento.

Gera o conjunto de Mandelbrot dividindo o cálculo entre **3 máquinas**
(uma linha da imagem por tarefa, distribuída dinamicamente pelo mestre),
seguindo o padrão mestre/escravo fornecido pelo professor
(`codigo_fonte/mestre_escravo_exemplo.c`). O mestre recebe as linhas
calculadas, monta a imagem completa e grava `saida.ppm`.

## Arquivos

- `codigo_fonte/mandelbrot.cpp` + `Mandel.h`/`Palette.h,.cpp`/`Color.h`/`X11Display.h,.cpp`:
  programa **serial original** (C++, janela X11), fornecido pelo
  professor, intocado — baseline de comparação no relatório.
- `codigo_fonte/mandelbrot_calc.h,.c`: as mesmas fórmulas do fractal e da
  cor, portadas para **C puro** (sem `std::complex`/classes).
- `codigo_fonte/mandelbrot_mpi.c`: o programa **distribuído** (C puro,
  MPI), a entrega da P1.
- `codigo_fonte/mestre_escravo_exemplo.c`: exemplo de referência do
  professor (mantido como veio).
- `setup_maquina.sh`: instala as dependências (`openmpi-bin`,
  `libopenmpi-dev`, `openssh-server`) em cada uma das 3 máquinas.
- `hosts`: hostfile do MPI (IPs das 3 máquinas na rede local).

## Compilar

Em cada uma das 3 máquinas, dentro de `codigo_fonte/`:

```sh
make mpi        # gera o executável mandelbrot_mpi (versão distribuída)
make serial     # gera o executável mandelbrot (versão serial, com X11)
```

## Rodar (3 máquinas em rede)

1. Rode `../setup_maquina.sh` nas 3 máquinas (precisa de `sudo`).
2. Garanta que as 3 estão na mesma rede Wi-Fi/LAN. No WSL (seu notebook e
   o do Diego), ative o modo de rede "mirrored" (`%UserProfile%\.wslconfig`
   com `networkingMode=mirrored` + `wsl --shutdown`).
3. Configure SSH sem senha do seu notebook (mestre) para as outras duas
   máquinas (chave pública em `~/.ssh/authorized_keys`).
4. Edite `hosts` com os IPs reais das 3 máquinas (`hostname -I` em cada
   uma).
5. A partir do seu notebook:
   ```sh
   cd codigo_fonte
   mpirun --hostfile ../hosts -np 4 ./mandelbrot_mpi
   ```
6. A imagem final fica em `codigo_fonte/saida.ppm`. Para visualizar/anexar
   no relatório, converta para PNG com `convert saida.ppm saida.png`
   (ImageMagick) ou, se não tiver o ImageMagick instalado,
   `python3 ppm2png.py saida.ppm saida.png` (sem dependências, já incluso
   no repositório).

## Testar localmente (sem rede, 1 máquina só)

```sh
cd codigo_fonte
make mpi
mpirun -np 4 ./mandelbrot_mpi
```
