# Árvores BST e AVL em C - Backend

Árvore Binária de Busca (BST) e Árvore AVL implementadas em C com alocação dinâmica
(`malloc`/`free`), expostas por um servidor HTTP minimalista escrito também em C,
sem bibliotecas externas.

Projeto desenvolvido na disciplina de **Paradigmas de Linguagens de Programação**.
A interface que desenha as árvores fica em
[alocacao-dinamica-arvore-frontend](https://github.com/Pedrohses/alocacao-dinamica-arvore-frontend).

```
arvore.h / arvore.c   struct No e as operações da BST
avl.h / avl.c         rotações, inserção e remoção da AVL
servidor.c            servidor HTTP (main), funciona no Windows e no Linux
arvore.dev            projeto do Dev-C++ (já linka ws2_32)
Dockerfile            imagem para deploy (ex.: Render)
```

## Operações

| Operação | BST | AVL | Função em C |
|----------|:---:|:---:|-------------|
| Inserir | x | x | `inserir()` / `inserirAVL()` + `criarNo()` |
| Buscar | x | x | `buscar()` |
| Remover | x | x | `remover()` / `removerAVL()` |
| Percorrer | x | x | `preOrdem()` / `emOrdem()` / `posOrdem()` |
| Percurso em largura | x | x | `emLargura()` - usa uma fila dinâmica (lista encadeada) |
| Altura, mínimo e máximo | x | x | `altura()`, `menorNo()`, `maiorNo()` |
| Espelhar | x | | `espelhar()` |
| Liberar | x | x | `liberarArvore()` - `free` em pós-ordem |

Na AVL, inserir e remover chamam `rebalancear()`, que aplica rotações simples ou
duplas quando o fator de balanceamento de um nó chega a +2 ou -2.

## Como rodar

**Dev-C++ (Windows):** abra `arvore.dev` e pressione **F11**.

> Se abrir só os `.c` sem o projeto, adicione `-lws2_32` em
> *Ferramentas > Opções do Compilador > linha de comando do linker*.

**Linux/macOS:**

```bash
make run
```

**Docker:**

```bash
docker build -t arvore-backend .
docker run -p 8080:8080 arvore-backend
```

O console mostra cada operação e desenha a árvore "deitada" (raiz à esquerda).

### Configuração

| Variável | Padrão | Uso |
|----------|--------|-----|
| `HOST` | `127.0.0.1` | Endereço de escuta. Use `0.0.0.0` em servidores e containers. |
| `PORT` | `8080` | Porta. Também pode ser passada como argumento: `./servidor 9000`. |

## API

Todas as rotas são `GET` e respondem JSON. O parâmetro `arvore=bst|avl` escolhe a
árvore (padrão `bst`). As rotas também aceitam o prefixo `/api`.

```
/arvore                       estado atual
/inserir?valor=N              N entre -999 e 999 (na AVL informa as rotações)
/buscar?valor=N
/remover?valor=N              informa o caso: folha | um_filho | dois_filhos
/percorrer?tipo=pre|em|pos
/largura
/estatisticas                 altura, mínimo, máximo e os caminhos até eles
/espelhar                     só na BST
/liberar
/fonte?arquivo=arvore.c       código-fonte usado nos slides do frontend
```

## Deploy no Render

1. **New > Web Service**, escolha este repositório e o runtime **Docker**.
2. Em *Health Check Path*, use `/arvore`.
3. Não é preciso definir variáveis: o Render informa `PORT` e o `Dockerfile` já usa `HOST=0.0.0.0`.

As árvores ficam só na memória: são compartilhadas por todos os visitantes e
voltam vazias quando o serviço reinicia (no plano gratuito, ele dorme após ~15 min sem acesso).

## Licença

[MIT](LICENSE)
