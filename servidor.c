#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arvore.h"
#include "avl.h"

#ifdef _WIN32
    #include <winsock2.h>
    typedef SOCKET Socket;
    #define fecharSocket closesocket
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    typedef int Socket;
    #define INVALID_SOCKET (-1)
    #define fecharSocket close
#endif

#define PORTA_PADRAO 8080
#define HOST_PADRAO  "127.0.0.1"
#define VALOR_MIN   -999
#define VALOR_MAX    999

static No *raizBST = NULL;
static No *raizAVL = NULL;

/* Arvore usada pela requisicao atual (escolhida pelo parametro "arvore"). */
static No *raiz = NULL;
static int ehAVL = 0;

/* Depois de espelhar, a BST deixa de respeitar a ordem (maiores a esquerda). */
static int espelhada = 0;

typedef struct {
    char *dados;
    size_t tam;
    size_t cap;
} Texto;

static void textoIniciar(Texto *t) {
    t->cap = 256;
    t->tam = 0;
    t->dados = (char *) malloc(t->cap);
    t->dados[0] = '\0';
}

static void textoAdd(Texto *t, const char *s) {
    size_t n = strlen(s);
    while (t->tam + n + 1 > t->cap) {
        t->cap *= 2;
        t->dados = (char *) realloc(t->dados, t->cap);
    }
    memcpy(t->dados + t->tam, s, n + 1);
    t->tam += n;
}

static void textoInt(Texto *t, int v) {
    char buf[32];
    sprintf(buf, "%d", v);
    textoAdd(t, buf);
}

static void textoLiberar(Texto *t) {
    free(t->dados);
    t->dados = NULL;
}

static void arvoreJson(Texto *t, No *no) {
    if (no == NULL) {
        textoAdd(t, "null");
        return;
    }
    textoAdd(t, "{\"v\":");
    textoInt(t, no->valor);
    textoAdd(t, ",\"e\":");
    arvoreJson(t, no->esquerda);
    textoAdd(t, ",\"d\":");
    arvoreJson(t, no->direita);
    textoAdd(t, "}");
}

static void vetorJson(Texto *t, const int *v, int n) {
    int i;
    textoAdd(t, "[");
    for (i = 0; i < n; i++) {
        if (i > 0) textoAdd(t, ",");
        textoInt(t, v[i]);
    }
    textoAdd(t, "]");
}

static void estadoJson(Texto *t) {
    textoAdd(t, "\"arvore\":");
    arvoreJson(t, raiz);
    textoAdd(t, ",\"total\":");
    textoInt(t, contarNos(raiz));
    textoAdd(t, ",\"altura\":");
    textoInt(t, altura(raiz));
    textoAdd(t, espelhada && !ehAVL ? ",\"espelhada\":true" : ",\"espelhada\":false");
    textoAdd(t, ehAVL ? ",\"tipo\":\"avl\"" : ",\"tipo\":\"bst\"");
}

static void caminhoExtremoJson(Texto *t, No *no, int paraEsquerda) {
    int primeiro = 1;
    textoAdd(t, "[");
    while (no != NULL) {
        if (!primeiro) textoAdd(t, ",");
        textoInt(t, no->valor);
        primeiro = 0;
        no = paraEsquerda ? no->esquerda : no->direita;
    }
    textoAdd(t, "]");
}

static int bloqueadaSeEspelhada(Texto *r) {
    if (ehAVL || !espelhada) return 0;
    textoAdd(r, "{\"erro\":\"A arvore esta espelhada e nao e mais uma BST. Espelhe de novo para voltar.\"}");
    printf("[BLOQUEADO] arvore espelhada\n\n");
    return 1;
}

static void caminhoJson(Texto *t, No *no, int valor) {
    int primeiro = 1;
    textoAdd(t, "[");
    while (no != NULL) {
        if (!primeiro) textoAdd(t, ",");
        textoInt(t, no->valor);
        primeiro = 0;
        if (valor == no->valor) break;
        no = (valor < no->valor) ? no->esquerda : no->direita;
    }
    textoAdd(t, "]");
}

