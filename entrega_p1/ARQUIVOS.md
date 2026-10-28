# Arquivos da P1 — o que é cada um

Projeto: Mandelbrot distribuído com MPI (Sistemas Distribuídos, UFC Itapajé, Prof. Julio Anjos).
Autores: José Levy Rodrigues da Silva e Diego Silva Nascimento.

## O que está nesta pasta (`entrega_p1/`)

| Arquivo | Para quê |
|---|---|
| `codigo_P1_mandelbrot_mpi.zip` | Zip dos códigos para mandar no SIGAA (só os arquivos do programa). |
| `relatorio_overleaf.zip` | Projeto LaTeX do relatório. No Overleaf: New Project → Upload Project. |
| `Relatorio_P1.pdf` | O relatório já compilado. |
| `Guia_Apresentacao_P1.pdf` | Guia completo: como o programa funciona, os arquivos principais com trechos de código, o passo a passo com comandos (WSL + VM Lubuntu, e como acrescentar máquinas), problemas já resolvidos e perguntas da apresentação. |
| `ROTEIRO_APRESENTACAO.md` | Roteiro do que falar e dos comandos do dia. |
| `ARQUIVOS.md` | Este arquivo. |
| `fontes/` | Versões abertas: `codigo/` (código), `overleaf/` (`.tex` + prints), `apresentacao/` (`.tex` do guia). |

## Código do professor (não foi alterado)

### Mandelbrot serial — `mandelbrot.cpp`, `Mandel.h`, `Palette.cpp/.h`, `Color.h`, `X11Display.cpp/.h`

Programa em C++ que desenha o fractal numa janela X11, num processo só.

- **`mandelbrot.cpp`**: o `main`. Define a imagem de 800×600, o máximo de 20000 iterações
  e a região do plano complexo. A região com zoom já vem ativada:
  `zmin = (-0.482536, -0.635201)` e `zmax = (-0.565928, -0.579588)`.
  Cria a paleta `RGBPalette p(0.4, 0.95, 0.65, 0.45, 0.5, 0.5, 0.5)` e chama `m.calcAndShow(...)`.
- **`Mandel.h`**: o cálculo.
  - `calcPoint(c, maxiter)` repete `z = z² + c` (começando com z = 0) até `|z|² ≥ 4` ou até
    `maxiter`, e devolve quantas iterações gastou.
  - `calcAndShow(...)` percorre todos os pixels, transforma cada (x, y) num ponto `c` do
    plano complexo (`c = zmin + (x·dx, y·dy)`), chama `calcPoint` e pinta o pixel.
- **`Palette.cpp/.h`**: transforma o número de iterações (de 0 a 1) numa cor. Cada canal
  (R, G, B) é uma senoide: `pow(sin(n^exp · 5 · 3.14 · f + p · 10 · 3.14 / 2), 2)`.
- **`Color.h`**: uma cor (r, g, b) com valores de 0 a 1.
- **`X11Display.cpp/.h`**: abre a janela e desenha pontos. É o motivo de o serial precisar de `libx11-dev`.

### Exemplo mestre/escravo — `mestre_escravo_exemplo.c`

Programa em C com MPI. O rank 0 é o mestre e os outros são escravos.

1. O mestre manda uma tarefa (um número) para cada escravo.
2. Num `while`, recebe um resultado de **qualquer** escravo (`MPI_ANY_SOURCE`) e manda a
   próxima tarefa para quem respondeu (`stat.MPI_SOURCE`).
3. No fim, recebe os últimos resultados e manda `END` (-1) para cada escravo parar.

Os escravos ficam num laço: recebem a tarefa, calculam (no exemplo, só devolvem o número) e mandam de volta.

## Código que nós fizemos (vai no zip)

### `mandelbrot_mpi.c` — o programa distribuído

É o exemplo do professor com outra tarefa: **cada tarefa é uma linha da imagem**.

- **`mestre()`**
  1. Recebe o nome do computador de cada escravo (`TAG_NOME`).
  2. Manda a primeira linha para cada escravo.
  3. A cada linha recebida, copia os pixels para o lugar certo da imagem (`memcpy` na
     posição `linha * 800 * 3`) e manda a próxima linha para quem respondeu.
  4. No fim, manda `END` e grava `saida.ppm`.
- **`escravo()`**: manda o nome do computador e depois fica em loop. Recebe o número da
  linha, chama `calcula_linha()` e devolve a linha. Para quando recebe `END`. Se a variável
  `APELIDO` existir, usa ela no lugar do nome do computador (assim aparece "diego" em vez
  de "DESKTOP-AVJFOQA").
- **`calcula_linha(y, buf)`**: faz o mesmo mapeamento pixel → ponto do `calcAndShow` do
  professor, mas só para uma linha `y`.
- **`LinhaResultado`**: `struct` com o número da linha e os 2400 bytes de cor
  (800 pixels × 3). Vai inteira numa mensagem `MPI_BYTE`.
- **`salvar_ppm()`**: grava a imagem no formato PPM (cabeçalho `P6 800 600 255` + bytes RGB),
  sem precisar de biblioteca. Para ver a imagem: `convert saida.ppm saida.png`.
- **Tags**: `TAG_TAREFA` (110) do mestre para o escravo, `TAG_RESULTADO` (111) do escravo
  para o mestre e `TAG_NOME` (112) para a apresentação inicial.

**Por que linha por linha e não 1/3 fixo da imagem por máquina?** No Mandelbrot tem linha
muito mais pesada que outra: os pontos da parte cinza rodam as 20000 iterações. Com divisão
fixa, uma máquina ficaria sobrecarregada e as outras paradas. Distribuindo uma linha por vez,
quem termina antes pega mais.

### `mandelbrot_calc.c` / `mandelbrot_calc.h` — as fórmulas em C

O exemplo de MPI é em C e o serial é C++ com X11. Para as máquinas escravas não precisarem de
interface gráfica, passamos só as duas fórmulas para C:

- **`calc_point(cr, ci, maxiter)`**: igual ao `Mandel::calcPoint`, mas com a parte real e a
  imaginária em dois `double`, porque `(a+bi)² = (a² − b²) + 2abi`.
- **`get_color(iter, maxiter, &r, &g, &b)`**: igual ao `RGBPalette::getColor`, com os mesmos
  parâmetros do `mandelbrot.cpp`. Converte cada canal de 0–1 para 0–255.

### `Makefile`

- `make mpi` gera o `mandelbrot_mpi` (com `mpicc`).
- `make serial` gera o serial do professor (precisa de `libx11-dev`).
- `make clean` apaga o que foi compilado.

### `hosts`

O arquivo de máquinas do `mpirun`: quais máquinas e quantos processos cada uma roda.

```
localhost slots=2    # notebook do José: mestre + 1 escravo
diego slots=1        # notebook do Diego
lab slots=1          # PC do laboratório
```

`diego` e `lab` são apelidos definidos no `~/.ssh/config` do José.

## Como rodar

Tudo é feito com comandos padrão (`apt`, `git`, `mpicc`, `ssh`, `mpirun`), digitados no terminal.
O passo a passo completo está no `Guia_Apresentacao_P1.pdf` e no `README.md` do repositório:
https://github.com/levyrodrigues23/mpi-mandelbrot-p1
