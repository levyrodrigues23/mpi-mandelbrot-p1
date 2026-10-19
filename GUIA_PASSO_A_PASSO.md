# Guia passo a passo — Mandelbrot MPI em 2 ou 3 máquinas

Siga na ordem. Só avance quando o **✅ Conferir** do passo funcionar.

## Visão geral

| Máquina | Papel | Processos | Roda `rodar.sh`? |
|---|---|---|---|
| **José** (notebook) | **MESTRE** (monta a imagem) + calcula | 2 | **Sim, só ele** |
| **Diego** (notebook) | calcula | 1 | Não |
| **Lab** (PC, opcional) | calcula | 1 | Não |

O mestre só distribui e monta a imagem, por isso o José roda 2 processos:
assim as 2 (ou 3) máquinas calculam. Cada máquina pode ser **Linux nativo**
ou **WSL no Windows**; o passo 2 tem uma opção para cada caso.

> **O que travava antes:** o `mpirun` ficava parado para sempre no WSL (até
> no teste local). Já está resolvido no `rodar.sh` e no `setup_maquina.sh`.
> Detalhes no README.

---

## Passo 1 — Em CADA máquina (com internet, pode ser em casa)

```sh
cd ~
git clone https://github.com/levyrodrigues23/mpi-mandelbrot-p1.git
cd mpi-mandelbrot-p1
bash setup_maquina.sh        # pede a senha do sudo; leva alguns minutos
```

Se já tinha clonado antes: `cd ~/mpi-mandelbrot-p1 && git fetch && git reset --hard origin/master`
e rode o `setup_maquina.sh` de novo.

Ao terminar, ele mostra **o IP e o usuário** da máquina: anote. Depois
**feche e abra o terminal** (para valer o `~/.bashrc`).

> Tem que ser em `~` nas máquinas, para o caminho ser `~/mpi-mandelbrot-p1`
> em todas.

✅ **Conferir (no mestre):** `cd ~/mpi-mandelbrot-p1 && ./rodar.sh --local`
tem que terminar em ~5 s com `OK! Imagem do mestre...`. Se isso falhar, o
problema é a instalação, não a rede.

---

## Passo 2 — Deixar a máquina acessível pela rede

Todas as máquinas na **mesma rede** (Wi-Fi ou cabo). Se o Wi-Fi da
universidade não deixar as máquinas se enxergarem, use o **hotspot do
celular** (Plano B).

### Opção A — Linux nativo (ex.: PC do lab)

```sh
sudo systemctl enable --now ssh        # ou: sudo service ssh start
sudo ufw status                        # se "active": sudo ufw allow from 192.168.0.0/24
```

Sem senha do sudo? Peça ao técnico para instalar
`build-essential openmpi-bin libopenmpi-dev openssh-server`
e depois rode só `cd ~/mpi-mandelbrot-p1/codigo_fonte && make mpi`.

### Opção B — WSL no Windows (ex.: notebooks)

1. No Windows, crie `C:\Users\SEU_USUARIO\.wslconfig` com:
   ```
   [wsl2]
   networkingMode=mirrored
   ```
   (cuidado para o Bloco de Notas não salvar como `.wslconfig.txt`)
2. PowerShell **como administrador**:
   ```powershell
   Set-NetFirewallHyperVVMSetting -Name '{40E0AC32-46A5-438A-A0B2-2B479E8F2E90}' -DefaultInboundAction Allow
   New-NetFirewallRule -DisplayName "WSL SSH" -Direction Inbound -Protocol TCP -LocalPort 22 -Action Allow
   ```
3. No PowerShell: `wsl --shutdown`. Abra o Ubuntu de novo.
4. No Ubuntu: `sudo service ssh start`
   (**refazer toda vez que o WSL reiniciar**).

✅ **Conferir:** `hostname -I` no Ubuntu mostra o **mesmo IP** do Wi-Fi no
`ipconfig` do Windows. Se aparecer `172.x.x.x`, o mirrored não pegou:
refaça os passos 1 a 3 (precisa de Windows 11 e `wsl --update`).

---

## Passo 3 — No mestre (José): SSH sem senha

O `mpirun` entra nas outras máquinas por SSH, então isso não pode pedir senha.

**a) Chave SSH.** Só se ainda **não existir** (`ls ~/.ssh/id_ed25519`):
```sh
ssh-keygen          # Enter em tudo
```
⚠️ Se já existir, **não** responda `y` para sobrescrever.

