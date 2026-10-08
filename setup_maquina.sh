#!/bin/sh
# Instala tudo que cada uma das 3 maquinas precisa para compilar e rodar o
# programa MPI (mandelbrot_mpi) e para o mestre conseguir se conectar nas
# outras via SSH: openmpi (mpicc/mpirun), servidor SSH, e um compilador
# C++ para a versao serial de comparacao (ja costuma vir instalado).
#
# Rode isto nas 3 maquinas (seu notebook/WSL, notebook do Diego/WSL, e o
# PC do laboratorio). Precisa de sudo.

set -e

echo "Atualizando lista de pacotes..."
sudo apt-get update

echo "Instalando openmpi, ssh e ferramentas de build..."
sudo apt-get install -y build-essential openmpi-bin libopenmpi-dev openssh-server

echo "Garantindo que o servico SSH esta rodando..."
sudo service ssh start || sudo systemctl start ssh || true

echo
echo "Pronto. Para permitir que o mestre (mpirun) faca login sem senha"
echo "nesta maquina, cole a chave publica do mestre em:"
echo "  ~/.ssh/authorized_keys"
echo
echo "IP desta maquina na rede local:"
hostname -I
