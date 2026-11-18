#include "caminho.h"
#include "mapa.h"
#include <cmath>
#include <cstdlib>

#define TOTAL_CELULAS (MAPA_LINHAS * MAPA_COLUNAS)

// As quatro direções da grade: norte (-z), sul (+z), leste (+x), oeste (-x).
static const int DIR_LINHA[4]  = { -1, 1, 0, 0 };
static const int DIR_COLUNA[4] = { 0, 0, 1, -1 };

void celulaDoMundo(float x, float z, int *linha, int *coluna) {
    *coluna = (int) floorf(x / TAM_CELULA);
    *linha  = (int) floorf(z / TAM_CELULA);
}

void centroDaCelula(int linha, int coluna, float *x, float *z) {
    *x = (coluna + 0.5f) * TAM_CELULA;
    *z = (linha + 0.5f) * TAM_CELULA;
}

bool sorteiaVizinho(int linha, int coluna, int lEvita, int cEvita, int *lProx, int *cProx) {
    int candidatosL[4], candidatosC[4], total = 0;
    int voltaL = -1, voltaC = -1;   // a célula de onde veio, guardada como última opção

    for (int d = 0; d < 4; d++) {
        int l = linha + DIR_LINHA[d], c = coluna + DIR_COLUNA[d];
        if (celulaSolida(l, c)) continue;
        if (l == lEvita && c == cEvita) { voltaL = l; voltaC = c; continue; }
        candidatosL[total] = l;
        candidatosC[total] = c;
        total++;
    }

    if (total == 0) {                  // beco sem saída: só resta voltar
        if (voltaL < 0) return false;
        *lProx = voltaL;
        *cProx = voltaC;
        return true;
    }

    int escolhido = rand() % total;
    *lProx = candidatosL[escolhido];
    *cProx = candidatosC[escolhido];
    return true;
}

// BFS clássica: uma fila de células, um vetor "visitado", um vetor "pai" para
// reconstruir o caminho e a distância (em passos) de cada célula até a origem.
// Como a grade é pequena (19 x 20), cabe tudo em arrays estáticos.
static int fila[TOTAL_CELULAS];
static int pai[TOTAL_CELULAS];
static int passosAte[TOTAL_CELULAS];
static bool visitado[TOTAL_CELULAS];

// Roda a BFS de (lDe, cDe) até alcançar (lPara, cPara). Retorna falso se não há caminho.
static bool buscaEmLargura(int lDe, int cDe, int lPara, int cPara) {
    if (celulaSolida(lDe, cDe) || celulaSolida(lPara, cPara)) return false;

    for (int i = 0; i < TOTAL_CELULAS; i++) { visitado[i] = false; pai[i] = -1; passosAte[i] = -1; }

    int origem = lDe * MAPA_COLUNAS + cDe;
    int destino = lPara * MAPA_COLUNAS + cPara;
    int inicio = 0, fim = 0;

    fila[fim++] = origem;
    visitado[origem] = true;
    passosAte[origem] = 0;

    while (inicio < fim) {
        int atual = fila[inicio++];
        if (atual == destino) break;

        int l = atual / MAPA_COLUNAS, c = atual % MAPA_COLUNAS;
        for (int d = 0; d < 4; d++) {
            int nl = l + DIR_LINHA[d], nc = c + DIR_COLUNA[d];
            if (celulaSolida(nl, nc)) continue;   // fora do mapa também conta como parede
            int vizinho = nl * MAPA_COLUNAS + nc;
            if (visitado[vizinho]) continue;
            visitado[vizinho] = true;
            pai[vizinho] = atual;
            passosAte[vizinho] = passosAte[atual] + 1;
            fila[fim++] = vizinho;
        }
    }
    return visitado[destino];
}

int distanciaNaGrade(int lDe, int cDe, int lPara, int cPara) {
    if (!buscaEmLargura(lDe, cDe, lPara, cPara)) return -1;
    return passosAte[lPara * MAPA_COLUNAS + cPara];
}

bool proximoPasso(int lDe, int cDe, int lPara, int cPara, int *lProx, int *cProx) {
    if (lDe == lPara && cDe == cPara) return false;
    if (!buscaEmLargura(lDe, cDe, lPara, cPara)) return false;   // sem caminho

    int origem = lDe * MAPA_COLUNAS + cDe;
    int destino = lPara * MAPA_COLUNAS + cPara;

    // Volta do destino até achar a célula cujo pai é a origem: é o primeiro passo.
    int passo = destino;
    while (pai[passo] != origem) passo = pai[passo];

    *lProx = passo / MAPA_COLUNAS;
    *cProx = passo % MAPA_COLUNAS;
    return true;
}

bool haLinhaDeVisao(float x0, float z0, float x1, float z1) {
    float dx = x1 - x0, dz = z1 - z0;
    float distancia = sqrtf(dx * dx + dz * dz);
    if (distancia < 0.001f) return true;

    // Passo de um quarto de célula: nenhuma célula no caminho fica sem teste.
    float passo = TAM_CELULA * 0.25f;
    int quantos = (int) (distancia / passo) + 1;
    for (int i = 0; i <= quantos; i++) {
        float t = (float) i / quantos;
        int l, c;
        celulaDoMundo(x0 + dx * t, z0 + dz * t, &l, &c);
        if (celulaSolida(l, c)) return false;
    }
    return true;
}
