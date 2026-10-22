/*
 * dijkstra.h - Caminho mínimo entre duas palavras (algoritmo de Dijkstra
 * com fila de prioridade em heap binário).
 *
 * Dois critérios de custo são suportados para permitir a comparação:
 *   CUSTO_UNITARIO  : toda aresta custa 1  -> menor nº de transformações;
 *   CUSTO_PONDERADO : aresta custa o seu peso (distância alfabética).
 */
#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "grafo.h"

typedef enum {
    CUSTO_UNITARIO,
    CUSTO_PONDERADO
} ModoCusto;

typedef struct {
    int alcancavel;                 /* 0 se não existe caminho */
    long custo;                     /* custo total no modo pedido */
    int num_arestas;                /* nº de arestas do caminho */
    int *caminho;                   /* vértices origem..destino
                                       (num_arestas + 1 posições) */
} Caminho;

/* Retorna 0 em sucesso (mesmo sem caminho) e -1 em erro de memória. */
int dijkstra_caminho(const Grafo *g, int origem, int destino, ModoCusto modo,
                     Caminho *saida);

/* Custo do caminho 'c' sob o critério 'modo' (independe de como foi achado). */
long caminho_custo(const Grafo *g, const Caminho *c, ModoCusto modo);

/* 1 se os dois caminhos passam exatamente pelos mesmos vértices. */
int caminhos_iguais(const Caminho *a, const Caminho *b);

void caminho_liberar(Caminho *c);

#endif /* DIJKSTRA_H */
