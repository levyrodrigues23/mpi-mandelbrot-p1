# Guia passo a passo — Mandelbrot MPI em 3 máquinas

Este guia diz **o que fazer em cada computador**, em ordem, com a forma de
conferir se cada passo deu certo. Siga na ordem e só avance quando o
"✅ Conferir" do passo funcionar.

## 0. Visão geral

**O que a prova pede:** 3 máquinas processando com MPI, cada uma calculando
uma parte da imagem, e o *master* juntando tudo na figura completa.

**Como o nosso programa faz isso:** o master manda uma linha da imagem de
cada vez para quem estiver livre; cada máquina calcula a linha e devolve; o
master monta a imagem e grava `saida.ppm`.

| Máquina | Sistema | Papel | Processos |
|---|---|---|---|
| **José** (notebook) | WSL | **MASTER** (organiza e monta a imagem) + calcula | 2 (o master e 1 que calcula) |
| **Diego** (notebook) | WSL | calcula linhas | 1 |
| **Lab** (PC) | Ubuntu | calcula linhas | 1 |

> Por que o José roda 2 processos? O master só distribui e monta a imagem,
> não calcula. Rodando também 1 processo de cálculo no notebook dele, as
> **3 máquinas calculam**, como a prova pede.

Só **o José** executa o `mpirun`. O Diego e o Lab só deixam a máquina
pronta (instalada, compilada, com SSH ligado) e **não rodam nada** na hora
da execução.

### Termos que aparecem

- **IP:** o "endereço" da máquina na rede (ex.: `192.168.0.15`).
- **SSH:** forma de uma máquina entrar na outra pelo terminal. O `mpirun`
  usa SSH para iniciar o programa nas outras máquinas.
- **WSL:** o Linux dentro do Windows (o terminal "Ubuntu").
- **WSL "mirrored":** configuração para o WSL usar o **mesmo IP do
  Windows**. Sem isso, o WSL fica escondido atrás do Windows e as outras
  máquinas não conseguem chegar nele.

---

## 1. Antes do dia (faça em casa, com internet)

Isto evita perder tempo na hora. Faça nas **3 máquinas**.

### 1.1 Preparar o programa (Diego, José e Lab)

Abra o terminal (Ubuntu / WSL) e rode:

```sh
cd ~
git clone https://github.com/levyrodrigues23/mpi-mandelbrot-p1.git
cd mpi-mandelbrot-p1
bash setup_maquina.sh      # pede a senha do sudo, demora alguns minutos
cd codigo_fonte
make mpi
```

> **Importante:** tem que ser em `~` (pasta pessoal) nas três, para o
> caminho ser **o mesmo** em todas: `~/mpi-mandelbrot-p1`.

✅ **Conferir:** rode `ls ~/mpi-mandelbrot-p1/codigo_fonte/mandelbrot_mpi`.
Tem que mostrar o arquivo, sem erro.

✅ **Conferir o teste local (só no José):**

```sh
cd ~/mpi-mandelbrot-p1/codigo_fonte
mpirun -np 4 ./mandelbrot_mpi
```

Tem que aparecer "Imagem gravada em saida.ppm". Se isso falhar, o problema
é o programa/instalação, não a rede.

### 1.2 Configurar o WSL (José e Diego — o Lab NÃO precisa)

No **Windows** (não no Ubuntu):

1. Confira a versão: abra o PowerShell e rode `wsl --version`. Precisa de
   WSL 2.0 ou mais novo e Windows 11. Se der erro, rode `wsl --update`.
2. Crie o arquivo `C:\Users\SEU_USUARIO\.wslconfig` (bloco de notas) com
   exatamente isto:
   ```
   [wsl2]
   networkingMode=mirrored
   ```
   (cuidado para o bloco de notas não salvar como `.wslconfig.txt`)
3. Abra o **PowerShell como administrador** e rode:
   ```powershell
   Set-NetFirewallHyperVVMSetting -Name '{40E0AC32-46A5-438A-A0B2-2B479E8F2E90}' -DefaultInboundAction Allow
   ```
4. No PowerShell, rode `wsl --shutdown` e abra o Ubuntu de novo.

✅ **Conferir:** no Ubuntu rode `hostname -I`; no PowerShell rode `ipconfig`.
O IP do Wi-Fi tem que ser **o mesmo** nos dois. Se for diferente
(`172.x.x.x`), o modo mirrored não pegou: refaça o passo 2 e o 4.