**b) Mandar a chave para cada máquina** (pede a senha dela uma vez; se
perguntar "Are you sure", digite `yes`):
```sh
ssh-copy-id USUARIO_DO_DIEGO@IP_DO_DIEGO
```

**c) Apelidos.** Edite `nano ~/.ssh/config` (Ctrl+O, Enter, Ctrl+X):
```
Host diego
    HostName IP_DO_DIEGO
    User USUARIO_DO_DIEGO

Host lab
    HostName IP_DO_LAB
    User USUARIO_DO_LAB
```
Os apelidos têm que ser **`diego`** e **`lab`** (o arquivo `hosts` usa
esses nomes). **O IP muda quando muda a rede**: confira e atualize aqui.

✅ **Conferir:** `ssh diego echo ok` responde `ok` **sem pedir senha**.

---

## Passo 4 — Rodar (só no mestre)

1. Em `~/mpi-mandelbrot-p1/hosts`, deixe só as máquinas que vão participar
   (o `lab` já vem comentado com `#`; tire o `#` se for usar):
   ```
   localhost slots=2
   diego slots=1
   # lab slots=1
   ```
2. Conferir tudo de uma vez (diz o que falta e como consertar):
   ```sh
   cd ~/mpi-mandelbrot-p1
   ./verificar.sh
   ```
3. Rodar:
   ```sh
   ./rodar.sh
   ```

✅ **Conferir:** aparecem linhas "Mestre manda linha X para a maquina Y" com
Y variando (1, 2, 3), e no fim `OK! Imagem do mestre: .../saida.ppm`.

**Provar que cada máquina trabalhou:** `./rodar.sh --quem` imprime o nome
da máquina de cada processo: o do José 2 vezes e o dos outros 1 vez cada.

**Ver a imagem:** `convert codigo_fonte/saida.ppm saida.png && explorer.exe saida.png`
(`sudo apt-get install imagemagick` na primeira vez).

---

## Se der errado

Primeiro rode `./verificar.sh`: ele aponta o problema.

| Sintoma | Causa provável | O que fazer |
|---|---|---|
| `mpirun` **trava sem mostrar nada** | hwloc sem conserto | `bash setup_maquina.sh` e **reabrir o terminal** (em todas as máquinas) |
| `Permission denied` / pede senha | `ssh-copy-id` não funcionou | Refazer o passo 3b; testar `ssh diego echo ok` |
| `Connection timed out` / `refused` | SSH desligado, IP errado, rede isolada | `sudo service ssh start` na outra máquina; conferir IP no `~/.ssh/config`; testar `ping IP` |
| `ping` não funciona entre as máquinas | Wi-Fi isola os clientes | Usar o hotspot do celular nas máquinas |
| `ssh` funciona mas o `mpirun` trava depois | Firewall bloqueando a volta | WSL: refazer o passo 2B(2). Windows sem WSL mirrored: liberar o firewall da rede privada temporariamente |
| `No such file or directory` | Programa não está em `~/mpi-mandelbrot-p1` ou sem `make mpi` | Refazer o passo 1 nessa máquina |
| `diego` ou `lab` "could not resolve" | Falta o apelido no `~/.ssh/config` | Passo 3c |
| Erro de versão do MPI | OpenMPI diferente | `mpirun --version` em todas e comparar |
| Diego sumiu depois de reiniciar o WSL | SSH não sobe sozinho | `sudo service ssh start` |

## Plano B (se a rede não cooperar)

1. **Hotspot do celular**: conecte as máquinas nele, confira os IPs
   novos (passo 1) e atualize o `~/.ssh/config` do mestre.
2. **Só se não houver outro jeito:** mostrar em uma máquina (`./rodar.sh --local`)
   e explicar que a execução em várias máquinas depende só da rede.

## Checklist

**Em casa:**
- [ ] Clone + `bash setup_maquina.sh` em cada máquina (terminal reaberto)
- [ ] `.wslconfig` + firewall + `wsl --shutdown` nos notebooks com WSL
- [ ] No mestre: `./rodar.sh --local` deu `OK!`

**No dia:**
- [ ] Mesma rede; IP e usuário de cada máquina anotados
- [ ] `sudo service ssh start` em todas
- [ ] Mestre: `ssh-copy-id`, `~/.ssh/config` com os IPs de hoje, `ssh diego echo ok`
- [ ] Mestre: `./verificar.sh` sem `[FALHOU]`, depois `./rodar.sh`
