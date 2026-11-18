#include "mapa.h"
#include "gl_includes.h"
#include "texturas.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>

// Cada texto é uma linha do labirinto, vista de cima.
// A coluna vira a coordenada x do mundo e a linha vira a coordenada z.
// # = parede, . = corredor, S = jogador, G = gato, C = gato guia, X = perseguidor, P = porta,
// F = sala da prova.
// Enfeites (sólidos como parede): A = armários; a, b, c... = porta de sala que não abre, com placa
// (o nome de cada letra está na tabela salasDecorativas, logo abaixo).
static const char *mapa[MAPA_LINHAS] = {
    "########a#####AAAA##",
    "#S...#.....#.......#",
    "#.k#.#.###.#.##b##.A",
    "#.#..#...#...#...#.A",
    "#.#.####.##c##.#.#.A",
    "#.#.C....#.....#...#",
    "#.AA#d##.#.##e####.#",
    "#......#.#.#.....#.j",
    "####.#.#...#.###.#.#",
    "l....#.##f##.#...#.#",
    "#.####.......#.###.#",
    "#.#.G..#AAA#.#.....#",
    "#.#.####...#.#AAgA.#",
    "#...#....#.#...X...#",
    "#.###.####.######P##",
    "#.....#....#FFFFFFF#",
    "###h#.#.####FFFFFFF#",
    "#..........#FFFFFFF#",
    "####AAA##i##########"
};

// Direções das faces de uma célula, na ordem em que desenhaBloco() as percorre.
enum { FACE_NORTE, FACE_SUL, FACE_OESTE, FACE_LESTE, QUANT_FACES };
static const int vizinhoLinha[QUANT_FACES]  = { -1, 1, 0, 0 };
static const int vizinhoColuna[QUANT_FACES] = { 0, 0, -1, 1 };

// Portas de sala de cenário. A posição i da tabela é a letra 'a' + i no mapa.
// A fonte do GLUT (stroke) só tem ASCII: nomes sem acento, em maiúsculas, em até duas linhas.
// "face" diz para qual lado fica a porta (o corredor); se esse lado não for livre,
// usa-se a primeira face livre na ordem norte, sul, oeste, leste. As outras faces da
// célula são parede comum, para a mesma sala não aparecer duas vezes.
struct SalaDecorativa {
    const char *linha1, *linha2;
    int face;
};

static const SalaDecorativa salasDecorativas[] = {
    { "COMPUTACAO",   "GRAFICA",      FACE_SUL   },   // a  (0, 8)
    { "ALGORITMOS",   "",             FACE_NORTE },   // b  (2, 15)
    { "ESTRUTURA",    "DE DADOS",     FACE_SUL   },   // c  (4, 11)
    { "CALCULO",      "",             FACE_NORTE },   // d  (6, 5)
    { "BANCO",        "DE DADOS",     FACE_SUL   },   // e  (6, 13)
    { "REDES DE",     "COMPUTADORES", FACE_SUL   },   // f  (9, 9)
    { "SISTEMAS",     "OPERACIONAIS", FACE_NORTE },   // g  (12, 16)
    { "COMPILADORES", "",             FACE_SUL   },   // h  (16, 3)
    { "INTELIGENCIA", "ARTIFICIAL",   FACE_NORTE },   // i  (18, 9)
    { "ENGENHARIA",   "DE SOFTWARE",  FACE_OESTE },   // j  (7, 19)
    { "COORDENACAO",  "DO CURSO",     FACE_NORTE },   // k  (2, 2), ao lado da entrada
    { "MATEMATICA",   "DISCRETA",     FACE_LESTE }    // l  (9, 0)
};
#define QUANT_SALAS_DECOR ((int) (sizeof(salasDecorativas) / sizeof(salasDecorativas[0])))

// Placa com o nome da sala, presa na porta, na altura dos olhos (JOGADOR_ALTURA = 1,5).
// Medidas no plano da face, em unidades do mundo.
#define PLACA_LARGURA  1.00f
#define PLACA_ALTURA   0.30f
#define PLACA_CENTRO_Y 1.60f
#define PLACA_MOLDURA  0.03f    // borda escura em volta da placa
// Afastamento da parede, para não brigar com ela no z-buffer (z-fighting).
#define PLACA_AFASTA_MOLDURA 0.012f
#define PLACA_AFASTA_FUNDO   0.020f
#define PLACA_AFASTA_TEXTO   0.028f
// Na GLUT_STROKE_ROMAN uma letra maiúscula tem cerca de 100 unidades de altura.
#define FONTE_PLACA            GLUT_STROKE_ROMAN
#define FONTE_ALTURA_MAIUSCULA 100.0f