---

## 2. No dia: rede

1. As **3 máquinas** conectam na **mesma rede** (Wi-Fi ou cabo).
2. Em **cada máquina**, no Ubuntu, rode e **anote**:
   ```sh
   hostname -I      # o IP
   whoami           # o nome de usuário
   ```
3. Preencha esta tabela (o José precisa dela):

   | Máquina | IP | Usuário |
   |---|---|---|
   | José | | |
   | Diego | | |
   | Lab | | |

4. Os 3 primeiros números dos IPs devem ser **iguais** (ex.:
   `192.168.0.x`). Se forem diferentes (ex.: Lab no cabo em
   `10.x.x.x` e os notebooks no Wi-Fi em `192.168.x.x`), as máquinas estão
   em redes separadas e podem não se enxergar → veja "Problemas".

> O PC do Lab pode estar no **cabo** sem problema, desde que esteja na
> mesma rede dos notebooks.

---

## 3. No dia: o que fazer em CADA computador

### 3.1 No PC do LAB (Ubuntu)

Você só precisa deixar o SSH ligado e dar o IP e o usuário para o José.

```sh
sudo service ssh start || sudo systemctl start ssh
sudo ufw status          # se disser "active", rode: sudo ufw allow 22
hostname -I              # anote
whoami                   # anote
```

✅ **Conferir:** `sudo service ssh status` mostra "active (running)".

Pronto. Daqui para frente o Lab **não faz mais nada**. Só não desligue,
não bloqueie a sessão e não feche a rede.

### 3.2 No notebook do DIEGO (WSL)

```sh
sudo service ssh start   # PRECISA ser refeito toda vez que o WSL reiniciar
hostname -I              # deve ser igual ao IP do Wi-Fi no ipconfig
whoami                   # anote
```

✅ **Conferir:** `sudo service ssh status` mostra que está rodando e o
`hostname -I` bate com o `ipconfig` do Windows.

Pronto. O Diego **não faz mais nada**. Deixe o Ubuntu aberto.

### 3.3 No notebook do JOSÉ (MASTER) — o passo mais longo

**a) Garantir o modo mirrored** (feito no passo 1.2): `hostname -I` igual ao
`ipconfig`.

**b) Criar a chave SSH (só na primeira vez):**

```sh
ssh-keygen          # aperte Enter em tudo
```

**c) Autorizar a entrada nas outras máquinas sem senha.** Troque pelos IPs
e usuários da tabela. Vai pedir a senha de cada máquina **uma vez**, e
na primeira vez pergunta "Are you sure...": digite `yes`.

```sh
ssh-copy-id USUARIO_DO_DIEGO@IP_DO_DIEGO
ssh-copy-id USUARIO_DO_LAB@IP_DO_LAB
```

**d) Criar apelidos.** Abra o arquivo `~/.ssh/config`:

```sh
nano ~/.ssh/config
```

e cole (trocando pelos valores da tabela):

```
Host diego
    HostName IP_DO_DIEGO
    User USUARIO_DO_DIEGO

Host lab
    HostName IP_DO_LAB
    User USUARIO_DO_LAB
```

Salve com `Ctrl+O`, `Enter`, e saia com `Ctrl+X`.

> O arquivo `hosts` do repositório já usa os apelidos `diego` e `lab`.
> Por isso os apelidos **têm que ter exatamente esses nomes**.

**e) Testar o SSH. Os dois têm que responder `ok` sem pedir senha:**

```sh
ssh diego echo ok
ssh lab echo ok
```

🛑 **Só continue quando os dois funcionarem.** Se pedir senha ou travar,
veja "Problemas".

**f) Testar se o programa existe nas outras máquinas:**

```sh
ssh diego ls mpi-mandelbrot-p1/codigo_fonte/mandelbrot_mpi
ssh lab ls mpi-mandelbrot-p1/codigo_fonte/mandelbrot_mpi
```

Os dois têm que mostrar o nome do arquivo.

**g) Rodar** (**a partir de `~`**):

```sh
cd ~
mpirun --hostfile mpi-mandelbrot-p1/hosts -np 4 mpi-mandelbrot-p1/codigo_fonte/mandelbrot_mpi
```

