# Roteiro da apresentação — P1 (dia 26)

Tempo total: uns 7 minutos (5 de fala + 2 rodando ao vivo).

## Antes da aula (em casa, no dia anterior)

Nas **3 máquinas** (José, Diego e lab), o código fica em `~/mpi-mandelbrot-p1`:

```sh
cd ~
git clone https://github.com/levyrodrigues23/mpi-mandelbrot-p1.git   # quem já tem: cd ~/mpi-mandelbrot-p1 && git pull
sudo apt-get install -y build-essential openmpi-bin libopenmpi-dev openssh-server
make -C ~/mpi-mandelbrot-p1/codigo_fonte mpi
```

Coloque esta linha como a **primeira** do `~/.bashrc` em cada máquina (sem ela, o `mpirun` trava no WSL):

```sh
export HWLOC_COMPONENTS=-gl
```

### Confira se o Open MPI é o mesmo nas 3 máquinas

Este é o erro mais traiçoeiro. Se as máquinas usarem builds diferentes do Open MPI, o `mpirun`
falha logo no início com `Data unpack had inadequate space`, **mesmo sendo todas 4.1.6**.

```sh
which mpirun        # tem que dar o MESMO tipo de caminho nas 3
mpirun --version
```

- O notebook do José hoje usa um Open MPI compilado à mão (`/home/josel/ompi-igual/bin/mpirun`,
  ativado nas duas primeiras linhas do `~/.bashrc`).
- O PC do lab, com `apt`, vai ter `/usr/bin/mpirun`.

Os dois não conversam. **O jeito mais simples é usar o do `apt` em todas:** apague (ou comente
com `#`) as linhas do `ompi-igual` no `~/.bashrc` do José, e do Diego se ele também tiver,
reabra o terminal e recompile:

```sh
make -C ~/mpi-mandelbrot-p1/codigo_fonte clean
make -C ~/mpi-mandelbrot-p1/codigo_fonte mpi
```

Faça isso **antes do dia** e teste com o Diego. Se der certo, está resolvido também para o lab.

## No dia — preparação (uns 5 minutos antes)

**1. As 3 máquinas na mesma rede.** Se a rede da UFC não deixar uma ver a outra, use o hotspot
de um celular. Teste com `ping IP_DA_OUTRA`.

**2. No Diego e no lab:**

```sh
sudo service ssh start        # no lab (Linux normal): sudo systemctl start ssh
hostname -I                   # passe o IP para o José
whoami                        # passe o usuário para o José
```

**3. No José (mestre):**

```sh
sudo service ssh start
nano ~/.ssh/config            # atualize os IPs de hoje
```

```
Host diego
    HostName IP_DO_DIEGO
    User USUARIO_DO_DIEGO

Host lab
    HostName IP_DO_LAB
    User USUARIO_DO_LAB
```

Se ainda não copiou a chave para o lab: `ssh-copy-id lab`, que pede a senha uma vez.

Teste. As duas máquinas têm que responder `ok` **sem pedir senha**:

```sh
ssh diego echo ok
ssh lab echo ok
```

## No dia — rodar (só no José)

Descubra a faixa de IP da rede de hoje:

```sh
ip -4 addr | grep inet        # ex.: 192.168.30.242/23  ->  a rede e' 192.168.30.0/23
```

Rode, trocando `REDE` pela faixa que apareceu:

```sh
cd ~
mpirun --hostfile mpi-mandelbrot-p1/hosts -np 4 \
    --mca btl_tcp_if_include REDE \
    --mca oob_tcp_if_include REDE \
    -x HWLOC_COMPONENTS \
    mpi-mandelbrot-p1/codigo_fonte/mandelbrot_mpi
```

Ver a imagem:

```sh
convert ~/saida.ppm ~/saida.png && explorer.exe "$(wslpath -w ~/saida.png)"
```

**Plano B (a rede não funcionou de jeito nenhum):** rode `mpirun --oversubscribe -np 4 ./mandelbrot_mpi`
dentro de `~/mpi-mandelbrot-p1/codigo_fonte` para mostrar o programa funcionando numa
máquina só. Explique que os prints da execução em rede estão no relatório.