static int lerParametro(const char *query, const char *nome, char *saida, int max) {
    char chave[32];
    const char *p;
    int i = 0;

    if (query == NULL) return 0;
    sprintf(chave, "%s=", nome);
    p = strstr(query, chave);
    if (p == NULL) return 0;
    p += strlen(chave);
    while (*p && *p != '&' && i < max - 1)
        saida[i++] = *p++;
    saida[i] = '\0';
    return i > 0;
}

static int lerValor(const char *query, int *valor) {
    char buf[16];
    char *fim;
    long v;
    if (!lerParametro(query, "valor", buf, sizeof(buf))) return 0;
    v = strtol(buf, &fim, 10);
    if (*fim != '\0' || v < VALOR_MIN || v > VALOR_MAX) return 0;
    *valor = (int) v;
    return 1;
}

static void mostrarNoConsole(void) {
    printf("  Arvore (deitada, raiz a esquerda):\n");
    if (raiz == NULL) printf("    (vazia)\n");
    else imprimirArvore(raiz, 1);
    printf("  Nos: %d | Altura: %d\n\n", contarNos(raiz), altura(raiz));
}

static int rotaInserir(const char *query, Texto *r) {
    int valor, duplicado, rotacoes = 0;
    if (bloqueadaSeEspelhada(r)) return 409;
    if (!lerValor(query, &valor)) return 400;

    duplicado = buscar(raiz, valor) != NULL;
    textoAdd(r, "{\"valor\":");
    textoInt(r, valor);
    textoAdd(r, ",\"caminho\":");
    caminhoJson(r, raiz, valor);

    if (ehAVL) raiz = inserirAVL(raiz, valor, &rotacoes);
    else       raiz = inserir(raiz, valor);

    textoAdd(r, ",\"rotacoes\":");
    textoInt(r, rotacoes);
    textoAdd(r, duplicado ? ",\"ok\":false," : ",\"ok\":true,");
    estadoJson(r);
    textoAdd(r, "}");

    printf("[INSERIR] %d %s", valor, duplicado ? "-> ja existe" : "-> inserido");
    if (rotacoes) printf(" (%d rotacao(oes))", rotacoes);
    printf("\n");
    mostrarNoConsole();
    return 200;
}

static int rotaBuscar(const char *query, Texto *r) {
    int valor;
    No *achado;
    if (bloqueadaSeEspelhada(r)) return 409;
    if (!lerValor(query, &valor)) return 400;

    achado = buscar(raiz, valor);

    textoAdd(r, "{\"valor\":");
    textoInt(r, valor);
    textoAdd(r, achado ? ",\"encontrado\":true" : ",\"encontrado\":false");
    textoAdd(r, ",\"caminho\":");
    caminhoJson(r, raiz, valor);
    textoAdd(r, ",");
    estadoJson(r);
    textoAdd(r, "}");

    printf("[BUSCAR] %d -> %s\n\n", valor, achado ? "encontrado" : "nao encontrado");
    return 200;
}

static int rotaRemover(const char *query, Texto *r) {
    int valor, rotacoes = 0;
    const char *caso;
    No *alvo, *sucessor = NULL;
    if (bloqueadaSeEspelhada(r)) return 409;
    if (!lerValor(query, &valor)) return 400;

    alvo = buscar(raiz, valor);
    if (alvo == NULL)                         caso = "inexistente";
    else if (!alvo->esquerda && !alvo->direita) caso = "folha";
    else if (!alvo->esquerda || !alvo->direita) caso = "um_filho";
    else {
        caso = "dois_filhos";
        sucessor = menorNo(alvo->direita);
    }

    textoAdd(r, "{\"valor\":");
    textoInt(r, valor);
    textoAdd(r, ",\"caso\":\"");
    textoAdd(r, caso);
    textoAdd(r, "\",\"caminho\":");
    caminhoJson(r, raiz, valor);
    textoAdd(r, ",\"sucessor\":");
    if (sucessor) {
        textoInt(r, sucessor->valor);
        textoAdd(r, ",\"caminhoSucessor\":");
        caminhoJson(r, alvo->direita, sucessor->valor);
    } else {
        textoAdd(r, "null,\"caminhoSucessor\":[]");
    }

    if (ehAVL) raiz = removerAVL(raiz, valor, &rotacoes);
    else       raiz = remover(raiz, valor);

    textoAdd(r, ",\"rotacoes\":");
    textoInt(r, rotacoes);
    textoAdd(r, alvo ? ",\"ok\":true," : ",\"ok\":false,");
    estadoJson(r);
    textoAdd(r, "}");

    printf("[REMOVER] %d -> caso: %s", valor, caso);
    if (rotacoes) printf(" (%d rotacao(oes))", rotacoes);
    printf("\n");
    mostrarNoConsole();
    return 200;
}

