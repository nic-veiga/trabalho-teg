/*
 * analise.c - componentes conexos e classificação simples/multigrafo.
 */
#include "analise.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LISTADOS 4              /* palavras listadas em caso de empate */

static int cmp_int(const void *pa, const void *pb)
{
    int a = *(const int *)pa;
    int b = *(const int *)pb;
    return (a > b) - (a < b);
}

static int cmp_componente(const void *pa, const void *pb)
{
    const Componente *a = pa;
    const Componente *b = pb;

    if (a->tamanho != b->tamanho)
        return b->tamanho - a->tamanho;          /* maior primeiro */
    return (a->inicio > b->inicio) - (a->inicio < b->inicio);
}

/* ------------------------------------------------------------------ */
/* Componentes conexos                                                 */
/* ------------------------------------------------------------------ */

int analisar_componentes(const Grafo *g, Componentes *cs)
{
    int n = g->n;
    size_t tam = (size_t)(n > 0 ? n : 1);
    char *visitado = calloc(tam, 1);
    int topo = 0, s, k, i;

    memset(cs, 0, sizeof *cs);
    cs->ordem = malloc(tam * sizeof *cs->ordem);
    cs->comp_de = malloc(tam * sizeof *cs->comp_de);
    cs->comp = malloc(tam * sizeof *cs->comp);
    if (visitado == NULL || cs->ordem == NULL || cs->comp_de == NULL ||
        cs->comp == NULL) {
        free(visitado);
        liberar_componentes(cs);
        return -1;
    }

    for (s = 0; s < n; s++) {
        int inicio = topo, cabeca = inicio;
        Componente *c;

        if (visitado[s])
            continue;

        /* BFS: o vetor ordem[] funciona como fila e já agrupa o componente */
        visitado[s] = 1;
        cs->ordem[topo++] = s;
        while (cabeca < topo) {
            const Vertice *u = &g->v[cs->ordem[cabeca++]];
            for (i = 0; i < u->grau; i++) {
                int v = u->adj[i].destino;
                if (!visitado[v]) {
                    visitado[v] = 1;
                    cs->ordem[topo++] = v;
                }
            }
        }

        /* Índices crescentes == ordem alfabética (vértices são ordenados) */
        qsort(cs->ordem + inicio, (size_t)(topo - inicio), sizeof(int),
              cmp_int);

        c = &cs->comp[cs->qtd++];
        c->inicio = inicio;
        c->tamanho = topo - inicio;
        c->grau_max = c->grau_min = g->v[cs->ordem[inicio]].grau;
        for (k = inicio + 1; k < topo; k++) {
            int d = g->v[cs->ordem[k]].grau;
            if (d > c->grau_max)
                c->grau_max = d;
            if (d < c->grau_min)
                c->grau_min = d;
        }
    }
    free(visitado);

    qsort(cs->comp, (size_t)cs->qtd, sizeof *cs->comp, cmp_componente);
    for (k = 0; k < cs->qtd; k++)
        for (i = 0; i < cs->comp[k].tamanho; i++)
            cs->comp_de[cs->ordem[cs->comp[k].inicio + i]] = k;

    return 0;
}

void liberar_componentes(Componentes *cs)
{
    free(cs->ordem);
    free(cs->comp_de);
    free(cs->comp);
    memset(cs, 0, sizeof *cs);
}

/* Lista as palavras do componente que têm exatamente 'grau' conexões. */
static void imprimir_palavras_com_grau(const Grafo *g, const Componentes *cs,
                                       const Componente *c, int grau)
{
    int mostradas = 0, total = 0, i;

    for (i = 0; i < c->tamanho; i++)
        if (g->v[cs->ordem[c->inicio + i]].grau == grau)
            total++;

    for (i = 0; i < c->tamanho && mostradas < MAX_LISTADOS; i++) {
        const Vertice *v = &g->v[cs->ordem[c->inicio + i]];
        if (v->grau == grau)
            printf("%s%s", mostradas++ ? ", " : "", v->exibicao);
    }
    if (total > mostradas)
        printf(", ... (+%d)", total - mostradas);
}

