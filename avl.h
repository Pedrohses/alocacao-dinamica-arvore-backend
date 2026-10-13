#ifndef AVL_H
#define AVL_H

#include "arvore.h"

int  fatorBalanceamento(No *raiz);
No  *rotacaoDireita(No *raiz);
No  *rotacaoEsquerda(No *raiz);
No  *rebalancear(No *raiz, int *rotacoes);
No  *inserirAVL(No *raiz, int valor, int *rotacoes);
No  *removerAVL(No *raiz, int valor, int *rotacoes);

#endif