static int rotaPercorrer(const char *query, Texto *r) {
    char tipo[8];
    int n = 0, total = contarNos(raiz);
    int *vetor;

    if (!lerParametro(query, "tipo", tipo, sizeof(tipo))) strcpy(tipo, "em");

    vetor = (int *) malloc(sizeof(int) * (total > 0 ? total : 1));

    if (strcmp(tipo, "pre") == 0)       preOrdem(raiz, vetor, &n);
    else if (strcmp(tipo, "pos") == 0)  posOrdem(raiz, vetor, &n);
    else { strcpy(tipo, "em");          emOrdem(raiz, vetor, &n); }

    textoAdd(r, "{\"tipo\":\"");
    textoAdd(r, tipo);
    textoAdd(r, "\",\"sequencia\":");
    vetorJson(r, vetor, n);
    textoAdd(r, ",");
    estadoJson(r);
    textoAdd(r, "}");

    {
        int i;
        printf("[PERCORRER] %s-ordem:", tipo);
        for (i = 0; i < n; i++) printf(" %d", vetor[i]);
        printf("\n\n");
    }

    free(vetor);
    return 200;
}

static int rotaLiberar(Texto *r) {
    int n = 0, liberados, total = contarNos(raiz);
    int *ordem = (int *) malloc(sizeof(int) * (total > 0 ? total : 1));

    posOrdem(raiz, ordem, &n);

    printf("[LIBERAR] liberando a arvore (pos-ordem):\n");
    liberados = liberarArvore(raiz);
    raiz = NULL;
    if (!ehAVL) espelhada = 0;
    printf("  %d no(s) liberado(s)\n\n", liberados);

    textoAdd(r, "{\"liberados\":");
    textoInt(r, liberados);
    textoAdd(r, ",\"ordem\":");
    vetorJson(r, ordem, n);
    textoAdd(r, ",");
    estadoJson(r);
    textoAdd(r, "}");

    free(ordem);
    return 200;
}

static int rotaEspelhar(Texto *r) {
    if (ehAVL) {
        textoAdd(r, "{\"erro\":\"Espelhar nao se aplica a AVL: quebraria a ordem da arvore.\"}");
        return 409;
    }
    espelhar(raiz);
    if (raiz != NULL) espelhada = !espelhada;

    textoAdd(r, "{");
    estadoJson(r);
    textoAdd(r, "}");

    printf("[ESPELHAR] %s\n", espelhada ? "espelhada (nao e mais BST)" : "voltou ao normal");
    mostrarNoConsole();
    return 200;
}

static int rotaLargura(Texto *r) {
    int n = 0, total = contarNos(raiz), i;
    int *vetor = (int *) malloc(sizeof(int) * (total > 0 ? total : 1));

    emLargura(raiz, vetor, &n);

    textoAdd(r, "{\"sequencia\":");
    vetorJson(r, vetor, n);
    textoAdd(r, ",");
    estadoJson(r);
    textoAdd(r, "}");

    printf("[LARGURA] por nivel:");
    for (i = 0; i < n; i++) printf(" %d", vetor[i]);
    printf("\n\n");

    free(vetor);
    return 200;
}

