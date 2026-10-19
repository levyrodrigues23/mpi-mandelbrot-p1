# Guia rápido — Mandelbrot MPI em 2 máquinas

**José = mestre** (roda tudo, calcula 2 processos) · **Diego = escravo** (calcula 1).
Só o José roda `./rodar.sh`. Para usar o lab também, veja o final.

## 1. Em casa — nas DUAS máquinas

```sh
cd ~
git clone https://github.com/levyrodrigues23/mpi-mandelbrot-p1.git
cd mpi-mandelbrot-p1
bash setup_maquina.sh      # pede a senha do sudo
```

Anote o **IP e o usuário** que ele mostra no fim. **Feche e abra o terminal.**
(Já tinha clonado? `git fetch && git reset --hard origin/master` e rode o setup de novo.)

No José, teste: `./rodar.sh --local` → tem que dar `OK!` em ~5 s.

## 2. WSL só: deixar visível na rede (1 vez, nos notebooks)

1. Crie `C:\Users\SEU_USUARIO\.wslconfig` com:
   ```
   [wsl2]
   networkingMode=mirrored
   ```
2. PowerShell **como administrador**:
   ```powershell
   Set-NetFirewallHyperVVMSetting -Name '{40E0AC32-46A5-438A-A0B2-2B479E8F2E90}' -DefaultInboundAction Allow
   ```
3. `wsl --shutdown` no PowerShell e abra o Ubuntu de novo.

✅ `hostname -I` no Ubuntu = IP do Wi-Fi no `ipconfig`. (Se for `172.x`, não pegou.)
PC com Linux nativo (lab): pule este passo.

## 3. No dia — mesma rede (ou hotspot do celular)

**Diego:** `sudo service ssh start` e passa o IP/usuário para o José. Fim.

**José:**
```sh
sudo service ssh start
ssh-copy-id USUARIO_DO_DIEGO@IP_DO_DIEGO     # pede a senha dele 1 vez
nano ~/.ssh/config
```
```
Host diego
    HostName IP_DO_DIEGO
    User USUARIO_DO_DIEGO
```
O IP muda quando muda a rede: use o de **hoje**. (Se não existir chave: `ssh-keygen`;
se já existir, **não** sobrescreva.)

✅ `ssh diego echo ok` responde `ok` **sem pedir senha**.

## 4. Rodar (José)

```sh
cd ~/mpi-mandelbrot-p1
./verificar.sh        # diz o que falta; só siga quando não tiver [FALHOU]
./rodar.sh            # roda; imagem em codigo_fonte/saida.ppm
./rodar.sh --quem     # mostra o nome de cada máquina (prova que o Diego trabalhou)
```

## Deu erro?

| Sintoma | Solução |
|---|---|
| `mpirun` trava sem mostrar nada | `bash setup_maquina.sh` na máquina e **reabrir o terminal** |
| pede senha / `Permission denied` | refazer `ssh-copy-id` |
| `timed out` / `refused` | `sudo service ssh start` lá; conferir o IP no `~/.ssh/config`; `ping IP` |
| `ping` falha entre as máquinas | Wi-Fi isola clientes → usar hotspot do celular |
| `ssh` ok, mas `mpirun` trava depois | firewall: refazer o passo 2 (item 2) |
| `No such file or directory` | programa não está em `~/mpi-mandelbrot-p1` nessa máquina (passo 1) |

## Com o lab (3 máquinas)

Passo 1 no lab (Linux: `sudo systemctl enable --now ssh`; sem sudo, peça ao técnico
`build-essential openmpi-bin libopenmpi-dev openssh-server`). No José, adicione o bloco
`Host lab` no `~/.ssh/config`, rode `ssh-copy-id lab` e tire o `#` da linha `lab` em `hosts`.
