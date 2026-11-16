#include "mapa.h"
#include "gl_includes.h"
#include "texturas.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>

// Cada texto é uma linha do labirinto, vista de cima.
// A coluna vira a coordenada x do mundo e a linha vira a coordenada z.
static const char *mapa[MAPA_LINHAS] = {
    "####################",
    "#S...#.....#.......#",
    "#.##.#.###.#.#####.#",
    "#.#..#...#...#...#.#",
    "#.#.####.#####.#.#.#",
    "#.#......#.....#...#",
    "#.######.#.#######.#",
    "#......#.#.#.....#.#",
    "####.#.#...#.###.#.#",
    "#....#.#####.#...#.#",
    "#.####.......#.###.#",
    "#.#....#####.#.....#",
    "#.#.####...#.#####.#",
    "#...#....#.#.......#",
    "#.###.####.######P##",
    "#.....#....#FFFFFFF#",
    "#####.#.####FFFFFFF#",
    "#..........#FFFFFFF#",
    "####################"
};

// Quantos quadradinhos por lado em cada face. A iluminação do OpenGL é calculada
// nos vértices (Gouraud, aula 25); com mais vértices, a luz do jogador fica mais suave.
#define SUBDIVISOES 4

static GLuint listaMapa = 0;   // display list: a geometria é enviada uma vez e reutilizada

char celula(int linha, int coluna) {
    if (linha < 0 || linha >= MAPA_LINHAS || coluna < 0 || coluna >= MAPA_COLUNAS)
        return CEL_PAREDE;
    return mapa[linha][coluna];
}

bool celulaSolida(int linha, int coluna) {
    char c = celula(linha, coluna);
    return c == CEL_PAREDE || c == CEL_PORTA;
}

// Desenha um retângulo subdividido, com normal e coordenadas de textura.
// p0 é um canto; du e dv são os dois lados. Os vértices saem em sentido anti-horário
// para quem olha de frente (lado da normal), o que permite usar back-face culling.
static void desenhaFace(const float p0[3], const float du[3], const float dv[3],
                        const float normal[3]) {
    glNormal3fv(normal);
    glBegin(GL_QUADS);
    for (int i = 0; i < SUBDIVISOES; i++) {
        for (int j = 0; j < SUBDIVISOES; j++) {
            float u0 = (float) i / SUBDIVISOES, u1 = (float) (i + 1) / SUBDIVISOES;
            float v0 = (float) j / SUBDIVISOES, v1 = (float) (j + 1) / SUBDIVISOES;
            float cantos[4][2] = { {u0, v0}, {u1, v0}, {u1, v1}, {u0, v1} };
            for (int k = 0; k < 4; k++) {
                float u = cantos[k][0], v = cantos[k][1];
                glTexCoord2f(u, v);
                glVertex3f(p0[0] + u * du[0] + v * dv[0],
                           p0[1] + u * du[1] + v * dv[1],
                           p0[2] + u * du[2] + v * dv[2]);
            }
        }
    }
    glEnd();
}

// Desenha as faces laterais de uma célula sólida. Uma face só é desenhada se a célula
// vizinha for vazia: faces entre duas paredes nunca seriam vistas.
static void desenhaBloco(int linha, int coluna) {
    float x0 = coluna * TAM_CELULA, x1 = x0 + TAM_CELULA;
    float z0 = linha * TAM_CELULA,  z1 = z0 + TAM_CELULA;
    float T = TAM_CELULA, H = ALTURA_PAREDE;
    float cima[3] = { 0, H, 0 };

    if (!celulaSolida(linha - 1, coluna)) {          // face norte (-z)
        float p[3] = { x1, 0, z0 }, du[3] = { -T, 0, 0 }, n[3] = { 0, 0, -1 };
        desenhaFace(p, du, cima, n);
    }
    if (!celulaSolida(linha + 1, coluna)) {          // face sul (+z)
        float p[3] = { x0, 0, z1 }, du[3] = { T, 0, 0 }, n[3] = { 0, 0, 1 };
        desenhaFace(p, du, cima, n);
    }
    if (!celulaSolida(linha, coluna - 1)) {          // face oeste (-x)
        float p[3] = { x0, 0, z0 }, du[3] = { 0, 0, T }, n[3] = { -1, 0, 0 };
        desenhaFace(p, du, cima, n);
    }
    if (!celulaSolida(linha, coluna + 1)) {          // face leste (+x)
        float p[3] = { x1, 0, z1 }, du[3] = { 0, 0, -T }, n[3] = { 1, 0, 0 };
        desenhaFace(p, du, cima, n);
    }
}

