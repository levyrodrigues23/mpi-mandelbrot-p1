# P1 — Mandelbrot distribuído com MPI

Sistemas Distribuídos (UFC, Campus Itapajé, Prof. Dr. Julio Anjos).
Autores: José Levy Rodrigues da Silva e Diego Silva Nascimento.

Gera o conjunto de Mandelbrot dividindo o cálculo entre várias máquinas no
padrão **mestre/escravo** do professor: o mestre (rank 0) manda uma linha da
imagem de cada vez para quem estiver livre, as outras máquinas calculam e
devolvem, e o mestre monta a imagem completa em `saida.ppm`.

> **Para rodar em 2 ou 3 máquinas, siga o [GUIA_PASSO_A_PASSO.md](GUIA_PASSO_A_PASSO.md)**
> (também em PDF).

## Teste rápido (1 máquina, sem rede)

```sh
cd ~
git clone https://github.com/levyrodrigues23/mpi-mandelbrot-p1.git
cd mpi-mandelbrot-p1
bash setup_maquina.sh     # instala tudo e compila (pede a senha do sudo)
# feche e abra o terminal
./rodar.sh --local        # 4 processos nesta máquina; demora ~5 s
```

Tem que terminar com `OK! Imagem do mestre: .../saida.ppm`.
Para ver a imagem: `convert codigo_fonte/saida.ppm saida.png` (pacote
`imagemagick`) e `explorer.exe saida.png` no WSL.

## Arquivos

| Arquivo | Para quê |
|---|---|
| `rodar.sh` | **Roda tudo** (`--local` ou nas máquinas do `hosts`). Só no mestre. |
| `verificar.sh` | Diz o que está errado (MPI, SSH, programa) em cada máquina. |
| `setup_maquina.sh` | Prepara uma máquina: instala, compila, aplica o conserto do hwloc. |
| `hosts` | Máquinas e quantos processos cada uma roda. |
| `codigo_fonte/mandelbrot_mpi.c` | Programa distribuído (C, MPI) — a entrega da P1. |
| `codigo_fonte/mandelbrot_calc.c,.h` | Fórmulas do fractal e da cor em C puro. |
| `codigo_fonte/mandelbrot.cpp` + `Mandel.h`, `Palette.*`, `Color.h`, `X11Display.*` | Versão **serial** original do professor (baseline, intocada). `make serial` precisa de `libx11-dev`. |
| `codigo_fonte/mestre_escravo_exemplo.c` | Exemplo de referência do professor. |

## Por que o `mpirun` travava (e como está resolvido)

No WSL o `mpirun` ficava **parado para sempre**, até num teste local: o
hwloc (que o OpenMPI usa para descobrir o hardware) tenta conectar em um
servidor gráfico X11 (`127.0.0.1:6001`) e essa conexão nunca responde. A
solução é `HWLOC_COMPONENTS=-gl`:

- o `rodar.sh` já define isso;
- o `setup_maquina.sh` grava a linha no começo do `~/.bashrc`, para valer
  também nas outras máquinas quando o mestre entra nelas por SSH.

Outros cuidados que o `rodar.sh` já faz: descobre a rede atual (o IP muda a
cada Wi-Fi) e manda o MPI usar só ela; o SSH nunca fica esperando senha; e o
programa é achado em `~/mpi-mandelbrot-p1` em cada máquina, mesmo com
nomes de usuário diferentes.
