/*
 * main.c - TEG, Tarefa 2: grafo de palavras de 4 letras.
 *
 * Uso: ./teg                              (menu interativo)
 *      ./teg [opcoes] [arquivo [origem destino]]   (linha de comando)
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
    printf("Uso: %s                (sem argumentos: abre o menu interativo)\n"
           "     %s [opcoes] [arquivo [origem destino]]\n\n"
           "  arquivo          base de palavras (padrao: %s)\n"
           "  origem destino   calcula o caminho entre as duas palavras e "
           "encerra;\n"
           "                   sem elas, o programa pergunta as palavras "
           "em loop\n\n"
           "Opcoes:\n"
           "  -s, --estrito        distingue maiusculas de minusculas "
           "(\"Rosa\" != \"rosa\")\n"
           "  -n, --sem-relatorio  pula o relatorio do grafo (itens 1 a 3)\n"
           "  -m, --menu           abre o menu interativo\n"
           "  -h, --ajuda          mostra esta ajuda\n",
           prog, prog, ARQUIVO_PADRAO);
}

/* Le uma linha (sem o \n). Retorna 0 em EOF. */
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
    printf("%s (grau %d, componente #%d de %d vertices)", g->v[v].exibicao,
           g->v[v].grau, cs->comp_de[v] + 1,
           cs->comp[cs->comp_de[v]].tamanho);
}

