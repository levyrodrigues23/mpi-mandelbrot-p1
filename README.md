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
- `hosts`: hostfile do MPI (quais máquinas participam e quantos processos
  cada uma roda).

## Compilar

Dentro de `codigo_fonte/`:

```sh
make mpi        # gera o executável mandelbrot_mpi (versão distribuída)
make serial     # gera o executável mandelbrot (versão serial, com X11;
                # precisa de: sudo apt-get install libx11-dev)
```

## Testar em uma máquina só (sem rede)

Simula o mestre e as 3 máquinas escravas como 4 processos no mesmo
computador. Serve para conferir se o programa está funcionando.

```sh
cd codigo_fonte
make mpi
mpirun -np 4 ./mandelbrot_mpi
```

A imagem é gravada em `codigo_fonte/saida.ppm`. Para ver:

```sh
sudo apt-get install -y imagemagick   # só na primeira vez
convert saida.ppm saida.png
explorer.exe saida.png                # abre no Windows (WSL)
```

## Rodar nas 3 máquinas

Máquinas: notebook do José (mestre, WSL), notebook do Diego (WSL) e PC do
laboratório (Ubuntu). As 3 precisam estar **na mesma rede Wi-Fi/LAN**.

O notebook do José roda 2 processos (o mestre e uma máquina escrava); o do
Diego e o do lab rodam 1 escravo cada — 4 processos no total.

### Passo 1 — Nas 3 máquinas

Clonar o repositório **dentro da pasta pessoal** (`~`), para o caminho ser
o mesmo em todas, instalar as dependências e compilar:

```sh
cd ~
git clone https://github.com/levyrodrigues23/mpi-mandelbrot-p1.git
cd mpi-mandelbrot-p1
sh setup_maquina.sh        # pede a senha do sudo
cd codigo_fonte
make mpi
hostname -I                # anote o IP desta máquina
whoami                     # anote o nome de usuário desta máquina
```

### Passo 2 — Nos dois notebooks com WSL (José e Diego)

Por padrão o WSL não é visível para outros computadores da rede. Para
resolver:

1. No Windows, criar o arquivo `C:\Users\<seu_usuario>\.wslconfig` com:
   ```
   [wsl2]
   networkingMode=mirrored
   ```
2. No PowerShell **como administrador**, liberar conexões de entrada no
   WSL:
   ```powershell
   Set-NetFirewallHyperVVMSetting -Name '{40E0AC32-46A5-438A-A0B2-2B479E8F2E90}' -DefaultInboundAction Allow
   ```
3. Reiniciar o WSL: `wsl --shutdown` no PowerShell e abrir o Ubuntu de
   novo.
4. Conferir no Ubuntu que `hostname -I` mostra o mesmo IP do Windows na
   Wi-Fi (`ipconfig` no PowerShell).
5. Garantir que o SSH está rodando: `sudo service ssh start`.

### Passo 3 — No notebook do José: SSH sem senha

O `mpirun` usa SSH para iniciar os processos nas outras máquinas, então o
notebook do José precisa entrar nelas sem senha.

```sh
ssh-keygen                                   # Enter em tudo (só na primeira vez)
ssh-copy-id USUARIO_DO_DIEGO@IP_DO_DIEGO     # pede a senha do Diego uma vez
ssh-copy-id USUARIO_DO_LAB@IP_DO_LAB         # pede a senha do lab uma vez
```

Como os nomes de usuário são diferentes em cada máquina, criar o arquivo
`~/.ssh/config` com apelidos (trocar pelos IPs e usuários anotados no
passo 1):

```
Host diego
    HostName IP_DO_DIEGO
    User USUARIO_DO_DIEGO

Host lab
    HostName IP_DO_LAB
    User USUARIO_DO_LAB
```

Testar — os dois precisam responder `ok` **sem pedir senha**:

```sh
ssh diego echo ok
ssh lab echo ok
```

### Passo 4 — No notebook do José: executar

O arquivo `hosts` (na raiz do repositório) já usa os apelidos do passo 3:

```
localhost slots=2
diego slots=1
lab slots=1
```

Executar **a partir da pasta pessoal**:

```sh
cd ~
mpirun --hostfile mpi-mandelbrot-p1/hosts -np 4 mpi-mandelbrot-p1/codigo_fonte/mandelbrot_mpi
```

O terminal mostra o mestre distribuindo linhas ("Mestre manda linha X
para a maquina Y") e as máquinas calculando. No fim, a imagem fica em
`~/saida.ppm` no notebook do José.

### Se travar ou der erro

- **Pede senha ao rodar o `mpirun`**: o passo 3 não funcionou; refazer o
  `ssh-copy-id` e testar `ssh diego echo ok` / `ssh lab echo ok`.
- **Trava sem terminar**: geralmente é firewall bloqueando a conexão
  (Windows ou rede da UFC). Conferir o passo 2 nos dois notebooks.
- **"No such file or directory" no executável**: o repositório não foi
  clonado em `~/mpi-mandelbrot-p1` em alguma das máquinas, ou o
  `make mpi` não foi rodado nela.
