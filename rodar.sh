#!/bin/bash
# Roda o Mandelbrot distribuido. Use SO no computador do MESTRE.
#
#   ./rodar.sh --local     testa tudo em 1 maquina so (4 processos)
#   ./rodar.sh             usa o arquivo "hosts" (maquinas de verdade)
#   ./rodar.sh outro_hosts usa outro arquivo de hosts
#   ./rodar.sh --quem      so mostra o nome de cada maquina que participa
#                          (prova de que todas estao respondendo)
#
# O que este script resolve sozinho (era o que travava o mpirun):
#  - HWLOC_COMPONENTS=-gl : sem isso o mpirun fica preso para sempre no
#    WSL (ele tenta falar com um servidor grafico X11 que nao responde).
#  - descobre a rede atual (a faixa de IP muda a cada Wi-Fi) e manda o MPI
#    usar so ela, em vez de tentar as placas virtuais do WSL.
#  - o ssh nunca pergunta nada: se faltar chave, falha na hora.
#  - nas outras maquinas o programa e' achado em ~/mpi-mandelbrot-p1, mesmo
#    que o usuario seja diferente (josel, diego...). No mestre, usa a pasta
#    de onde voce rodou este script.

DIR="$(cd "$(dirname "$0")" && pwd)"
export HWLOC_COMPONENTS=-gl
export MESTRE_DIR="$DIR"
export MESTRE_HOST="$(hostname)"

QUEM=0
if [ "$1" = "--quem" ]; then QUEM=1; shift; fi

if [ "$1" = "--local" ]; then
    HOSTFILE="$(mktemp)"
    echo "localhost slots=4" > "$HOSTFILE"
else
    HOSTFILE="${1:-$DIR/hosts}"
fi
[ -f "$HOSTFILE" ] || { echo "ERRO: nao achei o arquivo $HOSTFILE"; exit 1; }

# Total de processos = soma dos "slots=" das linhas que nao sao comentario.
NP=$(awk '!/^[[:space:]]*#/ { for (i = 2; i <= NF; i++) if ($i ~ /^slots=/) { split($i, a, "="); s += a[2] } } END { print s + 0 }' "$HOSTFILE")
[ "$NP" -ge 2 ] || { echo "ERRO: $HOSTFILE precisa somar pelo menos 2 slots"; exit 1; }

# Compila a versao desta maquina (nao faz nada se ja estiver pronta).
[ $QUEM -eq 1 ] || make -s -C "$DIR/codigo_fonte" mpi || { echo "ERRO: make mpi falhou (rode: bash setup_maquina.sh)"; exit 1; }

# Faixa de rede em uso (ex.: 192.168.0.0/24). So quando ha maquinas remotas.
REDE=""
if [ "$1" != "--local" ]; then
    DEV=$(ip -4 route get 1.1.1.1 2>/dev/null | awk '{for (i = 1; i <= NF; i++) if ($i == "dev") { print $(i + 1); exit }}')
    CIDR=$([ -n "$DEV" ] && ip -4 -o addr show dev "$DEV" | awk '{print $4; exit}')
    [ -n "$CIDR" ] && REDE=$(python3 -c 'import ipaddress,sys; print(ipaddress.ip_interface(sys.argv[1]).network)' "$CIDR")
fi
NETOPTS=()
if [ -n "$REDE" ]; then
    echo "Rede usada pelo MPI: $REDE (IP desta maquina: ${CIDR%/*})"
    NETOPTS=(--mca oob_tcp_if_include "$REDE" --mca btl_tcp_if_include "$REDE"
             --mca oob_tcp_disable_family IPv6 --mca btl_tcp_disable_family IPv6)
fi

echo "Rodando $NP processos (hosts: $HOSTFILE)"
if [ $QUEM -eq 1 ]; then
    PROGRAMA=(hostname)
else
    PROGRAMA=(/bin/bash -c 'if [ "$(hostname)" = "$MESTRE_HOST" ]; then cd "$MESTRE_DIR/codigo_fonte"; else cd ~/mpi-mandelbrot-p1/codigo_fonte; fi && exec ./mandelbrot_mpi')
fi
mpirun --hostfile "$HOSTFILE" -np "$NP" "${NETOPTS[@]}" \
    --mca plm_rsh_agent "${AGENTE_SSH:-ssh -o BatchMode=yes -o ConnectTimeout=10}" \
    -x HWLOC_COMPONENTS -x MESTRE_DIR -x MESTRE_HOST \
    "${PROGRAMA[@]}"
STATUS=$?

[ "$1" = "--local" ] && rm -f "$HOSTFILE"
if [ $STATUS -eq 0 ] && [ $QUEM -eq 1 ]; then
    echo "OK! Cada linha acima e' um processo e a maquina onde ele roda."
elif [ $STATUS -eq 0 ]; then
    echo "OK! Imagem do mestre: $DIR/codigo_fonte/saida.ppm"
else
    echo "FALHOU (codigo $STATUS). Rode: ./verificar.sh   para ver o que esta errado."
fi
exit $STATUS