static int rotaEstatisticas(Texto *r) {
    No *menor, *maior;
    if (bloqueadaSeEspelhada(r)) return 409;

    menor = menorNo(raiz);
    maior = maiorNo(raiz);

    textoAdd(r, "{\"minimo\":");
    if (menor) textoInt(r, menor->valor); else textoAdd(r, "null");
    textoAdd(r, ",\"maximo\":");
    if (maior) textoInt(r, maior->valor); else textoAdd(r, "null");
    textoAdd(r, ",\"caminhoMinimo\":");
    caminhoExtremoJson(r, raiz, 1);
    textoAdd(r, ",\"caminhoMaximo\":");
    caminhoExtremoJson(r, raiz, 0);
    textoAdd(r, ",");
    estadoJson(r);
    textoAdd(r, "}");

    if (raiz)
        printf("[ESTATISTICAS] altura %d | minimo %d | maximo %d\n\n",
               altura(raiz), menor->valor, maior->valor);
    else
        printf("[ESTATISTICAS] arvore vazia\n\n");
    return 200;
}

static int executarRota(const char *caminho, const char *query, Texto *r) {
    if (strcmp(caminho, "/arvore") == 0) {
        textoAdd(r, "{");
        estadoJson(r);
        textoAdd(r, "}");
        return 200;
    }
    if (strcmp(caminho, "/inserir") == 0)   return rotaInserir(query, r);
    if (strcmp(caminho, "/buscar") == 0)    return rotaBuscar(query, r);
    if (strcmp(caminho, "/remover") == 0)   return rotaRemover(query, r);
    if (strcmp(caminho, "/percorrer") == 0) return rotaPercorrer(query, r);
    if (strcmp(caminho, "/liberar") == 0)   return rotaLiberar(r);
    if (strcmp(caminho, "/espelhar") == 0)  return rotaEspelhar(r);
    if (strcmp(caminho, "/largura") == 0)   return rotaLargura(r);
    if (strcmp(caminho, "/estatisticas") == 0) return rotaEstatisticas(r);
    return 404;
}

static int rotear(const char *caminho, const char *query, Texto *r) {
    char tipo[8];
    int status;

    if (strncmp(caminho, "/api", 4) == 0) caminho += 4;

    ehAVL = lerParametro(query, "arvore", tipo, sizeof(tipo)) && strcmp(tipo, "avl") == 0;
    raiz = ehAVL ? raizAVL : raizBST;

    if (strcmp(caminho, "/arvore") != 0) printf("%s ", ehAVL ? "AVL" : "BST");
    status = executarRota(caminho, query, r);

    if (ehAVL) raizAVL = raiz;
    else       raizBST = raiz;
    return status;
}

static void enviarTudo(Socket s, const char *dados, size_t n) {
    while (n > 0) {
        int enviado = send(s, dados, (int) n, 0);
        if (enviado <= 0) return;
        dados += enviado;
        n -= (size_t) enviado;
    }
}

static void responderComTipo(Socket cliente, int status, const char *tipo, const char *corpo) {
    char cab[512];
    const char *texto = status == 200 ? "OK"
                      : status == 204 ? "No Content"
                      : status == 400 ? "Bad Request"
                      : status == 409 ? "Conflict" : "Not Found";
    size_t n = corpo ? strlen(corpo) : 0;

    sprintf(cab,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s; charset=utf-8\r\n"
        "Content-Length: %lu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: *\r\n"
        "Connection: close\r\n\r\n",
        status, texto, tipo, (unsigned long) n);

    enviarTudo(cliente, cab, strlen(cab));
    if (n > 0) enviarTudo(cliente, corpo, n);
}

static void responder(Socket cliente, int status, const char *corpo) {
    responderComTipo(cliente, status, "application/json", corpo);
}

