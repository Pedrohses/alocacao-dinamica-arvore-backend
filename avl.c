#include <stdlib.h>
#include "avl.h"

// [slide:rotacoes]
int fatorBalanceamento(No *raiz) {
    return altura(raiz->esquerda) - altura(raiz->direita);
}

No *rotacaoDireita(No *raiz) {
    No *novaRaiz = raiz->esquerda;
    raiz->esquerda = novaRaiz->direita;
    novaRaiz->direita = raiz;
    return novaRaiz;
}

No *rotacaoEsquerda(No *raiz) {
    No *novaRaiz = raiz->direita;
    raiz->direita = novaRaiz->esquerda;
    novaRaiz->esquerda = raiz;
    return novaRaiz;
}
// [/slide]

// [slide:rebalancear]
No *rebalancear(No *raiz, int *rotacoes) {
    int fb = fatorBalanceamento(raiz);

    if (fb > 1) {
        if (fatorBalanceamento(raiz->esquerda) < 0) {
            raiz->esquerda = rotacaoEsquerda(raiz->esquerda);
            (*rotacoes)++;
        }
        (*rotacoes)++;
        return rotacaoDireita(raiz);
    }
    if (fb < -1) {
        if (fatorBalanceamento(raiz->direita) > 0) {
            raiz->direita = rotacaoDireita(raiz->direita);
            (*rotacoes)++;
        }
        (*rotacoes)++;
        return rotacaoEsquerda(raiz);
    }
    return raiz;
}
// [/slide]

// [slide:inserirAVL]
No *inserirAVL(No *raiz, int valor, int *rotacoes) {
    if (raiz == NULL)
        return criarNo(valor);

    if (valor < raiz->valor)
        raiz->esquerda = inserirAVL(raiz->esquerda, valor, rotacoes);
    else if (valor > raiz->valor)
        raiz->direita = inserirAVL(raiz->direita, valor, rotacoes);
    else
        return raiz;

    return rebalancear(raiz, rotacoes);
}
// [/slide]

// [slide:removerAVL]
No *removerAVL(No *raiz, int valor, int *rotacoes) {
    if (raiz == NULL)
        return NULL;

    if (valor < raiz->valor) {
        raiz->esquerda = removerAVL(raiz->esquerda, valor, rotacoes);
    } else if (valor > raiz->valor) {
        raiz->direita = removerAVL(raiz->direita, valor, rotacoes);
    } else {
        if (raiz->esquerda == NULL || raiz->direita == NULL) {
            No *filho = raiz->esquerda ? raiz->esquerda : raiz->direita;
            free(raiz);
            return filho;
        }
        No *sucessor = menorNo(raiz->direita);
        raiz->valor = sucessor->valor;
        raiz->direita = removerAVL(raiz->direita, sucessor->valor, rotacoes);
    }
    return rebalancear(raiz, rotacoes);
}
// [/slide]
