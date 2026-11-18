// Navegação na grade do labirinto, compartilhada por todos os NPCs:
// conversão célula <-> mundo, sorteio de vizinho (vagar), busca em largura (perseguir)
// e linha de visão. Só arrays estáticos, sem STL.
#ifndef CAMINHO_H
#define CAMINHO_H

// Célula (linha, coluna) que contém o ponto (x, z) do mundo.
void celulaDoMundo(float x, float z, int *linha, int *coluna);

// Centro da célula (linha, coluna), em coordenadas do mundo.
void centroDaCelula(int linha, int coluna, float *x, float *z);

// Sorteia uma das quatro direções da grade (N, S, L, O) a partir de (linha, coluna),
// só entre as que dão em célula livre. Evita voltar para (lEvita, cEvita), a não ser
// que seja a única saída (beco). É o "estado 1" do fantasma do Pacman da aula 21.
// Retorna falso se não há vizinho livre.
bool sorteiaVizinho(int linha, int coluna, int lEvita, int cEvita, int *lProx, int *cProx);

// Busca em largura (BFS) de (lDe, cDe) até (lPara, cPara) pelas células livres.
// Devolve em (lProx, cProx) a primeira célula do caminho mais curto.
// Retorna falso se já está no destino ou se não há caminho.
bool proximoPasso(int lDe, int cDe, int lPara, int cPara, int *lProx, int *cProx);

// Número de passos (células) do caminho mais curto de (lDe, cDe) até (lPara, cPara),
// pela mesma BFS. 0 se é a mesma célula; -1 se não há caminho.
int distanciaNaGrade(int lDe, int cDe, int lPara, int cPara);

// Verdadeiro se o segmento de (x0, z0) a (x1, z1) não cruza nenhuma célula sólida.
// Anda ao longo do segmento em passos menores que uma célula, testando cada célula.
bool haLinhaDeVisao(float x0, float z0, float x1, float z1);

#endif