## O que falar (roteiro)

**1. O problema (30 s).**
"O professor passou o Mandelbrot serial: um processo só calcula os 480 mil pixels. Só que
cada pixel é independente dos outros, então dá para dividir o cálculo entre máquinas."

**2. A ideia (1 min).**
"Usamos o exemplo mestre/escravo do professor. Lá a tarefa era um número; aqui a tarefa é
**uma linha da imagem**. O mestre manda o número da linha, o escravo calcula as 800 cores e
devolve, e o mestre já manda a próxima linha para quem terminou."

**3. Por que linha por linha (30 s).**
"Dividir 1/3 fixo para cada máquina parece mais simples, mas no Mandelbrot tem linha muito
mais pesada que outra: a parte cinza roda as 20000 iterações. Com uma linha por vez, quem
termina antes pega mais, e ninguém fica parado."

**4. O código (1 min 30 s).** Abra `codigo_fonte/mandelbrot_mpi.c` e mostre:

- **o `while` do mestre**: `MPI_Recv` com `MPI_ANY_SOURCE` (aceita de quem terminar
  primeiro), `memcpy` (é aqui que a figura é integrada: a linha `y` vai para a posição
  `y × 800 × 3`) e `MPI_Send` para `stat.MPI_SOURCE` (a próxima linha vai para quem acabou
  de responder);
- **o `while` do escravo**: recebe a linha, `calcula_linha`, devolve, até receber `END`;
- **a `struct LinhaResultado`**: o número da linha mais os 2400 bytes de cor, numa
  mensagem só.

**5. Rodar ao vivo (1 min).** Rode o `mpirun`. Aponte na tela:

- `Maquina 1 esta no computador "Pesquisa"`, `Maquina 2 ... "diego"`, `Maquina 3 ... "lab"`:
  são 3 computadores diferentes;
- `Maquina 3 (lab): calculando linha 7`: cada máquina calcula uma parte;
- `Imagem gravada em saida.ppm`: o mestre juntou tudo.

**6. A imagem (30 s).** Abra o `saida.png` e diga que é a mesma região e as mesmas cores do
programa serial do professor.

**7. O que deu trabalho (30 s).**
"O código foi a parte fácil. O difícil foi fazer as máquinas conversarem: deixar o WSL
visível na rede, SSH sem senha, o `mpirun` travando por causa do hwloc e o Open MPI
precisando ser a mesma instalação em todas as máquinas."

## Perguntas que o professor pode fazer

- **Onde cada máquina faz uma parte da imagem?**
  Cada escravo calcula as linhas que recebe. A saída mostra qual máquina calculou cada linha.
- **Onde o mestre integra a figura?**
  No `memcpy` do laço do mestre, e depois o `salvar_ppm` grava o vetor inteiro.
- **Por que o mestre não calcula?**
  É o modelo do exemplo do professor: o mestre só coordena. Para não desperdiçar o notebook
  do José, ele também roda um escravo (`slots=2`).
- **Por que cada máquina não calcula exatamente 200 linhas?**
  Porque a distribuição é dinâmica: quem termina antes pega mais.
- **Por que reescreveram o cálculo em C?**
  O MPI do exemplo é C (`mpicc`) e o serial é C++ com janela X11. Passamos só as duas
  fórmulas para C, com os mesmos parâmetros. Os arquivos do professor não foram alterados.
- **O que é o `hosts` e o `slots`?**
  A lista de máquinas que o `mpirun` usa e quantos processos ele abre em cada uma. O
  `mpirun` entra nelas por SSH, por isso precisa do SSH sem senha.
- **Para que servem as opções `--mca ..._if_include`?**
  Elas mandam o MPI usar só a placa de rede da rede local. No WSL existem placas virtuais e
  o MPI travava tentando usá-las.
- **Como sabem que a imagem está certa?**
  Mesma região, mesmas fórmulas e mesma paleta do serial. E as imagens geradas em execuções
  diferentes saíram idênticas byte a byte (`md5sum`).