static void desenhaPiso(int linha, int coluna) {
    float x0 = coluna * TAM_CELULA, z1 = (linha + 1) * TAM_CELULA, T = TAM_CELULA;
    float p[3] = { x0, 0, z1 }, du[3] = { T, 0, 0 }, dv[3] = { 0, 0, -T }, n[3] = { 0, 1, 0 };
    desenhaFace(p, du, dv, n);
}

static void desenhaTeto(int linha, int coluna) {
    float x0 = coluna * TAM_CELULA, z0 = linha * TAM_CELULA, T = TAM_CELULA;
    float p[3] = { x0, ALTURA_PAREDE, z0 }, du[3] = { T, 0, 0 }, dv[3] = { 0, 0, T }, n[3] = { 0, -1, 0 };
    desenhaFace(p, du, dv, n);
}

void iniciaMapa() {
    // Confere se todas as linhas têm o tamanho certo (erro fácil de cometer ao editar o mapa).
    for (int l = 0; l < MAPA_LINHAS; l++) {
        if ((int) strlen(mapa[l]) != MAPA_COLUNAS) {
            printf("Erro no mapa: a linha %d tem %d colunas (esperado %d)\n",
                   l, (int) strlen(mapa[l]), MAPA_COLUNAS);
            exit(1);
        }
    }

    listaMapa = glGenLists(1);
    glNewList(listaMapa, GL_COMPILE);

    glBindTexture(GL_TEXTURE_2D, idTexturas[TEX_PAREDE]);
    for (int l = 0; l < MAPA_LINHAS; l++)
        for (int c = 0; c < MAPA_COLUNAS; c++)
            if (celula(l, c) == CEL_PAREDE) desenhaBloco(l, c);

    glBindTexture(GL_TEXTURE_2D, idTexturas[TEX_PORTA]);
    for (int l = 0; l < MAPA_LINHAS; l++)
        for (int c = 0; c < MAPA_COLUNAS; c++)
            if (celula(l, c) == CEL_PORTA) desenhaBloco(l, c);

    glBindTexture(GL_TEXTURE_2D, idTexturas[TEX_PISO]);
    for (int l = 0; l < MAPA_LINHAS; l++)
        for (int c = 0; c < MAPA_COLUNAS; c++)
            if (!celulaSolida(l, c)) desenhaPiso(l, c);

    glBindTexture(GL_TEXTURE_2D, idTexturas[TEX_TETO]);
    for (int l = 0; l < MAPA_LINHAS; l++)
        for (int c = 0; c < MAPA_COLUNAS; c++)
            if (!celulaSolida(l, c)) desenhaTeto(l, c);

    glEndList();
}

void desenhaMapa() {
    glCallList(listaMapa);
}

void posicaoInicial(float *x, float *z) {
    for (int l = 0; l < MAPA_LINHAS; l++) {
        for (int c = 0; c < MAPA_COLUNAS; c++) {
            if (celula(l, c) == CEL_INICIO) {
                *x = (c + 0.5f) * TAM_CELULA;
                *z = (l + 0.5f) * TAM_CELULA;
                return;
            }
        }
    }
    *x = *z = 1.5f * TAM_CELULA;   // mapa sem 'S': usa a célula (1, 1)
}

// Colisão círculo x AABB. Cada célula sólida é uma caixa alinhada aos eixos (aula 21).
// Para cada caixa perto do círculo, acha o ponto da caixa mais próximo do centro;
// se a distância até esse ponto for menor que o raio, há colisão.
bool colideComMapa(float x, float z, float raio) {
    int colMin = (int) floorf((x - raio) / TAM_CELULA);
    int colMax = (int) floorf((x + raio) / TAM_CELULA);
    int linMin = (int) floorf((z - raio) / TAM_CELULA);
    int linMax = (int) floorf((z + raio) / TAM_CELULA);

    for (int l = linMin; l <= linMax; l++) {
        for (int c = colMin; c <= colMax; c++) {
            if (!celulaSolida(l, c)) continue;

            float x0 = c * TAM_CELULA, x1 = x0 + TAM_CELULA;
            float z0 = l * TAM_CELULA, z1 = z0 + TAM_CELULA;

            float px = x < x0 ? x0 : (x > x1 ? x1 : x);   // ponto mais próximo em x
            float pz = z < z0 ? z0 : (z > z1 ? z1 : z);   // ponto mais próximo em z

            float dx = x - px, dz = z - pz;
            if (dx * dx + dz * dz < raio * raio) return true;
        }
    }
    return false;
}