/* Envia o codigo-fonte para os slides do frontend (so arquivos da lista). */
static void enviarFonte(Socket cliente, const char *query) {
    const char *permitidos[] = { "arvore.c", "arvore.h", "avl.c", "avl.h" };
    char nome[32], buf[1024];
    size_t i, lidos;
    FILE *arq = NULL;
    Texto corpo;

    if (lerParametro(query, "arquivo", nome, sizeof(nome)))
        for (i = 0; i < sizeof(permitidos) / sizeof(permitidos[0]); i++)
            if (strcmp(nome, permitidos[i]) == 0) arq = fopen(nome, "rb");

    if (arq == NULL) {
        responder(cliente, 404, "{\"erro\":\"arquivo nao encontrado\"}");
        return;
    }
    textoIniciar(&corpo);
    while ((lidos = fread(buf, 1, sizeof(buf) - 1, arq)) > 0) {
        buf[lidos] = '\0';
        textoAdd(&corpo, buf);
    }
    fclose(arq);
    responderComTipo(cliente, 200, "text/plain", corpo.dados);
    textoLiberar(&corpo);
}

static void atenderCliente(Socket cliente) {
    char req[4096];
    char metodo[8], alvo[512];
    char *query;
    int lido = 0, n, status;
    Texto corpo;

    while (lido < (int) sizeof(req) - 1) {
        n = recv(cliente, req + lido, sizeof(req) - 1 - lido, 0);
        if (n <= 0) break;
        lido += n;
        req[lido] = '\0';
        if (strstr(req, "\r\n\r\n")) break;
    }
    if (lido <= 0) return;
    req[lido] = '\0';

    if (sscanf(req, "%7s %511s", metodo, alvo) != 2) {
        responder(cliente, 400, "{\"erro\":\"requisicao invalida\"}");
        return;
    }
    if (strcmp(metodo, "OPTIONS") == 0) {
        responder(cliente, 204, NULL);
        return;
    }

    query = strchr(alvo, '?');
    if (query) *query++ = '\0';

    if (strcmp(alvo, "/fonte") == 0 || strcmp(alvo, "/api/fonte") == 0) {
        enviarFonte(cliente, query);
        return;
    }

    textoIniciar(&corpo);
    status = rotear(alvo, query, &corpo);
    if (status == 400)
        responder(cliente, 400, "{\"erro\":\"valor invalido (use um inteiro entre -999 e 999)\"}");
    else if (status == 404)
        responder(cliente, 404, "{\"erro\":\"rota nao encontrada\"}");
    else
        responder(cliente, status, corpo.dados);
    textoLiberar(&corpo);
}

int main(int argc, char *argv[]) {
    const char *host = getenv("HOST") ? getenv("HOST") : HOST_PADRAO;
    int porta = argc > 1 ? atoi(argv[1])
              : getenv("PORT") ? atoi(getenv("PORT"))
              : PORTA_PADRAO;
    Socket servidor;
    struct sockaddr_in endereco;
    int opcao = 1;

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Erro ao iniciar Winsock.\n");
        return 1;
    }
#endif

    servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor == INVALID_SOCKET) {
        printf("Erro ao criar socket.\n");
        return 1;
    }
    setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, (const char *) &opcao, sizeof(opcao));

    memset(&endereco, 0, sizeof(endereco));
    endereco.sin_family = AF_INET;
    endereco.sin_addr.s_addr = inet_addr(host);
    endereco.sin_port = htons((unsigned short) porta);

    if (bind(servidor, (struct sockaddr *) &endereco, sizeof(endereco)) != 0) {
        printf("Erro: nao foi possivel usar %s:%d (porta em uso?)\n", host, porta);
        return 1;
    }
    listen(servidor, 16);

    printf("==============================================\n");
    printf("  Arvores BST e AVL - servidor em C\n");
    printf("  Ouvindo em http://%s:%d\n", host, porta);
    printf("  Abra o frontend (npm run dev) no navegador.\n");
    printf("==============================================\n\n");
    fflush(stdout);

    for (;;) {
        Socket cliente = accept(servidor, NULL, NULL);
        if (cliente == INVALID_SOCKET) continue;
        atenderCliente(cliente);
        fecharSocket(cliente);
        fflush(stdout);
    }

    liberarArvore(raizBST);
    liberarArvore(raizAVL);
    fecharSocket(servidor);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
