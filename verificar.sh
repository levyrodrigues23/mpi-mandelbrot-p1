#!/bin/bash
# Confere, uma coisa por vez, tudo que o mpirun precisa. Rode no MESTRE
# (e, se quiser, so a parte "desta maquina" nas outras).
# Cada linha mostra OK ou FALHOU + o comando para consertar.

DIR="$(cd "$(dirname "$0")" && pwd)"
HOSTFILE="${1:-$DIR/hosts}"
ruim=0
ok()    { echo "  [OK]     $1"; }
falha() { echo "  [FALHOU] $1"; echo "           -> $2"; ruim=1; }

echo "== Esta maquina ($(hostname)) =="
command -v mpirun >/dev/null && ok "mpirun instalado" || falha "mpirun nao instalado" "bash setup_maquina.sh"
[ -x "$DIR/codigo_fonte/mandelbrot_mpi" ] && ok "programa compilado" || falha "programa nao compilado" "cd codigo_fonte && make mpi"
grep -q 'HWLOC_COMPONENTS=-gl' ~/.bashrc 2>/dev/null && ok "~/.bashrc tem o conserto do hwloc" || falha "falta o conserto do hwloc no ~/.bashrc" "bash setup_maquina.sh   (depois feche e abra o terminal)"
pgrep -x sshd >/dev/null && ok "servidor SSH rodando" || falha "SSH desligado" "sudo service ssh start"
IP=$(ip -4 route get 1.1.1.1 2>/dev/null | awk '{for (i = 1; i <= NF; i++) if ($i == "src") { print $(i + 1); exit }}')
[ -n "$IP" ] && ok "IP desta maquina: $IP  (usuario: $(whoami))" || falha "sem rede" "conecte no Wi-Fi/cabo"

echo
echo "== Maquinas do arquivo $HOSTFILE =="
HOSTS=$(awk '!/^[[:space:]]*#/ && NF && $1 != "localhost" {print $1}' "$HOSTFILE")
[ -z "$HOSTS" ] && echo "  (nenhuma maquina remota ativa no arquivo)"
for h in $HOSTS; do
    echo " -- $h"
    SSH="ssh -o BatchMode=yes -o ConnectTimeout=8 $h"
    if [ "$($SSH echo ok 2>/dev/null)" != ok ]; then
        falha "ssh $h nao entra sem senha" "ssh-copy-id $h   (e confira ~/.ssh/config, o SSH la e a rede)"
        continue
    fi
    ok "ssh sem senha"
    # bash -c faz o mesmo que o mpirun: shell nao interativo.
    [ "$($SSH 'echo $HWLOC_COMPONENTS' 2>/dev/null)" = "-gl" ] && ok "conserto do hwloc ativo la" || falha "hwloc sem conserto em $h (o mpirun vai travar la)" "em $h: bash setup_maquina.sh"
    $SSH 'command -v mpirun' >/dev/null 2>&1 && ok "mpirun instalado la" || falha "mpirun nao instalado em $h" "em $h: bash setup_maquina.sh"
    $SSH 'test -x ~/mpi-mandelbrot-p1/codigo_fonte/mandelbrot_mpi' 2>/dev/null && ok "programa compilado la (~/mpi-mandelbrot-p1)" || falha "programa nao encontrado em $h" "em $h: cd ~ && git clone https://github.com/levyrodrigues23/mpi-mandelbrot-p1.git && cd mpi-mandelbrot-p1/codigo_fonte && make mpi"
done

echo
[ $ruim -eq 0 ] && echo "Tudo certo. Pode rodar: ./rodar.sh" || echo "Resolva os [FALHOU] acima e rode ./verificar.sh de novo."
exit $ruim
