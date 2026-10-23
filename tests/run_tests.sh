#!/usr/bin/env bash
# Testes automatizados da Tarefa 2 (TEG).
# Compila uma versao com AddressSanitizer/UBSan e confere as saidas.
# Uso (na raiz do projeto):  make test   ou   bash tests/run_tests.sh

set -u
cd "$(dirname "$0")/.." || exit 1

BIN="$(mktemp -d)/teg_test"
TMP="$(mktemp -d)"
ENTRADA="entrada.txt"
export ASAN_OPTIONS="exitcode=99"
export UBSAN_OPTIONS="halt_on_error=1:exitcode=98:print_stacktrace=1"

echo "Compilando com sanitizers..."
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O1 \
    -fsanitize=address,undefined -o "$BIN" src/*.c || {
    echo "FALHA: erro de compilacao"; exit 1; }

total=0
falhas=0

# ok DESCRICAO  -> registra um teste aprovado
# falha DESCRICAO -> registra uma falha
ok()    { total=$((total + 1)); printf '  [ok]   %s\n' "$1"; }
falha() { total=$((total + 1)); falhas=$((falhas + 1)); printf '  [FALHOU] %s\n' "$1"; }

# executa o programa guardando stdout em $SAIDA e o codigo de saida em $RC
rodar() { SAIDA="$("$BIN" "$@" 2>"$TMP/stderr" </dev/null)"; RC=$?; ERRO="$(cat "$TMP/stderr")"; }

# contem DESCRICAO REGEX  -> $SAIDA casa com a expressao
contem() {
    if grep -qE -- "$2" <<<"$SAIDA"; then ok "$1"; else falha "$1  (esperado: $2)"; fi
}

# rc_igual DESCRICAO ESPERADO
rc_igual() {
    if [ "$RC" -eq "$2" ]; then ok "$1"; else falha "$1  (rc=$RC, esperado $2)"; fi
}

echo
echo "== Carga e relatorio (modo padrao: sem distincao de caixa) =="
rodar "$ENTRADA" lago cena
rc_igual "execucao termina com codigo 0 (sem erros de sanitizer)" 0
contem "1590 vertices"                  'Vertices \|V\|\s+: 1590$'
contem "7249 arestas"                   'Arestas  \|E\|\s+: 7249$'
contem "14 palavras repetidas fundidas" 'Palavras repetidas: 14'
contem "28 componentes conexos"         'Componentes conexos: 28$'
contem "maior componente: 1561 vertices, central 'caia' grau 26" \
       '#1 +\| +1561 vertices \| central \(grau 26\): caia \|'
contem "35 palavras de grau minimo (4 listadas + 31)" 'periferica \(grau 1\): Abel, Acab, aids, aqui, \.\.\. \(\+31\)'
contem "componentes de 2 vertices: broa/proa" 'central \(grau 1\): broa, proa'
contem "25 componentes isolados (ultimo e o #28)" '#28 +\| +1 vertice +\| isolada: zebu'
contem "grafo classificado como simples"  'o grafo e SIMPLES'
contem "0 lacos"                          'Lacos \(arestas u-u\) +: 0$'
contem "0 arestas multiplas"              'Arestas multiplas \(paralelas\) +: 0 '
contem "soma dos graus = 2|E|"            '\[ok\]'

echo
echo "== Dijkstra: lago -> cena =="
contem "origem e destino no componente #1" 'Destino: cena \(grau 9, componente #1'
contem "caminho A: 4 arestas, peso 34"     '^    arestas: 4 \| peso total: 34$'
# a sequencia ponderada pode ser qualquer caminho de peso 34; confere o custo
n=$(grep -cE '^    arestas: 4 \| peso total: 34$' <<<"$SAIDA")
if [ "$n" -eq 2 ]; then ok "caminhos A e B com 4 arestas e peso 34"; else falha "caminhos A e B (achou $n)"; fi

echo
echo "== Dijkstra: criterios divergem (valores conferidos com implementacao independente) =="
rodar -n "$ENTRADA" casa vida
contem "casa->vida: menor no. de arestas = 4"  '^    arestas: 4 \| peso total: 42$'
contem "casa->vida: menor peso = 42"          'peso total: 42'
contem "casa->vida: comparacao aponta a diferenca" 'mesmo peso total, mas A usa 1 aresta'
rodar -n "$ENTRADA" amor odio
contem "amor->odio: 6 arestas" 'arestas: 6 \|'
contem "amor->odio: menor peso = 36" 'peso total: 36$'

echo
echo "== Entrada do usuario =="
rodar -n "$ENTRADA" LAGO Cena
contem "aceita maiusculas/minusculas misturadas" 'Origem : lago'
rodar -n "$ENTRADA" lago lago
contem "mesma palavra: caminho de 0 arestas" 'arestas: 0 \| peso total: 0'
rodar -n "$ENTRADA" xyzw lago
contem "palavra fora da base e informada" '"xyzw" nao esta na base'
rodar -n "$ENTRADA" abc lago
contem "palavra com tamanho errado e informada" '"abc" nao esta na base'
rodar -n "$ENTRADA" flor lago
contem "palavras em componentes diferentes" 'Nao existe sequencia de transformacoes'
SAIDA="$(printf 'lago\ncena\n\n' | "$BIN" -n "$ENTRADA" 2>&1)"
contem "modo interativo (stdin)" 'arestas: 4 \| peso total: 34'

echo
echo "== Modo estrito (-s): Rosa e rosa sao vertices distintos =="
rodar -s "$ENTRADA" Rosa rosa
contem "1604 vertices"            'Vertices \|V\|\s+: 1604$'
contem "7216 arestas"             'Arestas  \|E\|\s+: 7216$'
contem "40 componentes conexos"   'Componentes conexos: 40$'
contem "Rosa-rosa ligadas por 1 aresta" 'Rosa -\(1\)- rosa'

echo
echo "== Robustez da leitura =="
# BOM UTF-8, CRLF, linhas em branco, tamanhos errados, nao-letras, espacos,
# linha gigante e ultima linha sem quebra de linha.
{
    printf '\xef\xbb\xbfRosa\r\n\r\nrosa\r\nrose\r\nabc\r\nabcde\r\n12ab\r\n  rosa  \r\n'
    head -c 3000 /dev/zero | tr '\0' 'x'; printf '\n'
    printf 'mesa\r\nmasa'
} > "$TMP/sujo.txt"
rodar "$TMP/sujo.txt" mesa masa
rc_igual "arquivo 'sujo' e lido sem erro" 0
contem "10 linhas lidas, 6 validas, 4 ignoradas" 'Linhas lidas +: 10 \(validas: 6, ignoradas: 4\)'
contem "2 repetidas fundidas"  'Palavras repetidas: 2'
contem "4 vertices e 2 arestas" 'Vertices \|V\|\s+: 4$'
contem "2 arestas"             'Arestas  \|E\|\s+: 2$'

rodar nao_existe.txt
rc_igual "arquivo inexistente -> codigo 1" 1
: > "$TMP/vazio.txt"
rodar "$TMP/vazio.txt"
rc_igual "arquivo sem palavras validas -> codigo 1" 1
rodar --opcao-invalida
rc_igual "opcao invalida -> codigo 1" 1
rodar -h
rc_igual "--ajuda -> codigo 0" 0

echo
if [ "$falhas" -eq 0 ]; then
    echo "TODOS OS TESTES PASSARAM ($total verificacoes)."
    exit 0
fi
echo "$falhas de $total verificacoes FALHARAM."
exit 1
