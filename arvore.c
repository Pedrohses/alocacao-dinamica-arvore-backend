#include <stdio.h>
#include <stdlib.h>
#include "arvore.h"

// Os marcadores [slide:...] delimitam os trechos exibidos nos slides do frontend.

// [slide:criarNo]
No *criarNo(int valor) {
    No *novo = (No *) malloc(sizeof(No));
    if (novo == NULL) {
        printf("Erro: memoria insuficiente!\n");
        exit(1);
    }
    novo->valor    = valor;
    novo->esquerda = NULL;
    novo->direita  = NULL;
    return novo;
}
// [/slide]

// [slide:inserir]
No *inserir(No *raiz, int valor) {
    if (raiz == NULL)
        return criarNo(valor);

    if (valor < raiz->valor)
        raiz->esquerda = inserir(raiz->esquerda, valor);
    else if (valor > raiz->valor)
        raiz->direita = inserir(raiz->direita, valor);

    return raiz;
}
// [/slide]

// [slide:buscar]
No *buscar(No *raiz, int valor) {
    if (raiz == NULL || raiz->valor == valor)
        return raiz;

    if (valor < raiz->valor)
        return buscar(raiz->esquerda, valor);
    else
        return buscar(raiz->direita, valor);
}
// [/slide]

// [slide:menorNo]
No *menorNo(No *raiz) {
    No *atual = raiz;
    while (atual != NULL && atual->esquerda != NULL)
        atual = atual->esquerda;
    return atual;
}
// [/slide]

// [slide:remover]
No *remover(No *raiz, int valor) {
    if (raiz == NULL)
        return NULL;

    if (valor < raiz->valor) {
        raiz->esquerda = remover(raiz->esquerda, valor);
    } else if (valor > raiz->valor) {
        raiz->direita = remover(raiz->direita, valor);
    } else {
        if (raiz->esquerda == NULL) {
            No *filho = raiz->direita;
            free(raiz);
            return filho;
        }
        if (raiz->direita == NULL) {
            No *filho = raiz->esquerda;
            free(raiz);
            return filho;
        }
        No *sucessor = menorNo(raiz->direita);
        raiz->valor = sucessor->valor;
        raiz->direita = remover(raiz->direita, sucessor->valor);
    }
    return raiz;
}
// [/slide]

// [slide:percorrer]
void preOrdem(No *raiz, int *vetor, int *n) {
    if (raiz == NULL) return;
    vetor[(*n)++] = raiz->valor;
    preOrdem(raiz->esquerda, vetor, n);
    preOrdem(raiz->direita, vetor, n);
}

void emOrdem(No *raiz, int *vetor, int *n) {
    if (raiz == NULL) return;
    emOrdem(raiz->esquerda, vetor, n);
    vetor[(*n)++] = raiz->valor;
    emOrdem(raiz->direita, vetor, n);
}

void posOrdem(No *raiz, int *vetor, int *n) {
    if (raiz == NULL) return;
    posOrdem(raiz->esquerda, vetor, n);
    posOrdem(raiz->direita, vetor, n);
    vetor[(*n)++] = raiz->valor;
}
// [/slide]

// [slide:liberar]
int liberarArvore(No *raiz) {
    if (raiz == NULL)
        return 0;

    int total = 0;
    total += liberarArvore(raiz->esquerda);
    total += liberarArvore(raiz->direita);

    printf("  free(%d)\n", raiz->valor);
    free(raiz);
    return total + 1;
}
// [/slide]

// [slide:espelhar]
void espelhar(No *raiz) {
    if (raiz == NULL) return;

    No *temp = raiz->esquerda;
    raiz->esquerda = raiz->direita;
    raiz->direita = temp;

    espelhar(raiz->esquerda);
    espelhar(raiz->direita);
}
// [/slide]

// [slide:fila]
typedef struct ItemFila {
    No *no;
    struct ItemFila *prox;
} ItemFila;

typedef struct {
    ItemFila *inicio;
    ItemFila *fim;
} Fila;

static void enfileirar(Fila *f, No *no) {
    ItemFila *item = (ItemFila *) malloc(sizeof(ItemFila));
    item->no = no;
    item->prox = NULL;
    if (f->fim == NULL) f->inicio = item;
    else f->fim->prox = item;
    f->fim = item;
}

static No *desenfileirar(Fila *f) {
    ItemFila *item = f->inicio;
    No *no = item->no;
    f->inicio = item->prox;
    if (f->inicio == NULL) f->fim = NULL;
    free(item);
    return no;
}
// [/slide]

// [slide:largura]
void emLargura(No *raiz, int *vetor, int *n) {
    Fila fila = { NULL, NULL };
    if (raiz == NULL) return;

    enfileirar(&fila, raiz);
    while (fila.inicio != NULL) {
        No *atual = desenfileirar(&fila);
        vetor[(*n)++] = atual->valor;
        if (atual->esquerda) enfileirar(&fila, atual->esquerda);
        if (atual->direita)  enfileirar(&fila, atual->direita);
    }
}
// [/slide]

// [slide:altura]
int altura(No *raiz) {
    if (raiz == NULL) return 0;
    int he = altura(raiz->esquerda);
    int hd = altura(raiz->direita);
    return 1 + (he > hd ? he : hd);
}
// [/slide]

// [slide:maiorNo]
No *maiorNo(No *raiz) {
    No *atual = raiz;
    while (atual != NULL && atual->direita != NULL)
        atual = atual->direita;
    return atual;
}
// [/slide]

int contarNos(No *raiz) {
    if (raiz == NULL) return 0;
    return 1 + contarNos(raiz->esquerda) + contarNos(raiz->direita);
}

void imprimirArvore(No *raiz, int nivel) {
    int i;
    if (raiz == NULL) return;
    imprimirArvore(raiz->direita, nivel + 1);
    for (i = 0; i < nivel; i++) printf("      ");
    printf("%d\n", raiz->valor);
    imprimirArvore(raiz->esquerda, nivel + 1);
}
