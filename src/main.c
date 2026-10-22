/*
 * main.c - TEG, Tarefa 2: grafo de palavras de 4 letras.
 *
 * Uso: ./teg [opções] [arquivo [origem destino]]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analise.h"
#include "dijkstra.h"
#include "grafo.h"

#define ARQUIVO_PADRAO "entrada.txt"

static void uso(const char *prog)
{
    printf("Uso: %s [opções] [arquivo [origem destino]]\n\n"
           "  arquivo          base de palavras (padrão: %s)\n"
           "  origem destino   calcula o caminho entre as duas palavras e "
           "encerra;\n"
           "                   sem elas, o programa pergunta as palavras "
           "em loop\n\n"
           "Opções:\n"
           "  -s, --estrito        distingue maiúsculas de minúsculas "
           "(\"Rosa\" != \"rosa\")\n"
           "  -n, --sem-relatorio  pula o relatório do grafo (itens 1 a 3)\n"
           "  -h, --ajuda          mostra esta ajuda\n",
           prog, ARQUIVO_PADRAO);
}

/* Lê uma linha (sem o \n). Retorna 0 em EOF. */
static int ler_linha(const char *prompt, char *buf, size_t tam)
{
    size_t len;

    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buf, (int)tam, stdin) == NULL)
        return 0;
    len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
        buf[--len] = '\0';
    return 1;
}

static void imprimir_sequencia(const Grafo *g, const Caminho *c)
{
    int i;

    printf("    ");
    for (i = 0; i <= c->num_arestas; i++) {
        printf("%s", g->v[c->caminho[i]].exibicao);
        if (i < c->num_arestas)
            printf(" -(%d)- ",
                   grafo_peso_aresta(g, c->caminho[i], c->caminho[i + 1]));
    }
    printf("\n");
}

static void resumo_vertice(const Grafo *g, const Componentes *cs, int v)
{
    printf("%s (grau %d, componente #%d de %d vértices)", g->v[v].exibicao,
           g->v[v].grau, cs->comp_de[v] + 1,
           cs->comp[cs->comp_de[v]].tamanho);
}

/* Item 4: caminho por Dijkstra sob dois critérios e comparação. */
static void consultar(const Grafo *g, const Componentes *cs,
                      const char *ori, const char *des)
{
    Caminho a = {0, 0, 0, NULL};     /* custo unitário  */
    Caminho b = {0, 0, 0, NULL};     /* custo ponderado */
    int s = grafo_buscar(g, ori);
    int t = grafo_buscar(g, des);

    if (s < 0 || t < 0) {
        if (s < 0)
            printf("  \"%s\" não está na base (precisa ter 4 letras e "
                   "constar no arquivo).\n", ori);
        if (t < 0)
            printf("  \"%s\" não está na base (precisa ter 4 letras e "
                   "constar no arquivo).\n", des);
        return;
    }

    printf("  Origem : ");
    resumo_vertice(g, cs, s);
    printf("\n  Destino: ");
    resumo_vertice(g, cs, t);
    printf("\n\n");

    if (dijkstra_caminho(g, s, t, CUSTO_UNITARIO, &a) != 0 ||
        dijkstra_caminho(g, s, t, CUSTO_PONDERADO, &b) != 0) {
        fprintf(stderr, "Erro: memória insuficiente no Dijkstra.\n");
        goto fim;
    }

    if (!a.alcancavel) {
        printf("  Não existe sequência de transformações: as palavras estão "
               "em componentes\n  conexos diferentes.\n");
        goto fim;
    }

    {
        long a_peso = caminho_custo(g, &a, CUSTO_PONDERADO);
        long b_arest = b.num_arestas;

        printf("  [A] Dijkstra, custo unitário (cada aresta = 1) -> menor nº "
               "de transformações\n");
        imprimir_sequencia(g, &a);
        printf("    arestas: %d | peso total: %ld\n\n", a.num_arestas, a_peso);

        printf("  [B] Dijkstra, custo ponderado (aresta = distância "
               "alfabética) -> menor peso\n");
        imprimir_sequencia(g, &b);
        printf("    arestas: %ld | peso total: %ld\n\n", b_arest, b.custo);

        /* A minimiza o nº de arestas e B minimiza o peso, logo
         * dif_arestas >= 0 e dif_peso >= 0 sempre. */
        long dif_arestas = b_arest - a.num_arestas;
        long dif_peso = a_peso - b.custo;

        printf("  Comparação:\n");
        printf("    Nº de arestas: A = %d, B = %ld\n", a.num_arestas, b_arest);
        printf("    Peso total   : A = %ld, B = %ld\n", a_peso, b.custo);
        if (caminhos_iguais(&a, &b)) {
            printf("    Resultado: os dois critérios escolheram a mesma "
                   "sequência de arestas.\n");
        } else if (dif_arestas > 0 && dif_peso > 0) {
            printf("    Resultado: os critérios divergem. A usa %ld aresta(s) "
                   "a menos, mas B pesa %ld a menos.\n", dif_arestas, dif_peso);
        } else if (dif_arestas > 0) {
            printf("    Resultado: mesmo peso total, mas A usa %ld aresta(s) "
                   "a menos.\n", dif_arestas);
        } else if (dif_peso > 0) {
            printf("    Resultado: mesmo nº de arestas, mas B pesa %ld a "
                   "menos.\n", dif_peso);
        } else {
            printf("    Resultado: sequências diferentes com custo igual nos "
                   "dois critérios\n    (existe mais de um caminho mínimo; o "
                   "Dijkstra devolve um deles).\n");
        }
    }

fim:
    caminho_liberar(&a);
    caminho_liberar(&b);
}