✅ **Conferir:** o terminal mostra "Mestre manda linha X para a maquina Y"
com Y variando entre 1, 2 e 3, e no fim "Imagem gravada em saida.ppm". A
imagem fica em `~/saida.ppm`.

**h) Ver a imagem:**

```sh
sudo apt-get install -y imagemagick   # só na primeira vez (faça em casa)
cd ~
convert saida.ppm saida.png
explorer.exe saida.png
```

---

## 4. Como provar que as 3 máquinas trabalharam

Para mostrar ao professor que as 3 participaram, confira as mensagens do
terminal: cada linha "Maquina N: calculando linha X" vem de um processo.
Para ver **em qual computador** cada processo rodou, rode:

```sh
cd ~
mpirun --hostfile mpi-mandelbrot-p1/hosts -np 4 hostname
```

Tem que imprimir 2 vezes o nome do notebook do José, 1 vez o do Diego e 1
vez o do Lab.

---

## 5. Problemas comuns

| Sintoma | Causa provável | O que fazer |
|---|---|---|
| `ssh ...` pede senha no `mpirun` | `ssh-copy-id` não funcionou | Refazer o 3.3 (c) e testar o (e) |
| `ssh ...` fica travado / "Connection timed out" | Rede isolando clientes, firewall, ou SSH desligado | Conferir `ping IP`; ligar o SSH (3.1/3.2); ver se os IPs estão na mesma faixa |
| `ping` não funciona entre as máquinas | Rede com "isolamento de clientes" (comum em Wi-Fi institucional) | Usar um roteador próprio ou o **hotspot de um celular** para as 3 máquinas |
| `ssh` funciona mas o `mpirun` trava sem sair | Firewall bloqueando a volta (o Lab/Diego não conseguem conectar no José) | Refazer o firewall do Hyper-V no José (1.2 passo 3) e conferir que o IP do WSL é o do Wi-Fi |
| `No such file or directory` ao rodar | Programa não está no mesmo caminho em todas | Conferir que está em `~/mpi-mandelbrot-p1` e que rodou `make mpi` em cada uma |
| `hostname -I` mostra `172.x.x.x` no WSL | Modo mirrored não ativou | Refazer 1.2 (confira `.wslconfig` e rode `wsl --shutdown`) |
| `Connection refused` | SSH não está rodando | `sudo service ssh start` na máquina em questão |
| O SSH não conecta no Diego depois de reiniciar o WSL | O SSH não sobe sozinho no WSL | `sudo service ssh start` de novo |
| Erro de versão / "incompatible" do MPI | OpenMPI diferente entre as máquinas | Rodar `mpirun --version` nas 3 e comparar; reinstalar na que estiver diferente |
| `mpirun` escolhe uma interface errada e trava | Existem várias placas de rede | Tentar `mpirun --mca btl_tcp_if_include wlan0 ...` (troque pelo nome da interface que aparece em `ip a`) |

---

## 6. Plano B (se a rede não cooperar)

Em ordem de preferência:

1. **Hotspot do celular:** conecte as 3 máquinas nele (o celular vira o
   "roteador"). Refaça a seção 2 porque os IPs mudam, e atualize o
   `~/.ssh/config` do José.
2. **Mostrar a versão funcionando em uma máquina só**, simulando as 3
   (`mpirun -np 4 ./mandelbrot_mpi`) e explicar que a execução em 3
   máquinas depende só da rede. **Só use isso se não houver outro jeito**,
   pois a prova pede as 3 máquinas de verdade.

---

## 7. Checklist rápido

**Antes (em casa):**
- [ ] Clone, `setup_maquina.sh` e `make mpi` nas 3 máquinas
- [ ] `.wslconfig` + firewall + `wsl --shutdown` no José e no Diego
- [ ] Teste local no José (`mpirun -np 4 ./mandelbrot_mpi`)

**No dia:**
- [ ] 3 máquinas na mesma rede, IPs na mesma faixa
- [ ] IP e usuário de cada uma anotados
- [ ] Lab: SSH ligado
- [ ] Diego: `sudo service ssh start`
- [ ] José: `ssh-copy-id`, `~/.ssh/config`, `ssh diego echo ok`, `ssh lab echo ok`
- [ ] José: `mpirun --hostfile ... -np 4 ...` e a imagem em `~/saida.ppm`
