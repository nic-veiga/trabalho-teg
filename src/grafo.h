/*
 * grafo.h - Grafo de palavras de 4 letras (lista de adjacencia).
 *
 * Vertices : palavras de 4 letras lidas da base.
 * Arestas  : (p,q) existe se p e q diferem em EXATAMENTE uma posicao.
 * Peso     : distancia alfabetica entre as duas letras que diferem
 *            (ex.: a -> e = 4). Diferenca so de caixa (A/a) vale 1.
 */
#ifndef GRAFO_H
#define GRAFO_H

#define TAM_PALAVRA 4

typedef struct {
    int destino;                    /* indice do vertice vizinho */
    int peso;                       /* distancia alfabetica (>= 1) */
} Aresta;

typedef struct {
    char chave[TAM_PALAVRA + 1];    /* forma usada nas comparacoes */
    char exibicao[TAM_PALAVRA + 1]; /* grafia da 1a ocorrencia na base */
    int grau;                       /* no. de arestas incidentes */
    int capacidade;                 /* capacidade alocada de adj[] */
    Aresta *adj;                    /* lista de adjacencia */
} Vertice;

typedef struct {
    int n;                          /* |V| */
    long m;                         /* |E| (cada aresta contada uma vez) */
    int estrito;                    /* 1 = distingue maiusc./minusc. */
    Vertice *v;                     /* vertices, ordenados por 'chave' */
} Grafo;

typedef struct {
    long linhas_lidas;              /* linhas nao vazias do arquivo */
    long linhas_validas;            /* linhas com palavra de 4 letras */
    long linhas_invalidas;          /* linhas ignoradas */
    long repetidas;                 /* palavras fundidas (duplicatas) */
} EstatisticasCarga;

/*
 * Carga primaria: le o arquivo, cria os vertices (sem duplicatas) e
 * todas as arestas. Retorna 0 em sucesso e -1 em erro (mensagem em stderr).
 * estrito = 0: "Rosa" e "rosa" sao o mesmo vertice.
 * estrito = 1: "Rosa" e "rosa" sao vertices distintos.
 */
int grafo_carregar(Grafo *g, const char *arquivo, int estrito,
                   EstatisticasCarga *est);

void grafo_liberar(Grafo *g);

/* Indice do vertice correspondente a palavra digitada, ou -1. */
int grafo_buscar(const Grafo *g, const char *palavra);

/* Peso da aresta u-v, ou -1 se u e v nao forem adjacentes. */
int grafo_peso_aresta(const Grafo *g, int u, int v);

#endif /* GRAFO_H */
