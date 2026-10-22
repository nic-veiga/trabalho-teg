# TEG – Tarefa 2: Grafo de palavras de 4 letras

Implementação em **C (C11)** de um grafo não direcionado e ponderado cujos vértices são
palavras de 4 letras e cujas arestas ligam palavras que diferem em **exatamente uma posição**
(ex.: `lago – logo – lego – cego – cega – cena`).

**Equipe:** _(preencher: nomes e matrículas)_

## O que o programa faz

| Requisito do enunciado | Onde aparece na saída |
|---|---|
| (b) Carga primária do grafo a partir da base | Seção **1** (arquivo, linhas lidas, \|V\|, \|E\|) |
| (c-i) Nº de componentes conexos, tamanho de cada um, palavra central (grau máximo) e palavra de menor conexão (grau mínimo) | Seção **2** |
| (c-ii) Grafo simples ou multigrafo (laços e arestas múltiplas) | Seção **3** |
| (c-iii) Sequência de arestas entre duas palavras do usuário, via **Dijkstra**, com comparação | Seção **4** |

## Compilar e executar (Ubuntu)

Requisitos: `gcc` e `make` (`sudo apt install build-essential`).

```bash
make                      # gera o executável ./teg
./teg                     # usa entrada.txt; relatório + consulta interativa
./teg entrada.txt lago cena        # consulta direta (origem e destino)
./teg -n entrada.txt lago cena     # idem, pulando o relatório (itens 1 a 3)
./teg -s entrada.txt               # modo estrito (ver "Decisões de projeto")
./teg --ajuda
```

Sem compilar via `make`: `gcc -std=c11 -O2 -o teg src/*.c`.

No modo interativo o programa pergunta a **origem** e o **destino** em loop; uma linha vazia
(ou `sair`) encerra. Maiúsculas/minúsculas na digitação são indiferentes (`LAGO` = `lago`).

### Testes

```bash
make test     # compila com AddressSanitizer + UBSan e confere 40 verificações
make debug    # só gera ./teg com sanitizers, para depuração
```

## Estrutura

```
.
├── Makefile
├── entrada.txt          # base de palavras (Moodle)
├── src/
│   ├── grafo.h/.c       # estruturas, carga do arquivo, construção das arestas, busca
│   ├── analise.h/.c     # componentes conexos (BFS), graus, simples × multigrafo
│   ├── dijkstra.h/.c    # Dijkstra com heap binário + reconstrução do caminho
│   └── main.c           # linha de comando, relatório e consulta interativa
└── tests/run_tests.sh   # testes automatizados
```

## Decisões de projeto

O enunciado deixa alguns pontos em aberto; estas foram as escolhas feitas:

**1. Peso das arestas.** O enunciado pede um grafo *ponderado* mas não define o peso. Foi
adotada a **distância alfabética** entre as duas letras que diferem (`a→e` = 4, `o→a` = 14).
Uma diferença só de caixa (`Rosa`/`rosa`, modo estrito) vale 1, para manter todos os pesos ≥ 1.

**2. "Encontre e compare as sequências de arestas… utilizando Dijkstra".** O Dijkstra é executado
**duas vezes**, com critérios de custo diferentes, e os resultados são comparados:

- **[A] custo unitário** (cada aresta = 1): menor número de transformações;
- **[B] custo ponderado** (aresta = seu peso): menor soma de distâncias alfabéticas.

A saída mostra as duas sequências (com o peso de cada aresta), o nº de arestas e o peso total de
ambas, e diz se os critérios coincidem, divergem ou empatam. Exemplo real: `casa → vida` tem
caminho de 4 arestas (A) e outro de 5 arestas (B), ambos com peso total 42.

**3. Maiúsculas × minúsculas.** A base contém 14 palavras que aparecem duas vezes só com a caixa
trocada (`Rosa`/`rosa`, `Roma`/`roma`, `Para`/`para`…). Por padrão elas são **fundidas em um único
vértice** (1604 linhas → **1590 vértices**), mantendo a grafia da primeira ocorrência. Com `-s` /
`--estrito` cada grafia vira um vértice distinto (**1604 vértices**), e `Rosa` e `rosa` passam a ser
adjacentes, pois diferem em uma posição. Os dois modos estão implementados para que a equipe
possa escolher o que a disciplina espera.

**4. Simples × multigrafo.** O programa não assume a resposta: ele **inspeciona as listas de
adjacência** contando laços (`u–u`) e arestas paralelas, e confere que a soma dos graus é 2·\|E\|.
Pela regra do enunciado o grafo sai **simples** (0 laços, 0 arestas múltiplas): uma palavra
difere de si mesma em 0 posições, e um par de palavras distintas é ligado por no máximo uma aresta.

**5. Palavra central e periférica.** Em caso de empate no grau máximo/mínimo, as palavras
empatadas são listadas em ordem alfabética (até 4, seguidas de `(+N)`). Componentes de uma só
palavra aparecem como "isolada" (grau 0).

**6. Leitura do arquivo.** Tolera finais de linha CRLF (a `entrada.txt` do Moodle usa CRLF e não
termina com quebra de linha), BOM UTF-8, linhas em branco e espaços nas pontas. Linhas que não
tenham exatamente 4 letras ASCII são ignoradas e contadas no relatório.

## Complexidade

| Etapa | Custo |
|---|---|
| Construção das arestas (comparação par a par, 4 letras por par) | O(\|V\|²) |
| Componentes conexos (BFS iterativa, sem recursão) | O(\|V\| + \|E\|) |
| Simples × multigrafo | O(\|E\| log grau) |
| Dijkstra (heap binário, remoção preguiçosa) | O((\|V\| + \|E\|) log \|V\|) |

Com a base fornecida (1590 vértices, 7249 arestas) o programa roda em cerca de 10 ms.

## Resultados com a `entrada.txt` fornecida (modo padrão)

- **1590** vértices, **7249** arestas, grafo **simples**.
- **28** componentes conexos: um gigante com **1561** vértices (central: `caia`, grau 26;
  35 palavras com grau mínimo 1), dois pares (`broa–proa`, `IMPA–impo`) e **25 palavras isoladas**.

Trecho da saída para `lago → cena`:

```
  [A] Dijkstra, custo unitário (cada aresta = 1) -> menor nº de transformações
    lago -(9)- cago -(14)- caga -(7)- cana -(4)- cena
    arestas: 4 | peso total: 34

  [B] Dijkstra, custo ponderado (aresta = distância alfabética) -> menor peso
    lago -(4)- lego -(9)- cego -(14)- cega -(7)- cena
    arestas: 4 | peso total: 34
```

Observação: o exemplo do enunciado (`LAGO → LOGO → LEGO → CEGO → CEGA → CENA`, 5 arestas) é uma
sequência válida, mas não a mais curta: existem caminhos com 4 arestas.

## Verificação

Os resultados (|V|, |E|, componentes e custos de Dijkstra) foram conferidos contra uma
implementação independente em 400 pares de palavras sorteados, nos dois modos, sem divergências;
a versão com sanitizers não reportou erros de memória nem comportamento indefinido.
