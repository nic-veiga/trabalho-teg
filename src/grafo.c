/*
 * grafo.c - carga primária do grafo de palavras.
 */
#include "grafo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Utilitários de caracteres (ASCII, independentes de locale)          */
/* ------------------------------------------------------------------ */

static char minuscula(char c)
{
    return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
}

static char maiuscula(char c)
{
    return (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
}

static int eh_letra(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int eh_espaco(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
           c == '\f' || c == '\v';
}

/*
 * Remove espaços/CR/LF das pontas e valida: exatamente 4 letras ASCII.
 * Copia a palavra para 'saida' e devolve 1; caso contrário devolve 0.
 */
static int extrair_palavra(const char *texto, char saida[TAM_PALAVRA + 1])
{
    size_t ini = 0;
    size_t fim = strlen(texto);
    size_t i;

    while (ini < fim && eh_espaco(texto[ini]))
        ini++;
    while (fim > ini && eh_espaco(texto[fim - 1]))
        fim--;

    if (fim - ini != TAM_PALAVRA)
        return 0;
    for (i = 0; i < TAM_PALAVRA; i++)
        if (!eh_letra(texto[ini + i]))
            return 0;

    memcpy(saida, texto + ini, TAM_PALAVRA);
    saida[TAM_PALAVRA] = '\0';
    return 1;
}

/* Peso = distância alfabética entre as letras; no mínimo 1. */
static int peso_letras(char a, char b)
{
    int d = minuscula(a) - minuscula(b);
    if (d < 0)
        d = -d;
    return d ? d : 1;
}

/*
 * Devolve 1 se a e b diferem em exatamente uma posição (e grava essa
 * posição em *pos); devolve 0 caso contrário (0 ou 2+ diferenças).
 */
static int difere_em_uma_posicao(const char *a, const char *b, int *pos)
{
    int i, diferencas = 0;

    for (i = 0; i < TAM_PALAVRA; i++) {
        if (a[i] != b[i]) {
            if (++diferencas > 1)
                return 0;
            *pos = i;
        }
    }
    return diferencas == 1;
}

/* ------------------------------------------------------------------ */
/* Lista de adjacência                                                 */
/* ------------------------------------------------------------------ */

static int adicionar_adj(Vertice *v, int destino, int peso)
{
    if (v->grau == v->capacidade) {
        int nova = v->capacidade ? v->capacidade * 2 : 8;
        Aresta *tmp = realloc(v->adj, (size_t)nova * sizeof *tmp);
        if (tmp == NULL)
            return -1;
        v->adj = tmp;
        v->capacidade = nova;
    }
    v->adj[v->grau].destino = destino;
    v->adj[v->grau].peso = peso;
    v->grau++;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Carga do arquivo                                                    */
/* ------------------------------------------------------------------ */

typedef struct {
    char chave[TAM_PALAVRA + 1];
    char exibicao[TAM_PALAVRA + 1];
    long ordem;                     /* posição na base (desempate) */
} Entrada;

static int cmp_entrada(const void *pa, const void *pb)
{
    const Entrada *a = pa;
    const Entrada *b = pb;
    int c = strcmp(a->chave, b->chave);

    if (c != 0)
        return c;
    return (a->ordem > b->ordem) - (a->ordem < b->ordem);
}

static int adicionar_entrada(Entrada **vet, size_t *qtd, size_t *cap,
                             const char *palavra, int estrito, long ordem)
{
    Entrada *e;
    int i;

    if (*qtd == *cap) {
        size_t nova = *cap ? *cap * 2 : 2048;
        Entrada *tmp = realloc(*vet, nova * sizeof **vet);
        if (tmp == NULL)
            return -1;
        *vet = tmp;
        *cap = nova;
    }
    e = &(*vet)[(*qtd)++];
    for (i = 0; i < TAM_PALAVRA; i++)
        e->chave[i] = estrito ? palavra[i] : minuscula(palavra[i]);
    e->chave[TAM_PALAVRA] = '\0';
    memcpy(e->exibicao, palavra, TAM_PALAVRA + 1);
    e->ordem = ordem;
    return 0;
}

int grafo_carregar(Grafo *g, const char *arquivo, int estrito,
                   EstatisticasCarga *est)
{
    EstatisticasCarga s = {0, 0, 0, 0};
    Entrada *ent = NULL;
    size_t qtd = 0, cap = 0, u = 0, i, j;
    char linha[512];
    int primeira = 1;
    FILE *f;

    memset(g, 0, sizeof *g);
    g->estrito = estrito;

    f = fopen(arquivo, "r");
    if (f == NULL) {
        fprintf(stderr, "Erro: não foi possível abrir \"%s\": ", arquivo);
        perror(NULL);
        return -1;
    }

    while (fgets(linha, sizeof linha, f) != NULL) {
        const char *p = linha;
        const char *q;
        char palavra[TAM_PALAVRA + 1];
        int truncada = 0;
        size_t len = strlen(linha);

        /* Linha maior que o buffer: descarta o resto e a invalida. */
        if (len > 0 && linha[len - 1] != '\n') {
            int c = fgetc(f);
            if (c != EOF) {
                truncada = 1;
                while (c != '\n' && c != EOF)
                    c = fgetc(f);
            }
        }

        if (primeira) {             /* ignora BOM UTF-8, se houver */
            primeira = 0;
            if (strncmp(p, "\xEF\xBB\xBF", 3) == 0)
                p += 3;
        }

        for (q = p; *q != '\0' && eh_espaco(*q); q++)
            ;
        if (*q == '\0')
            continue;               /* linha em branco */

        s.linhas_lidas++;
        if (truncada || !extrair_palavra(p, palavra)) {
            s.linhas_invalidas++;
            continue;
        }
        s.linhas_validas++;
        if (adicionar_entrada(&ent, &qtd, &cap, palavra, estrito,
                              s.linhas_validas) != 0) {
            fprintf(stderr, "Erro: memória insuficiente.\n");
            free(ent);
            fclose(f);
            return -1;
        }
    }
    fclose(f);

    /* Ordena e elimina duplicatas (fica a 1ª ocorrência de cada chave). */
    if (qtd > 0) {
        qsort(ent, qtd, sizeof *ent, cmp_entrada);
        for (i = 0; i < qtd; i++)
            if (u == 0 || strcmp(ent[i].chave, ent[u - 1].chave) != 0)
                ent[u++] = ent[i];
    }
    s.repetidas = (long)(qtd - u);

    g->n = (int)u;
    if (u > 0) {
        g->v = calloc(u, sizeof *g->v);
        if (g->v == NULL) {
            fprintf(stderr, "Erro: memória insuficiente.\n");
            free(ent);
            g->n = 0;
            return -1;
        }
    }
    for (i = 0; i < u; i++) {
        memcpy(g->v[i].chave, ent[i].chave, TAM_PALAVRA + 1);
        memcpy(g->v[i].exibicao, ent[i].exibicao, TAM_PALAVRA + 1);
    }
    free(ent);

    /* Arestas: todo par (i,j), i<j, que difere em exatamente 1 posição. */
    for (i = 0; i < u; i++) {
        for (j = i + 1; j < u; j++) {
            int pos = 0;
            if (difere_em_uma_posicao(g->v[i].chave, g->v[j].chave, &pos)) {
                int w = peso_letras(g->v[i].chave[pos], g->v[j].chave[pos]);
                if (adicionar_adj(&g->v[i], (int)j, w) != 0 ||
                    adicionar_adj(&g->v[j], (int)i, w) != 0) {
                    fprintf(stderr, "Erro: memória insuficiente.\n");
                    grafo_liberar(g);
                    return -1;
                }
                g->m++;
            }
        }
    }

    if (est != NULL)
        *est = s;
    return 0;
}

void grafo_liberar(Grafo *g)
{
    int i;

    if (g->v != NULL)
        for (i = 0; i < g->n; i++)
            free(g->v[i].adj);
    free(g->v);
    memset(g, 0, sizeof *g);
}

/* ------------------------------------------------------------------ */
/* Consultas                                                           */
/* ------------------------------------------------------------------ */

static int busca_binaria(const Grafo *g, const char *chave)
{
    int ini = 0, fim = g->n - 1;

    while (ini <= fim) {
        int meio = ini + (fim - ini) / 2;
        int c = strcmp(chave, g->v[meio].chave);
        if (c == 0)
            return meio;
        if (c < 0)
            fim = meio - 1;
        else
            ini = meio + 1;
    }
    return -1;
}

int grafo_buscar(const Grafo *g, const char *palavra)
{
    char digitada[TAM_PALAVRA + 1];
    char var[TAM_PALAVRA + 1];
    int i, r;

    if (!extrair_palavra(palavra, digitada))
        return -1;

    for (i = 0; i <= TAM_PALAVRA; i++)
        var[i] = minuscula(digitada[i]);

    if (!g->estrito)
        return busca_binaria(g, var);

    /* Modo estrito: tenta como digitada, depois "abcd" e "Abcd". */
    r = busca_binaria(g, digitada);
    if (r >= 0)
        return r;
    r = busca_binaria(g, var);
    if (r >= 0)
        return r;
    var[0] = maiuscula(var[0]);
    return busca_binaria(g, var);
}

int grafo_peso_aresta(const Grafo *g, int u, int v)
{
    int i;

    for (i = 0; i < g->v[u].grau; i++)
        if (g->v[u].adj[i].destino == v)
            return g->v[u].adj[i].peso;
    return -1;
}
