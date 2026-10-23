/*
 * dijkstra.h - Caminho minimo entre duas palavras (algoritmo de Dijkstra
 * com fila de prioridade em heap binario).
 *
 * Dois criterios de custo sao suportados para permitir a comparacao:
 *   CUSTO_UNITARIO  : toda aresta custa 1  -> menor no. de transformacoes;
 *   CUSTO_PONDERADO : aresta custa o seu peso (distancia alfabetica).
 */
#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "grafo.h"

typedef enum {
    CUSTO_UNITARIO,
    CUSTO_PONDERADO
} ModoCusto;

typedef struct {
    int alcancavel;                 /* 0 se nao existe caminho */
    long custo;                     /* custo total no modo pedido */
    int num_arestas;                /* no. de arestas do caminho */
    int *caminho;                   /* vertices origem..destino
                                       (num_arestas + 1 posicoes) */
} Caminho;

/* Retorna 0 em sucesso (mesmo sem caminho) e -1 em erro de memoria. */
int dijkstra_caminho(const Grafo *g, int origem, int destino, ModoCusto modo,
                     Caminho *saida);

/* Custo do caminho 'c' sob o criterio 'modo' (independe de como foi achado). */
long caminho_custo(const Grafo *g, const Caminho *c, ModoCusto modo);

/* 1 se os dois caminhos passam exatamente pelos mesmos vertices. */
int caminhos_iguais(const Caminho *a, const Caminho *b);

void caminho_liberar(Caminho *c);

#endif /* DIJKSTRA_H */
