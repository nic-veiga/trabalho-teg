/*
 * grafo.h - Grafo de palavras de 4 letras (lista de adjacência).
 *
 * Vértices : palavras de 4 letras lidas da base.
 * Arestas  : (p,q) existe se p e q diferem em EXATAMENTE uma posição.
 * Peso     : distância alfabética entre as duas letras que diferem
 *            (ex.: a -> e = 4). Diferença só de caixa (A/a) vale 1.
 */
#ifndef GRAFO_H
#define GRAFO_H

#define TAM_PALAVRA 4

typedef struct {
    int destino;                    /* índice do vértice vizinho */
    int peso;                       /* distância alfabética (>= 1) */
} Aresta;

typedef struct {
    char chave[TAM_PALAVRA + 1];    /* forma usada nas comparações */
    char exibicao[TAM_PALAVRA + 1]; /* grafia da 1ª ocorrência na base */
    int grau;                       /* nº de arestas incidentes */
    int capacidade;                 /* capacidade alocada de adj[] */
    Aresta *adj;                    /* lista de adjacência */
} Vertice;

typedef struct {
    int n;                          /* |V| */
    long m;                         /* |E| (cada aresta contada uma vez) */
    int estrito;                    /* 1 = distingue maiúsc./minúsc. */
    Vertice *v;                     /* vértices, ordenados por 'chave' */
} Grafo;

typedef struct {
    long linhas_lidas;              /* linhas não vazias do arquivo */
    long linhas_validas;            /* linhas com palavra de 4 letras */
    long linhas_invalidas;          /* linhas ignoradas */
    long repetidas;                 /* palavras fundidas (duplicatas) */
} EstatisticasCarga;

/*
 * Carga primária: lê o arquivo, cria os vértices (sem duplicatas) e
 * todas as arestas. Retorna 0 em sucesso e -1 em erro (mensagem em stderr).
 * estrito = 0: "Rosa" e "rosa" são o mesmo vértice.
 * estrito = 1: "Rosa" e "rosa" são vértices distintos.
 */
int grafo_carregar(Grafo *g, const char *arquivo, int estrito,
                   EstatisticasCarga *est);

void grafo_liberar(Grafo *g);

/* Índice do vértice correspondente à palavra digitada, ou -1. */
int grafo_buscar(const Grafo *g, const char *palavra);

/* Peso da aresta u-v, ou -1 se u e v não forem adjacentes. */
int grafo_peso_aresta(const Grafo *g, int u, int v);

#endif /* GRAFO_H */
