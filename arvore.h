#ifndef ARVORE_H
#define ARVORE_H

typedef struct No {
    int valor;
    struct No *esquerda;
    struct No *direita;
} No;

No  *inserir(No *raiz, int valor);
No  *buscar(No *raiz, int valor);
No  *remover(No *raiz, int valor);
void preOrdem(No *raiz, int *vetor, int *n);
void emOrdem(No *raiz, int *vetor, int *n);
void posOrdem(No *raiz, int *vetor, int *n);
int  liberarArvore(No *raiz);

void espelhar(No *raiz);
void emLargura(No *raiz, int *vetor, int *n);
No  *maiorNo(No *raiz);

No  *criarNo(int valor);
No  *menorNo(No *raiz);
int  contarNos(No *raiz);
int  altura(No *raiz);
void imprimirArvore(No *raiz, int nivel);

#endif
