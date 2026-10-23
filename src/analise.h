/*
 * analise.h - Analises estruturais do grafo:
 *   - componentes conexos (tamanho, vertice de grau maximo e minimo);
 *   - classificacao em grafo simples ou multigrafo (lacos/arestas multiplas).
 */
#ifndef ANALISE_H
#define ANALISE_H

#include "grafo.h"

typedef struct {
    int tamanho;                    /* no. de vertices do componente */
    int inicio;                     /* posicao do 1o vertice em ordem[] */
    int grau_max;                   /* maior grau dentro do componente */
    int grau_min;                   /* menor grau dentro do componente */
} Componente;

typedef struct {
    int qtd;                        /* no. de componentes conexos */
    Componente *comp;               /* ordenados por tamanho (decrescente) */
    int *ordem;                     /* vertices agrupados por componente */
    int *comp_de;                   /* comp_de[v] = indice em comp[] */
} Componentes;

typedef struct {
    long lacos;                     /* arestas (u,u) */
    long multiplas;                 /* arestas paralelas excedentes */
    long pares_multiplos;           /* pares de vertices com arestas paralelas */
    int simples;                    /* 1 se nao ha lacos nem multiplas */
} TipoGrafo;

/* Busca em largura (iterativa) em cada componente. 0 = ok, -1 = erro. */
int analisar_componentes(const Grafo *g, Componentes *cs);
void liberar_componentes(Componentes *cs);
void imprimir_componentes(const Grafo *g, const Componentes *cs);

/* Inspeciona as listas de adjacencia. 0 = ok, -1 = erro de memoria. */
int verificar_tipo_grafo(const Grafo *g, TipoGrafo *t);
void imprimir_tipo_grafo(const Grafo *g, const TipoGrafo *t);

#endif /* ANALISE_H */
