# P1 — Mandelbrot distribuído com MPI

Sistemas Distribuídos (UFC, Campus Itapajé, Prof. Dr. Julio Anjos).
Autores: José Levy Rodrigues da Silva e Diego Silva Nascimento.

O programa gera o conjunto de Mandelbrot dividindo o cálculo entre várias
máquinas, no modelo mestre/escravo do exemplo do professor
(`codigo_fonte/mestre_escravo_exemplo.c`). O mestre manda uma linha da
imagem por vez para cada escravo, recebe as linhas calculadas, monta a
imagem completa e grava `saida.ppm`.

O guia completo (como funciona, os arquivos principais com trechos de
código, o passo a passo e os problemas já resolvidos) está em
[`GUIA_PASSO_A_PASSO.pdf`](GUIA_PASSO_A_PASSO.pdf).

## Arquivos

| Arquivo | O que tem |
|---|---|
| `codigo_fonte/mandelbrot_mpi.c` | O programa distribuído (mestre e escravo). É a entrega principal. |
| `codigo_fonte/mandelbrot_calc.c/.h` | As fórmulas do fractal e da cor, do código do professor, em C. |
| `codigo_fonte/Makefile` | `make mpi` compila a versão MPI; `make serial` compila a original. |
| `hosts` | Máquinas que participam e quantos processos cada uma roda. |
| `codigo_fonte/mandelbrot.cpp`, `Mandel.h`, `Palette.*`, `Color.h`, `X11Display.*` | Mandelbrot serial do professor (sem alteração). |
| `codigo_fonte/mestre_escravo_exemplo.c` | Exemplo mestre/escravo do professor (sem alteração). |
| `entrega_p1/` | O que foi entregue: zip do código, guia em PDF e documentos da apresentação. |
| `prints/` | Capturas de tela das execuções. |

Tudo é feito com comandos padrão, digitados no terminal: `apt`, `git`,
`mpicc`, `ssh` e `mpirun`.

## Em cada máquina: instalar e compilar

O projeto tem que ficar na pasta pessoal (`~/mpi-mandelbrot-p1`), para o
programa estar no mesmo caminho em todas as máquinas.

```sh
sudo apt update
sudo apt install -y git build-essential openmpi-bin libopenmpi-dev openssh-server
sudo service ssh start            # Linux nativo/VM: sudo systemctl enable --now ssh
cd ~
git clone https://github.com/levyrodrigues23/mpi-mandelbrot-p1.git
cd ~/mpi-mandelbrot-p1/codigo_fonte
mpicc -O2 -o mandelbrot_mpi mandelbrot_mpi.c mandelbrot_calc.c -lm
which mpirun                      # tem que ser /usr/bin/mpirun em TODAS as máquinas
hostname -I                       # IP desta máquina
whoami                            # usuário desta máquina
```

Teste numa máquina só (1 mestre + 3 escravos):

```sh
cd ~/mpi-mandelbrot-p1/codigo_fonte
mpirun --oversubscribe -np 4 ./mandelbrot_mpi
```

## Só no WSL

Sem isto o `mpirun` trava no WSL. A linha tem que ser a primeira do
`~/.bashrc`, para valer também nas sessões SSH:

```sh
grep HWLOC ~/.bashrc || sed -i '1i export HWLOC_COMPONENTS=-gl' ~/.bashrc
```

Para o WSL ficar visível na rede, crie `C:\Users\SEU_USUARIO\.wslconfig`:

```
[wsl2]
networkingMode=mirrored
```

E, no PowerShell como administrador:

```powershell
Set-NetFirewallHyperVVMSetting -Name '{40E0AC32-46A5-438A-A0B2-2B479E8F2E90}' -DefaultInboundAction Allow
wsl --shutdown
```

## Máquina virtual (VirtualBox)

Instale os pacotes com a rede da VM em **NAT**. Depois, desligue a VM e
mude a rede para **Placa em modo Bridge**, escolhendo a placa do Windows
que está conectada (`Get-NetAdapter` no PowerShell: a que está com status
*Up*). Na VM, `ip -4 addr` tem que mostrar um IP da mesma faixa do Windows.

## No mestre: SSH sem senha

O `mpirun` abre os processos nas outras máquinas por SSH.

```sh
ls ~/.ssh/id_*.pub || ssh-keygen
ssh-copy-id USUARIO@IP_DA_MAQUINA         # pede a senha uma vez
ssh USUARIO@IP_DA_MAQUINA hostname        # tem que responder sem pedir senha
```

## No mestre: arquivo de máquinas e execução

Exemplo com o WSL (mestre) e uma VM Lubuntu (`~/hosts_vm`):

```
localhost slots=2
joselrds@192.168.0.5 slots=2
```

Sempre a partir da pasta pessoal. `192.168.0.0/24` é a faixa da rede (os três
primeiros números dos IPs + `.0/24`):

```sh
cd ~
mpirun --hostfile hosts_vm -np 4 \
  --mca btl_tcp_if_include 192.168.0.0/24 \
  --mca oob_tcp_if_include 192.168.0.0/24 \
  hostname                                     # mostra onde cada processo abriu

mpirun --hostfile hosts_vm -np 4 \
  --mca btl_tcp_if_include 192.168.0.0/24 \
  --mca oob_tcp_if_include 192.168.0.0/24 \
  mpi-mandelbrot-p1/codigo_fonte/mandelbrot_mpi

convert saida.ppm saida.png && explorer.exe saida.png
```

O `-np` é a soma dos `slots`. Para acrescentar uma máquina, basta uma linha
nova no arquivo de máquinas.

## Problemas comuns

| O que aparece | Solução |
|---|---|
| `Data unpack had inadequate space` | As máquinas usam instalações diferentes do Open MPI. `which mpirun` tem que dar `/usr/bin/mpirun` em todas; tire do `~/.bashrc` qualquer Open MPI compilado à mão, abra um terminal novo e recompile. |
| `mpirun` parado sem mostrar nada | Falta o `HWLOC_COMPONENTS=-gl` no WSL ou as opções `--mca ..._if_include`. |
| `could not access or execute an executable` | O programa não está em `~/mpi-mandelbrot-p1/codigo_fonte` na outra máquina. |
| `unable to open the hostfile` | Rode o `mpirun` na pasta onde está o arquivo de máquinas (`cd ~`). |
| VM sem IP (só `127.0.0.1`) | Bridge na placa errada: use a que está *Up* no `Get-NetAdapter`. |