// Quantos quadradinhos por lado em cada face. A iluminação do OpenGL é calculada
// nos vértices (Gouraud, aula 25); com mais vértices, a luz do jogador fica mais suave.
#define SUBDIVISOES 4

static GLuint listaMapa = 0;   // display list: a geometria é enviada uma vez e reutilizada

char celula(int linha, int coluna) {
    if (linha < 0 || linha >= MAPA_LINHAS || coluna < 0 || coluna >= MAPA_COLUNAS)
        return CEL_PAREDE;
    return mapa[linha][coluna];
}

// Verdadeiro para as letras das portas de sala de cenário.
static bool ehSalaDecorativa(char c) {
    return c >= CEL_SALA_DECOR_PRIMEIRA && c <= CEL_SALA_DECOR_ULTIMA;
}

bool celulaSolida(int linha, int coluna) {
    char c = celula(linha, coluna);
    return c == CEL_PAREDE || c == CEL_PORTA || c == CEL_ARMARIO || ehSalaDecorativa(c);
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

// Canto de partida (p0) e lado horizontal (du) de cada face de uma célula. O lado vertical é
// sempre "para cima". Com du, cima e a normal formando um sistema destro, quem olha a face de
// frente vê du apontando para a direita: a textura e o texto da placa não saem espelhados.
static void cantoDaFace(int linha, int coluna, int face, float p[3], float du[3], float n[3]) {
    float x0 = coluna * TAM_CELULA, x1 = x0 + TAM_CELULA;
    float z0 = linha * TAM_CELULA,  z1 = z0 + TAM_CELULA;
    float T = TAM_CELULA;
    p[1] = 0; du[1] = 0; n[1] = 0;
    switch (face) {
    case FACE_NORTE:   // -z
        p[0] = x1; p[2] = z0; du[0] = -T; du[2] = 0;  n[0] = 0;  n[2] = -1; break;
    case FACE_SUL:     // +z
        p[0] = x0; p[2] = z1; du[0] = T;  du[2] = 0;  n[0] = 0;  n[2] = 1;  break;
    case FACE_OESTE:   // -x
        p[0] = x0; p[2] = z0; du[0] = 0;  du[2] = T;  n[0] = -1; n[2] = 0;  break;
    default:           // FACE_LESTE, +x
        p[0] = x1; p[2] = z1; du[0] = 0;  du[2] = -T; n[0] = 1;  n[2] = 0;  break;
    }
}

// Verdadeiro se a face da célula dá para uma célula livre (e por isso pode ser vista).
static bool faceLivre(int linha, int coluna, int face) {
    return !celulaSolida(linha + vizinhoLinha[face], coluna + vizinhoColuna[face]);
}

static void desenhaFaceDaCelula(int linha, int coluna, int face) {
    float p[3], du[3], n[3], cima[3] = { 0, ALTURA_PAREDE, 0 };
    cantoDaFace(linha, coluna, face, p, du, n);
    desenhaFace(p, du, cima, n);
}

// Desenha as faces laterais de uma célula sólida. Uma face só é desenhada se a célula
// vizinha for vazia: faces entre duas paredes nunca seriam vistas.
// faceExcluida (ou -1) é pulada: numa porta de sala, essa face é desenhada à parte.
static void desenhaBloco(int linha, int coluna, int faceExcluida) {
    for (int f = 0; f < QUANT_FACES; f++)
        if (f != faceExcluida && faceLivre(linha, coluna, f))
            desenhaFaceDaCelula(linha, coluna, f);
}

// Face de uma porta de sala onde ficam a porta e a placa: a da tabela, se estiver livre;
// senão a primeira face livre. -1 se a célula estiver cercada de paredes.
static int faceDaSala(int linha, int coluna) {
    int face = salasDecorativas[celula(linha, coluna) - CEL_SALA_DECOR_PRIMEIRA].face;
    if (faceLivre(linha, coluna, face)) return face;
    for (int f = 0; f < QUANT_FACES; f++)
        if (faceLivre(linha, coluna, f)) return f;
    return -1;
}

// Retângulo no plano local da placa (x para a direita, y para cima), a uma distância z da parede.
// Anti-horário visto de frente, como as paredes, para passar pelo back-face culling.
static void retanguloPlaca(float x0, float y0, float x1, float y1, float z) {
    glBegin(GL_QUADS);
    glVertex3f(x0, y0, z);
    glVertex3f(x1, y0, z);
    glVertex3f(x1, y1, z);
    glVertex3f(x0, y1, z);
    glEnd();
}

// Largura de um texto na fonte stroke, em unidades da fonte.
static float larguraTexto(const char *texto) {
    float largura = 0;
    for (const char *p = texto; *p; p++) largura += glutStrokeWidth(FONTE_PLACA, *p);
    return largura;
}

// Escreve uma linha centrada em xCentro, com a base das letras em yBase.
// glutStrokeCharacter desenha cada letra com linhas (GL_LINE_STRIP) e anda para a direita.
static void escreveLinha(const char *texto, float xCentro, float yBase, float escala) {
    glPushMatrix();
    glTranslatef(xCentro - larguraTexto(texto) * escala / 2, yBase, PLACA_AFASTA_TEXTO);
    // z fica em 1: escalar z também mudaria o tamanho da normal e estragaria a iluminação.
    glScalef(escala, escala, 1.0f);
    for (const char *p = texto; *p; p++) glutStrokeCharacter(FONTE_PLACA, *p);
    glPopMatrix();
}

// Placa com o nome da sala, presa à frente da face escolhida. A matriz é levada para o canto
// da face e girada em torno de y; no sistema local, x corre ao longo da face (0 a TAM_CELULA),
// y sobe e z sai da parede para o corredor.
static void desenhaPlaca(int linha, int coluna, int face, const SalaDecorativa *sala) {
    static const float angulo[QUANT_FACES] = { 180.0f, 0.0f, -90.0f, 90.0f };   // N, S, O, L
    float p[3], du[3], n[3];
    cantoDaFace(linha, coluna, face, p, du, n);

    glPushMatrix();
    glTranslatef(p[0], 0.0f, p[2]);
    glRotatef(angulo[face], 0.0f, 1.0f, 0.0f);
    glNormal3f(0.0f, 0.0f, 1.0f);   // vale para os retângulos e para as linhas do texto

    float xc = TAM_CELULA / 2, yc = PLACA_CENTRO_Y;
    float meiaL = PLACA_LARGURA / 2, meiaA = PLACA_ALTURA / 2;

    GLfloat corMoldura[] = { 0.25f, 0.20f, 0.15f, 1.0f };
    GLfloat corFundo[]   = { 0.10f, 0.20f, 0.45f, 1.0f };   // azul de placa de escola
    GLfloat corLetra[]   = { 0.95f, 0.95f, 0.88f, 1.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corMoldura);
    retanguloPlaca(xc - meiaL - PLACA_MOLDURA, yc - meiaA - PLACA_MOLDURA,
                   xc + meiaL + PLACA_MOLDURA, yc + meiaA + PLACA_MOLDURA, PLACA_AFASTA_MOLDURA);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corFundo);
    retanguloPlaca(xc - meiaL, yc - meiaA, xc + meiaL, yc + meiaA, PLACA_AFASTA_FUNDO);

    // Tamanho das letras: o maior possível que caiba na placa, com margem.
    int linhas = sala->linha2[0] ? 2 : 1;
    float alturaLetra = (linhas == 1) ? 0.12f : 0.085f;
    float escala = alturaLetra / FONTE_ALTURA_MAIUSCULA;
    float maior = larguraTexto(sala->linha1);
    if (larguraTexto(sala->linha2) > maior) maior = larguraTexto(sala->linha2);
    float larguraUtil = PLACA_LARGURA - 0.08f;
    if (maior * escala > larguraUtil) escala = larguraUtil / maior;
    alturaLetra = escala * FONTE_ALTURA_MAIUSCULA;

    // Linhas centradas na vertical, com meia letra de espaço entre elas.
    float espaco = alturaLetra * 0.5f;
    float total = linhas * alturaLetra + (linhas - 1) * espaco;
    float base1 = yc + total / 2 - alturaLetra;

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corLetra);
    glLineWidth(2.0f);
    escreveLinha(sala->linha1, xc, base1, escala);
    if (linhas == 2) escreveLinha(sala->linha2, xc, base1 - alturaLetra - espaco, escala);
    glLineWidth(1.0f);

    glPopMatrix();
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

    // Cada porta de sala precisa de um nome na tabela e de uma face livre.
    static int faceSala[MAPA_LINHAS][MAPA_COLUNAS];
    for (int l = 0; l < MAPA_LINHAS; l++) {
        for (int c = 0; c < MAPA_COLUNAS; c++) {
            faceSala[l][c] = -1;
            char ch = celula(l, c);
            if (!ehSalaDecorativa(ch)) continue;
            if (ch - CEL_SALA_DECOR_PRIMEIRA >= QUANT_SALAS_DECOR) {
                printf("Erro no mapa: a sala '%c' em (%d, %d) nao tem nome na tabela (so ha %d)\n",
                       ch, l, c, QUANT_SALAS_DECOR);
                exit(1);
            }
            faceSala[l][c] = faceDaSala(l, c);
            if (faceSala[l][c] < 0)
                printf("Aviso: a sala '%c' em (%d, %d) nao da para nenhum corredor\n", ch, l, c);
        }
    }

    listaMapa = glGenLists(1);
    glNewList(listaMapa, GL_COMPILE);

    // Paredes comuns, e as faces das portas de sala que não têm a porta.
    glBindTexture(GL_TEXTURE_2D, idTexturas[TEX_PAREDE]);
    for (int l = 0; l < MAPA_LINHAS; l++) {
        for (int c = 0; c < MAPA_COLUNAS; c++) {
            if (celula(l, c) == CEL_PAREDE)         desenhaBloco(l, c, -1);
            else if (ehSalaDecorativa(celula(l, c))) desenhaBloco(l, c, faceSala[l][c]);
        }
    }

    glBindTexture(GL_TEXTURE_2D, idTexturas[TEX_PORTA]);
    for (int l = 0; l < MAPA_LINHAS; l++)
        for (int c = 0; c < MAPA_COLUNAS; c++)
            if (celula(l, c) == CEL_PORTA) desenhaBloco(l, c, -1);

    glBindTexture(GL_TEXTURE_2D, idTexturas[TEX_ARMARIO]);
    for (int l = 0; l < MAPA_LINHAS; l++)
        for (int c = 0; c < MAPA_COLUNAS; c++)
            if (celula(l, c) == CEL_ARMARIO) desenhaBloco(l, c, -1);

    glBindTexture(GL_TEXTURE_2D, idTexturas[TEX_PORTA_SALA]);
    for (int l = 0; l < MAPA_LINHAS; l++)
        for (int c = 0; c < MAPA_COLUNAS; c++)
            if (faceSala[l][c] >= 0) desenhaFaceDaCelula(l, c, faceSala[l][c]);

    // Placas: sem textura (a cor vem do material, com a mesma iluminação das paredes).
    // No fim, o estado volta ao que o resto do jogo espera: textura ligada e material branco.
    glDisable(GL_TEXTURE_2D);
    for (int l = 0; l < MAPA_LINHAS; l++)
        for (int c = 0; c < MAPA_COLUNAS; c++)
            if (faceSala[l][c] >= 0)
                desenhaPlaca(l, c, faceSala[l][c],
                             &salasDecorativas[celula(l, c) - CEL_SALA_DECOR_PRIMEIRA]);
    // Placa da porta 'P' (a sala de aula da prova), só no lado do corredor.
    static const SalaDecorativa placaProva = { "SALA DE AULA", "PROVA FINAL", FACE_NORTE };
    for (int l = 0; l < MAPA_LINHAS; l++)
        for (int c = 0; c < MAPA_COLUNAS; c++)
            if (celula(l, c) == CEL_PORTA)
                for (int f = 0; f < QUANT_FACES; f++)
                    if (faceLivre(l, c, f)
                        && celula(l + vizinhoLinha[f], c + vizinhoColuna[f]) != CEL_SALA)
                        desenhaPlaca(l, c, f, &placaProva);
    GLfloat branco[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, branco);
    glEnable(GL_TEXTURE_2D);

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