int main(int argc, char **argv)
{
    const char *posicionais[3];
    int npos = 0, estrito = 0, relatorio = 1, i;
    const char *arquivo;
    Grafo g;
    EstatisticasCarga est;
    Componentes cs;
    TipoGrafo tipo;
    int rc = 0;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--ajuda")) {
            uso(argv[0]);
            return 0;
        } else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--estrito")) {
            estrito = 1;
        } else if (!strcmp(argv[i], "-n") ||
                   !strcmp(argv[i], "--sem-relatorio")) {
            relatorio = 0;
        } else if (argv[i][0] == '-' && argv[i][1] != '\0') {
            fprintf(stderr, "Opção desconhecida: %s\n\n", argv[i]);
            uso(argv[0]);
            return 1;
        } else if (npos < 3) {
            posicionais[npos++] = argv[i];
        } else {
            fprintf(stderr, "Argumentos demais.\n\n");
            uso(argv[0]);
            return 1;
        }
    }
    if (npos == 2) {
        fprintf(stderr, "Informe o arquivo seguido das duas palavras "
                        "(origem e destino).\n\n");
        uso(argv[0]);
        return 1;
    }
    arquivo = npos >= 1 ? posicionais[0] : ARQUIVO_PADRAO;

    /* ---- 1. Carga primária ---- */
    if (grafo_carregar(&g, arquivo, estrito, &est) != 0)
        return 1;
    if (g.n == 0) {
        fprintf(stderr, "Erro: nenhuma palavra válida em \"%s\".\n", arquivo);
        grafo_liberar(&g);
        return 1;
    }
    if (analisar_componentes(&g, &cs) != 0 ||
        verificar_tipo_grafo(&g, &tipo) != 0) {
        fprintf(stderr, "Erro: memória insuficiente.\n");
        liberar_componentes(&cs);
        grafo_liberar(&g);
        return 1;
    }

    if (relatorio) {
        printf("=== 1. Carga primária do grafo ===\n");
        printf("Arquivo           : %s\n", arquivo);
        printf("Comparação        : %s\n",
               estrito ? "estrita (\"Rosa\" e \"rosa\" são vértices distintos)"
                       : "sem distinção de maiúsculas/minúsculas");
        printf("Linhas lidas      : %ld (válidas: %ld, ignoradas: %ld)\n",
               est.linhas_lidas, est.linhas_validas, est.linhas_invalidas);
        printf("Palavras repetidas: %ld (fundidas em um único vértice)\n",
               est.repetidas);
        printf("Vértices |V|      : %d\n", g.n);
        printf("Arestas  |E|      : %ld\n\n", g.m);

        printf("=== 2. Componentes conexos ===\n");
        imprimir_componentes(&g, &cs);

        printf("\n=== 3. Simples ou multigrafo? ===\n");
        imprimir_tipo_grafo(&g, &tipo);
        printf("\n");
    }

    /* ---- 4. Caminhos por Dijkstra ---- */
    printf("=== 4. Caminho entre duas palavras (Dijkstra) ===\n");
    if (npos == 3) {
        consultar(&g, &cs, posicionais[1], posicionais[2]);
    } else {
        char ori[256], des[256];

        printf("Digite duas palavras de 4 letras (ex.: lago e cena). "
               "Linha vazia encerra.\n");
        for (;;) {
            if (!ler_linha("\nPalavra de origem : ", ori, sizeof ori) ||
                ori[0] == '\0' || !strcmp(ori, "sair"))
                break;
            if (!ler_linha("Palavra de destino: ", des, sizeof des) ||
                des[0] == '\0' || !strcmp(des, "sair"))
                break;
            printf("\n");
            consultar(&g, &cs, ori, des);
        }
        printf("\n");
    }

    liberar_componentes(&cs);
    grafo_liberar(&g);
    return rc;
}