/* Item 4: caminho por Dijkstra sob dois criterios e comparacao. */
static void consultar(const Grafo *g, const Componentes *cs,
                      const char *ori, const char *des)
{
    Caminho a = {0, 0, 0, NULL};     /* custo unitario  */
    Caminho b = {0, 0, 0, NULL};     /* custo ponderado */
    int s = grafo_buscar(g, ori);
    int t = grafo_buscar(g, des);

    if (s < 0 || t < 0) {
        if (s < 0)
            printf("  \"%s\" nao esta na base (precisa ter 4 letras e "
                   "constar no arquivo).\n", ori);
        if (t < 0)
            printf("  \"%s\" nao esta na base (precisa ter 4 letras e "
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
        fprintf(stderr, "Erro: memoria insuficiente no Dijkstra.\n");
        goto fim;
    }

    if (!a.alcancavel) {
        printf("  Nao existe sequencia de transformacoes: as palavras estao "
               "em componentes\n  conexos diferentes.\n");
        goto fim;
    }

    {
        long a_peso = caminho_custo(g, &a, CUSTO_PONDERADO);
        long b_arest = b.num_arestas;

        printf("  [A] Dijkstra, custo unitario (cada aresta = 1) -> menor no. "
               "de transformacoes\n");
        imprimir_sequencia(g, &a);
        printf("    arestas: %d | peso total: %ld\n\n", a.num_arestas, a_peso);

        printf("  [B] Dijkstra, custo ponderado (aresta = distancia "
               "alfabetica) -> menor peso\n");
        imprimir_sequencia(g, &b);
        printf("    arestas: %ld | peso total: %ld\n\n", b_arest, b.custo);

        /* A minimiza o no. de arestas e B minimiza o peso, logo
         * dif_arestas >= 0 e dif_peso >= 0 sempre. */
        long dif_arestas = b_arest - a.num_arestas;
        long dif_peso = a_peso - b.custo;

        printf("  Comparacao:\n");
        printf("    No. de arestas: A = %d, B = %ld\n", a.num_arestas, b_arest);
        printf("    Peso total   : A = %ld, B = %ld\n", a_peso, b.custo);
        if (caminhos_iguais(&a, &b)) {
            printf("    Resultado: os dois criterios escolheram a mesma "
                   "sequencia de arestas.\n");
        } else if (dif_arestas > 0 && dif_peso > 0) {
            printf("    Resultado: os criterios divergem. A usa %ld aresta(s) "
                   "a menos, mas B pesa %ld a menos.\n", dif_arestas, dif_peso);
        } else if (dif_arestas > 0) {
            printf("    Resultado: mesmo peso total, mas A usa %ld aresta(s) "
                   "a menos.\n", dif_arestas);
        } else if (dif_peso > 0) {
            printf("    Resultado: mesmo no. de arestas, mas B pesa %ld a "
                   "menos.\n", dif_peso);
        } else {
            printf("    Resultado: sequencias diferentes com custo igual nos "
                   "dois criterios\n    (existe mais de um caminho minimo; o "
                   "Dijkstra devolve um deles).\n");
        }
    }

fim:
    caminho_liberar(&a);
    caminho_liberar(&b);
}

/* ------------------------------------------------------------------ */
/* Secoes do relatorio (usadas pela linha de comando e pelo menu)      */
/* ------------------------------------------------------------------ */

static void relatorio_carga(const Grafo *g, const EstatisticasCarga *est,
                            const char *arquivo, int estrito)
{
    printf("=== 1. Carga primaria do grafo ===\n");
    printf("Arquivo           : %s\n", arquivo);
    printf("Comparacao        : %s\n",
           estrito ? "estrita (\"Rosa\" e \"rosa\" sao vertices distintos)"
                   : "sem distincao de maiusculas/minusculas");
    printf("Linhas lidas      : %ld (validas: %ld, ignoradas: %ld)\n",
           est->linhas_lidas, est->linhas_validas, est->linhas_invalidas);
    printf("Palavras repetidas: %ld (fundidas em um unico vertice)\n",
           est->repetidas);
    printf("Vertices |V|      : %d\n", g->n);
    printf("Arestas  |E|      : %ld\n\n", g->m);
}

static void relatorio_componentes(const Grafo *g, const Componentes *cs)
{
    printf("=== 2. Componentes conexos ===\n");
    imprimir_componentes(g, cs);
    printf("\n");
}

static void relatorio_tipo(const Grafo *g, const TipoGrafo *tipo)
{
    printf("=== 3. Simples ou multigrafo? ===\n");
    imprimir_tipo_grafo(g, tipo);
    printf("\n");
}

/* ------------------------------------------------------------------ */
/* Menu interativo                                                     */
/* ------------------------------------------------------------------ */

typedef struct {
    int carregado;                  /* 1 se g/cs/tipo/est sao validos */
    int estrito;
    char arquivo[256];
    Grafo g;
    Componentes cs;
    TipoGrafo tipo;
    EstatisticasCarga est;
} Estado;

static void estado_descarregar(Estado *e)
{
    if (e->carregado) {
        liberar_componentes(&e->cs);
        grafo_liberar(&e->g);
        e->carregado = 0;
    }
}

/* Carrega 'arquivo' com o modo atual. Em caso de erro mantem a base antiga. */
static int estado_carregar(Estado *e, const char *arquivo)
{
    Grafo g;
    EstatisticasCarga est;
    Componentes cs;
    TipoGrafo tipo;

    memset(&cs, 0, sizeof cs);
    if (grafo_carregar(&g, arquivo, e->estrito, &est) != 0)
        return -1;
    if (g.n == 0) {
        fprintf(stderr, "Erro: nenhuma palavra valida em \"%s\".\n", arquivo);
        grafo_liberar(&g);
        return -1;
    }
    if (analisar_componentes(&g, &cs) != 0 ||
        verificar_tipo_grafo(&g, &tipo) != 0) {
        fprintf(stderr, "Erro: memoria insuficiente.\n");
        liberar_componentes(&cs);
        grafo_liberar(&g);
        return -1;
    }

    estado_descarregar(e);
    e->g = g;
    e->cs = cs;
    e->tipo = tipo;
    e->est = est;
    snprintf(e->arquivo, sizeof e->arquivo, "%s", arquivo);
    e->carregado = 1;
    return 0;
}

static int exige_carga(const Estado *e)
{
    if (!e->carregado)
        printf("\n  Nenhuma base carregada. Use a opcao 1 primeiro.\n");
    return e->carregado;
}

static void menu_mostrar(const Estado *e)
{
    printf("\n==================== TEG - Grafo de palavras ====================\n");
    if (e->carregado)
        printf(" Base: %s  (%d vertices, %ld arestas)\n", e->arquivo, e->g.n,
               e->g.m);
    else
        printf(" Base: (nenhuma carregada)\n");
    printf(" Comparacao: %s\n",
           e->estrito ? "estrita (Rosa != rosa)"
                      : "sem distincao de maiusculas/minusculas");
    printf("-------------------------------------------------------------------\n");
    printf("  1) Carregar base de palavras\n");
    printf("  2) Resumo da carga (vertices e arestas)\n");
    printf("  3) Componentes conexos (tamanhos, grau maximo e minimo)\n");
    printf("  4) Grafo simples ou multigrafo?\n");
    printf("  5) Caminho entre duas palavras (Dijkstra)\n");
    printf("  6) Relatorio completo (2 + 3 + 4)\n");
    printf("  7) Alternar modo de comparacao (recarrega a base)\n");
    printf("  0) Sair\n");
    printf("===================================================================\n");
}

static void menu_carregar(Estado *e)
{
    char nome[256];
    char prompt[320];

    snprintf(prompt, sizeof prompt, "\nArquivo da base [Enter = %s]: ",
             e->carregado ? e->arquivo : ARQUIVO_PADRAO);
    if (!ler_linha(prompt, nome, sizeof nome))
        return;
    if (nome[0] == '\0')
        snprintf(nome, sizeof nome, "%s",
                 e->carregado ? e->arquivo : ARQUIVO_PADRAO);

    printf("\n");
    if (estado_carregar(e, nome) == 0)
        relatorio_carga(&e->g, &e->est, e->arquivo, e->estrito);
    else
        printf("  Nao foi possivel carregar \"%s\".\n", nome);
}

static void menu_consulta(const Estado *e)
{
    char ori[256], des[256];

    printf("\nDigite duas palavras de 4 letras (ex.: lago e cena). "
           "Enter vazio volta ao menu.\n");
    for (;;) {
        if (!ler_linha("\nPalavra de origem : ", ori, sizeof ori) ||
            ori[0] == '\0' || !strcmp(ori, "sair"))
            break;
        if (!ler_linha("Palavra de destino: ", des, sizeof des) ||
            des[0] == '\0' || !strcmp(des, "sair"))
            break;
        printf("\n");
        consultar(&e->g, &e->cs, ori, des);
    }
}

static void menu_alternar_modo(Estado *e)
{
    int novo = !e->estrito;

    if (!e->carregado) {
        e->estrito = novo;
        printf("\n  Modo alterado para: %s.\n",
               novo ? "estrito" : "sem distincao de maiusculas");
        return;
    }
    /* recarrega com o novo modo; se falhar, mantem o modo anterior */
    e->estrito = novo;
    {
        char arq[256];

        snprintf(arq, sizeof arq, "%s", e->arquivo);
        if (estado_carregar(e, arq) == 0) {
            printf("\n  Modo alterado e base recarregada.\n\n");
            relatorio_carga(&e->g, &e->est, e->arquivo, e->estrito);
        } else {
            e->estrito = !novo;
            printf("\n  Falha ao recarregar; modo mantido.\n");
        }
    }
}

static int menu_principal(void)
{
    Estado e;
    char op[64];

    memset(&e, 0, sizeof e);

    for (;;) {
        menu_mostrar(&e);
        if (!ler_linha("Escolha uma opcao: ", op, sizeof op))
            break;                              /* EOF (Ctrl+Z / Ctrl+D) */

        switch (op[0] == '\0' ? '?' : op[0]) {
        case '1':
            menu_carregar(&e);
            break;
        case '2':
            if (exige_carga(&e)) {
                printf("\n");
                relatorio_carga(&e.g, &e.est, e.arquivo, e.estrito);
            }
            break;
        case '3':
            if (exige_carga(&e)) {
                printf("\n");
                relatorio_componentes(&e.g, &e.cs);
            }
            break;
        case '4':
            if (exige_carga(&e)) {
                printf("\n");
                relatorio_tipo(&e.g, &e.tipo);
            }
            break;
        case '5':
            if (exige_carga(&e))
                menu_consulta(&e);
            break;
        case '6':
            if (exige_carga(&e)) {
                printf("\n");
                relatorio_carga(&e.g, &e.est, e.arquivo, e.estrito);
                relatorio_componentes(&e.g, &e.cs);
                relatorio_tipo(&e.g, &e.tipo);
            }
            break;
        case '7':
            menu_alternar_modo(&e);
            break;
        case '0':
            estado_descarregar(&e);
            printf("\nAte logo!\n");
            return 0;
        default:
            printf("\n  Opcao invalida.\n");
        }
    }
    estado_descarregar(&e);
    printf("\n");
    return 0;
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

    /* Sem argumentos: menu interativo. */
    if (argc == 1)
        return menu_principal();

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--ajuda")) {
            uso(argv[0]);
            return 0;
        } else if (!strcmp(argv[i], "-m") || !strcmp(argv[i], "--menu")) {
            return menu_principal();
        } else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--estrito")) {
            estrito = 1;
        } else if (!strcmp(argv[i], "-n") ||
                   !strcmp(argv[i], "--sem-relatorio")) {
            relatorio = 0;
        } else if (argv[i][0] == '-' && argv[i][1] != '\0') {
            fprintf(stderr, "Opcao desconhecida: %s\n\n", argv[i]);
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

    /* ---- 1. Carga primaria ---- */
    if (grafo_carregar(&g, arquivo, estrito, &est) != 0)
        return 1;
    if (g.n == 0) {
        fprintf(stderr, "Erro: nenhuma palavra valida em \"%s\".\n", arquivo);
        grafo_liberar(&g);
        return 1;
    }
    memset(&cs, 0, sizeof cs);
    if (analisar_componentes(&g, &cs) != 0 ||
        verificar_tipo_grafo(&g, &tipo) != 0) {
        fprintf(stderr, "Erro: memoria insuficiente.\n");
        liberar_componentes(&cs);
        grafo_liberar(&g);
        return 1;
    }

    if (relatorio) {
        relatorio_carga(&g, &est, arquivo, estrito);
        relatorio_componentes(&g, &cs);
        relatorio_tipo(&g, &tipo);
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
