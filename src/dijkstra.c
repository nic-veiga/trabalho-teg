/*
 * dijkstra.c - Dijkstra com heap binario (remocao preguicosa).
 *
 * Complexidade: O((V + E) log V).
 */
#include "dijkstra.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define INFINITO (LONG_MAX / 2)

/* ------------------------------------------------------------------ */
/* Heap binario de minimo                                              */
/* ------------------------------------------------------------------ */

typedef struct {
    long dist;
    int vertice;
} ItemHeap;

typedef struct {
    ItemHeap *a;
    size_t n;
    size_t cap;
} Heap;

/* Desempata por indice do vertice para o resultado ser deterministico. */
static int menor(ItemHeap x, ItemHeap y)
{
    return x.dist < y.dist || (x.dist == y.dist && x.vertice < y.vertice);
}

static int heap_inserir(Heap *h, long dist, int vertice)
{
    size_t i;

    if (h->n == h->cap) {
        size_t nova = h->cap ? h->cap * 2 : 64;
        ItemHeap *tmp = realloc(h->a, nova * sizeof *tmp);
        if (tmp == NULL)
            return -1;
        h->a = tmp;
        h->cap = nova;
    }
    i = h->n++;
    h->a[i].dist = dist;
    h->a[i].vertice = vertice;
    while (i > 0) {
        size_t pai = (i - 1) / 2;
        ItemHeap tmp;
        if (!menor(h->a[i], h->a[pai]))
            break;
        tmp = h->a[i];
        h->a[i] = h->a[pai];
        h->a[pai] = tmp;
        i = pai;
    }
    return 0;
}

static int heap_remover_minimo(Heap *h, ItemHeap *out)
{
    size_t i = 0;

    if (h->n == 0)
        return 0;
    *out = h->a[0];
    h->a[0] = h->a[--h->n];
    for (;;) {
        size_t esq = 2 * i + 1, dir = esq + 1, m = i;
        ItemHeap tmp;
        if (esq < h->n && menor(h->a[esq], h->a[m]))
            m = esq;
        if (dir < h->n && menor(h->a[dir], h->a[m]))
            m = dir;
        if (m == i)
            break;
        tmp = h->a[i];
        h->a[i] = h->a[m];
        h->a[m] = tmp;
        i = m;
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* Dijkstra                                                            */
/* ------------------------------------------------------------------ */

int dijkstra_caminho(const Grafo *g, int origem, int destino, ModoCusto modo,
                     Caminho *saida)
{
    size_t n = (size_t)(g->n > 0 ? g->n : 1);
    long *dist = malloc(n * sizeof *dist);
    int *pai = malloc(n * sizeof *pai);
    char *fechado = calloc(n, 1);
    Heap heap = {NULL, 0, 0};
    ItemHeap item;
    int i, rc = -1;

    memset(saida, 0, sizeof *saida);
    if (dist == NULL || pai == NULL || fechado == NULL)
        goto fim;

    for (i = 0; i < g->n; i++) {
        dist[i] = INFINITO;
        pai[i] = -1;
    }
    dist[origem] = 0;
    if (heap_inserir(&heap, 0, origem) != 0)
        goto fim;

    while (heap_remover_minimo(&heap, &item)) {
        int u = item.vertice;
        const Vertice *vu = &g->v[u];

        if (fechado[u])
            continue;                   /* entrada obsoleta do heap */
        fechado[u] = 1;
        if (u == destino)
            break;                      /* distancia final ja conhecida */

        for (i = 0; i < vu->grau; i++) {
            int v = vu->adj[i].destino;
            long w = (modo == CUSTO_UNITARIO) ? 1 : vu->adj[i].peso;

            if (!fechado[v] && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pai[v] = u;
                if (heap_inserir(&heap, dist[v], v) != 0)
                    goto fim;
            }
        }
    }

    rc = 0;
    if (dist[destino] >= INFINITO)
        goto fim;                       /* inalcancavel: alcancavel = 0 */

    /* Reconstroi o caminho seguindo os predecessores. */
    {
        int len = 1, v;
        for (v = destino; v != origem; v = pai[v])
            len++;
        saida->caminho = malloc((size_t)len * sizeof *saida->caminho);
        if (saida->caminho == NULL) {
            rc = -1;
            goto fim;
        }
        saida->num_arestas = len - 1;
        for (v = destino, i = len - 1; i >= 0; i--, v = pai[v])
            saida->caminho[i] = v;
        saida->alcancavel = 1;
        saida->custo = dist[destino];
    }

fim:
    free(dist);
    free(pai);
    free(fechado);
    free(heap.a);
    return rc;
}

long caminho_custo(const Grafo *g, const Caminho *c, ModoCusto modo)
{
    long total = 0;
    int i;

    if (!c->alcancavel)
        return -1;
    for (i = 0; i < c->num_arestas; i++)
        total += (modo == CUSTO_UNITARIO)
                     ? 1
                     : grafo_peso_aresta(g, c->caminho[i], c->caminho[i + 1]);
    return total;
}

int caminhos_iguais(const Caminho *a, const Caminho *b)
{
    int i;

    if (!a->alcancavel || !b->alcancavel || a->num_arestas != b->num_arestas)
        return 0;
    for (i = 0; i <= a->num_arestas; i++)
        if (a->caminho[i] != b->caminho[i])
            return 0;
    return 1;
}

void caminho_liberar(Caminho *c)
{
    free(c->caminho);
    memset(c, 0, sizeof *c);
}
