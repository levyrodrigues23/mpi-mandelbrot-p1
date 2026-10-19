#!/bin/bash
# Prepara UMA maquina (rode em cada uma: mestre, escrava, lab).
# Instala OpenMPI + SSH, compila o programa e aplica o conserto do hwloc.
# Precisa de sudo (pede a senha desta maquina).

set -e
DIR="$(cd "$(dirname "$0")" && pwd)"

echo "[1/4] Instalando openmpi, ssh e ferramentas de build..."
sudo apt-get update
sudo apt-get install -y build-essential openmpi-bin libopenmpi-dev openssh-server

echo "[2/4] Ligando o servidor SSH..."
sudo service ssh start || sudo systemctl start ssh || true

# Sem esta linha o mpirun trava para sempre no WSL (o hwloc fica tentando
# falar com um servidor grafico X11 que nao responde). Tem que ficar no
# INICIO do ~/.bashrc, antes da parte "se nao for interativo, sai", senao
# o ssh (que e' nao interativo) nao enxerga.
echo "[3/4] Aplicando o conserto do hwloc no ~/.bashrc..."
if ! grep -q 'HWLOC_COMPONENTS=-gl' ~/.bashrc 2>/dev/null; then
    { echo 'export HWLOC_COMPONENTS=-gl   # evita o mpirun travar (setup_maquina.sh)'; cat ~/.bashrc 2>/dev/null; } > ~/.bashrc.novo
    mv ~/.bashrc.novo ~/.bashrc
fi

echo "[4/4] Compilando..."
make -C "$DIR/codigo_fonte" mpi

IP=$(ip -4 route get 1.1.1.1 2>/dev/null | awk '{for (i = 1; i <= NF; i++) if ($i == "src") { print $(i + 1); exit }}')
echo
echo "PRONTO. Anote para o mestre:"
echo "  IP:      ${IP:-"(sem rede ainda)"}"
echo "  usuario: $(whoami)"
echo
echo "IMPORTANTE: feche este terminal e abra de novo (para valer o ~/.bashrc)."
