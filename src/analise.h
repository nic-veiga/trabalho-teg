/*
 * analise.h - Análises estruturais do grafo:
 *   - componentes conexos (tamanho, vértice de grau máximo e mínimo);
 *   - classificação em grafo simples ou multigrafo (laços/arestas múltiplas).
 */
#ifndef ANALISE_H
#define ANALISE_H

#include "grafo.h"

typedef struct {
    int tamanho;                    /* nº de vértices do componente */
    int inicio;                     /* posição do 1º vértice em ordem[] */
    int grau_max;                   /* maior grau dentro do componente */
    int grau_min;                   /* menor grau dentro do componente */
} Componente;

typedef struct {
    int qtd;                        /* nº de componentes conexos */
    Componente *comp;               /* ordenados por tamanho (decrescente) */
    int *ordem;                     /* vértices agrupados por componente */
    int *comp_de;                   /* comp_de[v] = índice em comp[] */
} Componentes;

typedef struct {
    long lacos;                     /* arestas (u,u) */
    long multiplas;                 /* arestas paralelas excedentes */
    long pares_multiplos;           /* pares de vértices com arestas paralelas */
    int simples;                    /* 1 se não há laços nem múltiplas */
} TipoGrafo;

/* Busca em largura (iterativa) em cada componente. 0 = ok, -1 = erro. */
int analisar_componentes(const Grafo *g, Componentes *cs);
void liberar_componentes(Componentes *cs);
void imprimir_componentes(const Grafo *g, const Componentes *cs);

/* Inspeciona as listas de adjacência. 0 = ok, -1 = erro de memória. */
int verificar_tipo_grafo(const Grafo *g, TipoGrafo *t);
void imprimir_tipo_grafo(const Grafo *g, const TipoGrafo *t);

#endif /* ANALISE_H */