void imprimir_componentes(const Grafo *g, const Componentes *cs)
{
    int k;

    printf("Componentes conexos: %d\n", cs->qtd);
    printf("(\"central\" = vértice de grau máximo; \"periférica\" = vértice de "
           "grau mínimo;\n em caso de empate, as palavras empatadas são "
           "listadas em ordem alfabética)\n\n");

    for (k = 0; k < cs->qtd; k++) {
        const Componente *c = &cs->comp[k];

        if (c->tamanho == 1) {
            printf("  #%-2d | %4d vértice  | isolada: %s (grau 0)\n", k + 1,
                   c->tamanho, g->v[cs->ordem[c->inicio]].exibicao);
            continue;
        }
        printf("  #%-2d | %4d vértices | central (grau %d): ", k + 1,
               c->tamanho, c->grau_max);
        imprimir_palavras_com_grau(g, cs, c, c->grau_max);
        printf(" | periférica (grau %d): ", c->grau_min);
        imprimir_palavras_com_grau(g, cs, c, c->grau_min);
        printf("\n");
    }
}

/* ------------------------------------------------------------------ */
/* Simples x multigrafo                                                */
/* ------------------------------------------------------------------ */

/*
 * Convenção: um laço (u,u) aparece UMA vez na lista de u; uma aresta
 * u-v (u != v) aparece na lista de u e na de v. Contamos os pares de
 * vértices só uma vez (do lado do menor índice).
 */
int verificar_tipo_grafo(const Grafo *g, TipoGrafo *t)
{
    int u, i, maior = 0;
    int *tmp;

    memset(t, 0, sizeof *t);
    for (u = 0; u < g->n; u++)
        if (g->v[u].grau > maior)
            maior = g->v[u].grau;

    tmp = malloc((size_t)(maior > 0 ? maior : 1) * sizeof *tmp);
    if (tmp == NULL)
        return -1;

    for (u = 0; u < g->n; u++) {
        const Vertice *v = &g->v[u];

        for (i = 0; i < v->grau; i++)
            tmp[i] = v->adj[i].destino;
        qsort(tmp, (size_t)v->grau, sizeof *tmp, cmp_int);

        for (i = 0; i < v->grau;) {
            int j = i, destino = tmp[i];
            while (j < v->grau && tmp[j] == destino)
                j++;
            if (destino == u) {
                t->lacos += j - i;
            } else if (destino > u && j - i > 1) {
                t->multiplas += (j - i) - 1;
                t->pares_multiplos++;
            }
            i = j;
        }
    }
    free(tmp);

    t->simples = (t->lacos == 0 && t->multiplas == 0);
    return 0;
}

void imprimir_tipo_grafo(const Grafo *g, const TipoGrafo *t)
{
    long soma_graus = 0;
    int u;

    for (u = 0; u < g->n; u++)
        soma_graus += g->v[u].grau;

    printf("Laços (arestas u-u)            : %ld\n", t->lacos);
    printf("Arestas múltiplas (paralelas)  : %ld (em %ld pares de vértices)\n",
           t->multiplas, t->pares_multiplos);
    printf("Soma dos graus = %ld = 2|E| = %ld  [%s]\n", soma_graus, 2 * g->m,
           soma_graus == 2 * g->m ? "ok" : "INCONSISTENTE");

    if (t->simples) {
        printf("Conclusão: o grafo é SIMPLES.\n"
               "  - Não há laços: uma palavra difere de si mesma em 0 "
               "posições, nunca em 1.\n"
               "  - Não há arestas múltiplas: dois vértices distintos "
               "diferem em 1 posição no\n"
               "    máximo uma vez, e cada par é ligado por uma única "
               "aresta.\n");
    } else {
        printf("Conclusão: o grafo é um MULTIGRAFO (%ld laço(s) e %ld aresta(s) "
               "múltipla(s)).\n", t->lacos, t->multiplas);
    }
}
