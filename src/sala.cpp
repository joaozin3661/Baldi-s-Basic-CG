#include "sala.h"
#include "mapa.h"
#include "caminho.h"
#include "jogo.h"
#include "jogador.h"
#include "perseguidor.h"
#include "texturas.h"
#include "gl_includes.h"
#include <cmath>
#include <cstdio>

// A sala é o retângulo formado pelas células 'F' do mapa. O quadro fica na parede oeste (-x)
// e as carteiras olham para ele; as posições saem das medidas da sala, então mudar o tamanho
// das 'F' no mapa rearranja os móveis sozinho (com a porta 'P' na parede norte ou sul).

#define SALA_ALCANCE_PORTA  1.1f   // distância máxima do jogador à célula da porta para usar E
#define SALA_TEMPO_PASSAGEM 0.6f   // segundos de escurecimento depois de passar pela porta
#define OLHA_PARA_OESTE    -1.5708f   // yaw de quem olha para -x (o quadro)

#define MAX_MOVEIS 16

// Caixa de colisão de um móvel, vista de cima (só x e z: o jogador não pula).
struct Caixa2D { float x0, z0, x1, z1; };

static bool temSala = false;
static int lPorta, cPorta;      // célula 'P'
static int lFora, cFora;        // corredor na frente da porta
static int lDentro, cDentro;    // célula da sala logo atrás da porta
static float salaX0, salaZ0, salaX1, salaZ1;   // retângulo da sala, em coordenadas do mundo

static Caixa2D moveis[MAX_MOVEIS];
static int quantMoveis = 0;

static GLuint listaSala = 0;
static float tempoPassagem = 0.0f;

static const GLfloat corBranca[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static const GLfloat corMetal[]  = { 0.32f, 0.34f, 0.38f, 1.0f };
static const GLfloat corGiz[]    = { 0.92f, 0.92f, 0.86f, 1.0f };

// ---------------------------------------------------------------- primitivas

// Caixa alinhada aos eixos, de (x0, y0, z0) a (x1, y1, z1), com as seis faces em sentido
// anti-horário vistas de fora (passa pelo back-face culling) e coordenadas de textura em
// unidades do mundo (a textura se repete a cada unidade, com GL_REPEAT).
static void caixa(float x0, float y0, float z0, float x1, float y1, float z1) {
    glBegin(GL_QUADS);
    glNormal3f(1, 0, 0);    // +x
    glTexCoord2f(z1, y0); glVertex3f(x1, y0, z1);
    glTexCoord2f(z0, y0); glVertex3f(x1, y0, z0);
    glTexCoord2f(z0, y1); glVertex3f(x1, y1, z0);
    glTexCoord2f(z1, y1); glVertex3f(x1, y1, z1);
    glNormal3f(-1, 0, 0);   // -x
    glTexCoord2f(z0, y0); glVertex3f(x0, y0, z0);
    glTexCoord2f(z1, y0); glVertex3f(x0, y0, z1);
    glTexCoord2f(z1, y1); glVertex3f(x0, y1, z1);
    glTexCoord2f(z0, y1); glVertex3f(x0, y1, z0);
    glNormal3f(0, 0, 1);    // +z
    glTexCoord2f(x0, y0); glVertex3f(x0, y0, z1);
    glTexCoord2f(x1, y0); glVertex3f(x1, y0, z1);
    glTexCoord2f(x1, y1); glVertex3f(x1, y1, z1);
    glTexCoord2f(x0, y1); glVertex3f(x0, y1, z1);
    glNormal3f(0, 0, -1);   // -z
    glTexCoord2f(x1, y0); glVertex3f(x1, y0, z0);
    glTexCoord2f(x0, y0); glVertex3f(x0, y0, z0);
    glTexCoord2f(x0, y1); glVertex3f(x0, y1, z0);
    glTexCoord2f(x1, y1); glVertex3f(x1, y1, z0);
    glNormal3f(0, 1, 0);    // +y (tampo)
    glTexCoord2f(x0, z1); glVertex3f(x0, y1, z1);
    glTexCoord2f(x1, z1); glVertex3f(x1, y1, z1);
    glTexCoord2f(x1, z0); glVertex3f(x1, y1, z0);
    glTexCoord2f(x0, z0); glVertex3f(x0, y1, z0);
    glNormal3f(0, -1, 0);   // -y
    glTexCoord2f(x0, z0); glVertex3f(x0, y0, z0);
    glTexCoord2f(x1, z0); glVertex3f(x1, y0, z0);
    glTexCoord2f(x1, z1); glVertex3f(x1, y0, z1);
    glTexCoord2f(x0, z1); glVertex3f(x0, y0, z1);
    glEnd();
}

// Pé (ou haste) de metal: caixa fina de lado 2*meio, centrada em (x, z), do chão até y1.
static void pe(float x, float z, float meio, float y0, float y1) {
    caixa(x - meio, y0, z - meio, x + meio, y1, z + meio);
}

// Retângulo no plano local do quadro (x para a direita, y para cima, z saindo da parede),
// subdividido para a luz do jogador (calculada nos vértices, Gouraud) não ficar "chapada".
// A textura se repete repU vezes na horizontal e repV na vertical.
static void retanguloLocal(float x0, float y0, float x1, float y1, float z,
                           int divisoes, float repU, float repV) {
    glNormal3f(0, 0, 1);
    glBegin(GL_QUADS);
    for (int i = 0; i < divisoes; i++) {
        for (int j = 0; j < divisoes; j++) {
            float u0 = (float) i / divisoes, u1 = (float) (i + 1) / divisoes;
            float v0 = (float) j / divisoes, v1 = (float) (j + 1) / divisoes;
            float cantos[4][2] = { {u0, v0}, {u1, v0}, {u1, v1}, {u0, v1} };
            for (int k = 0; k < 4; k++) {
                float u = cantos[k][0], v = cantos[k][1];
                glTexCoord2f(u * repU, v * repV);
                glVertex3f(x0 + u * (x1 - x0), y0 + v * (y1 - y0), z);
            }
        }
    }
    glEnd();
}

// ---------------------------------------------------------------- giz no quadro

// Na fonte stroke do GLUT uma letra maiúscula tem cerca de 100 unidades de altura.
#define ALTURA_MAIUSCULA 100.0f

static float larguraTexto(void *fonte, const char *texto) {
    float largura = 0;
    for (const char *p = texto; *p; p++) largura += glutStrokeWidth(fonte, *p);
    return largura;
}

// Escreve uma linha de giz no plano do quadro. alinhamento: 0 = começa em x, 1 = centrada em x.
static void escreveGiz(void *fonte, const char *texto, float x, float yBase,
                       float alturaLetra, int alinhamento) {
    float escala = alturaLetra / ALTURA_MAIUSCULA;
    float x0 = alinhamento ? x - larguraTexto(fonte, texto) * escala / 2 : x;
    glPushMatrix();
    glTranslatef(x0, yBase, 0.006f);
    glScalef(escala, escala, 1.0f);   // z fica em 1 para não estragar a normal (iluminação)
    for (const char *p = texto; *p; p++) glutStrokeCharacter(fonte, *p);
    glPopMatrix();
}

static void linhaGiz(float xa, float ya, float xb, float yb) {
    glBegin(GL_LINES);
    glVertex3f(xa, ya, 0.006f);
    glVertex3f(xb, yb, 0.006f);
    glEnd();
}

// Cubo em arame desenhado a giz (projeção oblíqua): dois quadrados e as quatro arestas de fundo.
static void cuboDeGiz(float x, float y, float lado) {
    float dx = lado * 0.45f, dy = lado * 0.3f;
    float fx[4] = { x, x + lado, x + lado, x };
    float fy[4] = { y, y, y + lado, y + lado };
    for (int i = 0; i < 4; i++) {
        int j = (i + 1) % 4;
        linhaGiz(fx[i], fy[i], fx[j], fy[j]);                       // frente
        linhaGiz(fx[i] + dx, fy[i] + dy, fx[j] + dx, fy[j] + dy);   // fundo
        linhaGiz(fx[i], fy[i], fx[i] + dx, fy[i] + dy);             // arestas de profundidade
    }
    // Eixos x, y e z saindo do canto de baixo.
    linhaGiz(x, y, x + lado * 1.6f, y);
    linhaGiz(x, y, x, y + lado * 1.6f);
    linhaGiz(x, y, x - dx, y - dy);
}

// ---------------------------------------------------------------- móveis

// Quadro-negro com moldura, porta-giz, texto e um relógio em cima, na parede oeste.
// A matriz leva para o canto esquerdo do quadro (visto de dentro da sala) e gira 90 graus
// em torno de y: no sistema local, x corre ao longo da parede, y sobe e z sai da parede.
static void desenhaQuadro(float zCentro) {
    const float LARGURA = 4.0f, Y0 = 0.85f, Y1 = 2.05f, MOLDURA = 0.06f;
    float l = LARGURA / 2;
    if (l > (salaZ1 - salaZ0) / 2 - 0.4f) l = (salaZ1 - salaZ0) / 2 - 0.4f;   // sala estreita
    float largura = 2 * l;

    glPushMatrix();
    glTranslatef(salaX0, 0.0f, zCentro + l);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

    // Moldura e porta-giz de madeira (caixas no sistema local: x ao longo, z para fora).
    glBindTexture(GL_TEXTURE_2D, idTexturas[TEX_MADEIRA]);
    caixa(-MOLDURA, Y1, 0.0f, largura + MOLDURA, Y1 + MOLDURA, 0.05f);   // em cima
    caixa(-MOLDURA, Y0 - MOLDURA, 0.0f, largura + MOLDURA, Y0, 0.05f);   // embaixo
    caixa(-MOLDURA, Y0, 0.0f, 0.0f, Y1, 0.05f);                          // esquerda
    caixa(largura, Y0, 0.0f, largura + MOLDURA, Y1, 0.05f);              // direita
    caixa(0.0f, Y0 - MOLDURA - 0.03f, 0.0f, largura, Y0 - MOLDURA, 0.14f);   // porta-giz

    // Superfície verde do quadro, um pouco à frente da parede.
    glBindTexture(GL_TEXTURE_2D, idTexturas[TEX_QUADRO]);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.03f);
    retanguloLocal(0.0f, Y0, largura, Y1, 0.0f, 8, largura, 1.0f);

    // Giz: sem textura, cor do material, com a mesma luz das paredes.
    glDisable(GL_TEXTURE_2D);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corGiz);
    glLineWidth(2.0f);
    escreveGiz(GLUT_STROKE_ROMAN, "PROVA FINAL", largura / 2, Y1 - 0.26f, 0.16f, 1);
    escreveGiz(GLUT_STROKE_ROMAN, "Computacao Grafica", largura / 2, Y1 - 0.45f, 0.09f, 1);
    cuboDeGiz(0.35f, Y0 + 0.2f, 0.35f);
    float xCodigo = largura * 0.42f;
    escreveGiz(GLUT_STROKE_MONO_ROMAN, "glTranslatef(x, y, z);", xCodigo, Y0 + 0.50f, 0.065f, 0);
    escreveGiz(GLUT_STROKE_MONO_ROMAN, "glRotatef(a, 0, 1, 0);", xCodigo, Y0 + 0.35f, 0.065f, 0);
    escreveGiz(GLUT_STROKE_MONO_ROMAN, "glScalef(s, s, s);",     xCodigo, Y0 + 0.20f, 0.065f, 0);
    glLineWidth(1.0f);
    glPopMatrix();

    // Pedaços de giz e apagador no porta-giz.
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corGiz);
    caixa(0.40f, Y0 - MOLDURA, 0.06f, 0.48f, Y0 - MOLDURA + 0.015f, 0.075f);
    caixa(0.60f, Y0 - MOLDURA, 0.08f, 0.66f, Y0 - MOLDURA + 0.015f, 0.095f);
    GLfloat corApagador[] = { 0.15f, 0.15f, 0.18f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corApagador);
    caixa(largura - 0.45f, Y0 - MOLDURA, 0.04f, largura - 0.25f, Y0 - MOLDURA + 0.05f, 0.11f);

    // Relógio de parede acima do quadro, marcando 7h55: a prova já vai começar.
    float cx = largura / 2, cy = 2.29f;
    const int LADOS = 24;
    GLfloat corAro[] = { 0.12f, 0.12f, 0.12f, 1.0f };
    float raios[2] = { 0.18f, 0.16f };
    float afasta[2] = { 0.010f, 0.016f };
    for (int d = 0; d < 2; d++) {
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, d == 0 ? corAro : corBranca);
        glNormal3f(0, 0, 1);
        glBegin(GL_TRIANGLE_FAN);   // anti-horário visto de frente
        glVertex3f(cx, cy, afasta[d]);
        for (int i = 0; i <= LADOS; i++) {
            float t = 2.0f * 3.14159265f * i / LADOS;
            glVertex3f(cx + raios[d] * cosf(t), cy + raios[d] * sinf(t), afasta[d]);
        }
        glEnd();
    }
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corAro);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    for (int h = 0; h < 12; h++) {   // marcas das horas
        float a = h * 3.14159265f / 6.0f;
        glVertex3f(cx + 0.13f * sinf(a), cy + 0.13f * cosf(a), 0.020f);
        glVertex3f(cx + 0.155f * sinf(a), cy + 0.155f * cosf(a), 0.020f);
    }
    // Ponteiros: ângulo no sentido horário a partir das 12, direção (sen a, cos a).
    float aHora = (7.0f + 55.0f / 60.0f) * 3.14159265f / 6.0f;
    float aMin  = 55.0f * 3.14159265f / 30.0f;
    glVertex3f(cx, cy, 0.022f); glVertex3f(cx + 0.08f * sinf(aHora), cy + 0.08f * cosf(aHora), 0.022f);
    glVertex3f(cx, cy, 0.024f); glVertex3f(cx + 0.13f * sinf(aMin),  cy + 0.13f * cosf(aMin),  0.024f);
    glEnd();
    glLineWidth(1.0f);

    glEnable(GL_TEXTURE_2D);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corBranca);
    glPopMatrix();
}

static void adicionaMovel(float x0, float z0, float x1, float z1) {
    if (quantMoveis >= MAX_MOVEIS) return;
    Caixa2D c = { x0, z0, x1, z1 };
    moveis[quantMoveis++] = c;
}

// Mesa do professor de frente para a turma (para +x) e a cadeira dele entre a mesa e o quadro.
static void desenhaMesaProfessor(float zc) {
    float x0 = salaX0 + 1.4f, x1 = salaX0 + 2.2f;
    float z0 = zc - 0.8f, z1 = zc + 0.8f;

    glBindTexture(GL_TEXTURE_2D, idTexturas[TEX_MADEIRA]);
    caixa(x0, 0.72f, z0, x1, 0.77f, z1);                          // tampo
    caixa(x1 - 0.06f, 0.08f, z0 + 0.03f, x1 - 0.03f, 0.72f, z1 - 0.03f);   // painel da frente
    caixa(x0 + 0.02f, 0.0f, z0 + 0.02f, x1 - 0.02f, 0.72f, z0 + 0.06f);    // laterais
    caixa(x0 + 0.02f, 0.0f, z1 - 0.06f, x1 - 0.02f, 0.72f, z1 - 0.02f);

    // Cadeira do professor, virada para a turma.
    float cx0 = salaX0 + 0.70f, cx1 = salaX0 + 1.15f;
    caixa(cx0 + 0.04f, 0.45f, zc - 0.22f, cx1, 0.49f, zc + 0.22f);   // assento
    caixa(cx0, 0.49f, zc - 0.22f, cx0 + 0.04f, 1.00f, zc + 0.22f);   // encosto

    glDisable(GL_TEXTURE_2D);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corMetal);
    pe(cx0 + 0.07f, zc - 0.18f, 0.02f, 0.0f, 0.45f);
    pe(cx0 + 0.07f, zc + 0.18f, 0.02f, 0.0f, 0.45f);
    pe(cx1 - 0.04f, zc - 0.18f, 0.02f, 0.0f, 0.45f);
    pe(cx1 - 0.04f, zc + 0.18f, 0.02f, 0.0f, 0.45f);

    // Em cima da mesa: a pilha de provas, um livro e uma maçã para o professor.
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corBranca);
    caixa(x0 + 0.20f, 0.77f, zc + 0.20f, x0 + 0.50f, 0.81f, zc + 0.62f);
    GLfloat corLivro[] = { 0.55f, 0.12f, 0.12f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corLivro);
    caixa(x0 + 0.15f, 0.77f, zc - 0.25f, x0 + 0.45f, 0.83f, zc + 0.05f);
    GLfloat corMaca[] = { 0.75f, 0.08f, 0.06f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corMaca);
    glPushMatrix();
    glTranslatef(x0 + 0.35f, 0.82f, zc - 0.50f);
    glutSolidSphere(0.05, 12, 8);
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corBranca);

    adicionaMovel(cx0 - 0.02f, z0, x1, z1);
}

// Carteira escolar com tampo (centro em dx, dz) e a cadeira logo atrás (+x), olhando para o quadro.
static void desenhaCarteira(float dx, float dz, bool comFolha) {
    glBindTexture(GL_TEXTURE_2D, idTexturas[TEX_MADEIRA]);
    caixa(dx - 0.25f, 0.70f, dz - 0.35f, dx + 0.25f, 0.73f, dz + 0.35f);   // tampo
    caixa(dx + 0.32f, 0.42f, dz - 0.20f, dx + 0.72f, 0.45f, dz + 0.20f);   // assento
    caixa(dx + 0.70f, 0.55f, dz - 0.20f, dx + 0.73f, 0.85f, dz + 0.20f);   // encosto

    glDisable(GL_TEXTURE_2D);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corMetal);
    // Pés da carteira e a grade de guardar material embaixo do tampo.
    pe(dx - 0.21f, dz - 0.31f, 0.015f, 0.0f, 0.70f);
    pe(dx - 0.21f, dz + 0.31f, 0.015f, 0.0f, 0.70f);
    pe(dx + 0.21f, dz - 0.31f, 0.015f, 0.0f, 0.70f);
    pe(dx + 0.21f, dz + 0.31f, 0.015f, 0.0f, 0.70f);
    caixa(dx - 0.21f, 0.55f, dz - 0.31f, dx + 0.15f, 0.565f, dz + 0.31f);
    // Pés da cadeira e as hastes do encosto.
    pe(dx + 0.35f, dz - 0.17f, 0.015f, 0.0f, 0.42f);
    pe(dx + 0.35f, dz + 0.17f, 0.015f, 0.0f, 0.42f);
    pe(dx + 0.69f, dz - 0.17f, 0.015f, 0.0f, 0.42f);
    pe(dx + 0.69f, dz + 0.17f, 0.015f, 0.0f, 0.42f);
    pe(dx + 0.715f, dz - 0.17f, 0.012f, 0.45f, 0.55f);
    pe(dx + 0.715f, dz + 0.17f, 0.012f, 0.45f, 0.55f);

    if (comFolha) {   // folha de prova esperando o aluno
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corBranca);
        caixa(dx - 0.12f, 0.73f, dz - 0.16f, dx + 0.12f, 0.736f, dz + 0.16f);
    }
    glEnable(GL_TEXTURE_2D);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corBranca);

    adicionaMovel(dx - 0.27f, dz - 0.37f, dx + 0.75f, dz + 0.37f);
}

// Verdadeiro se o ponto (x, z) fica a menos de folga da caixa.
static bool pertoDaCaixa(float x, float z, float x0, float z0, float x1, float z1, float folga) {
    return x > x0 - folga && x < x1 + folga && z > z0 - folga && z < z1 + folga;
}

// ---------------------------------------------------------------- interface

void iniciaSala() {
    temSala = false;
    quantMoveis = 0;

    // Porta 'P' e as células dos dois lados dela: o corredor e a sala ('F').
    bool achouPorta = false;
    for (int l = 0; l < MAPA_LINHAS && !achouPorta; l++)
        for (int c = 0; c < MAPA_COLUNAS && !achouPorta; c++)
            if (celula(l, c) == CEL_PORTA) { lPorta = l; cPorta = c; achouPorta = true; }
    if (!achouPorta) { printf("Aviso: mapa sem porta 'P', sem sala de aula\n"); return; }

    bool achouFora = false, achouDentro = false;
    const int dl[4] = { -1, 1, 0, 0 }, dc[4] = { 0, 0, -1, 1 };
    for (int d = 0; d < 4; d++) {
        int l = lPorta + dl[d], c = cPorta + dc[d];
        if (celula(l, c) == CEL_SALA)  { lDentro = l; cDentro = c; achouDentro = true; }
        else if (!celulaSolida(l, c))  { lFora = l;   cFora = c;   achouFora = true; }
    }
    if (!achouFora || !achouDentro) {
        printf("Aviso: a porta 'P' precisa de um corredor de um lado e da sala 'F' do outro\n");
        return;
    }

    // Retângulo que envolve todas as células 'F'.
    int lMin = MAPA_LINHAS, lMax = -1, cMin = MAPA_COLUNAS, cMax = -1;
    for (int l = 0; l < MAPA_LINHAS; l++) {
        for (int c = 0; c < MAPA_COLUNAS; c++) {
            if (celula(l, c) != CEL_SALA) continue;
            if (l < lMin) lMin = l;
            if (l > lMax) lMax = l;
            if (c < cMin) cMin = c;
            if (c > cMax) cMax = c;
        }
    }
    salaX0 = cMin * TAM_CELULA;       salaX1 = (cMax + 1) * TAM_CELULA;
    salaZ0 = lMin * TAM_CELULA;       salaZ1 = (lMax + 1) * TAM_CELULA;
    float zc = (salaZ0 + salaZ1) / 2;
    temSala = true;

    float xEntra, zEntra;   // onde o jogador aparece ao entrar: nenhum móvel pode ficar ali
    centroDaCelula(lDentro, cDentro, &xEntra, &zEntra);

    listaSala = glGenLists(1);
    glNewList(listaSala, GL_COMPILE);
    desenhaQuadro(zc);
    desenhaMesaProfessor(zc);

    // Fileiras de carteiras viradas para o quadro: colunas a cada 2,5 unidades a partir da mesa
    // do professor, até 1,6 unidade da parede do fundo; até três carteiras por coluna.
    float profundidade = salaZ1 - salaZ0;
    int porColuna = profundidade >= 5.5f ? 3 : (profundidade >= 3.5f ? 2 : 1);
    int n = 0;
    for (float dx = salaX0 + 4.5f; dx + 0.75f <= salaX1 - 1.6f; dx += 2.5f) {
        for (int i = 0; i < porColuna; i++) {
            float dz = zc + (i - (porColuna - 1) / 2.0f) * 1.9f;
            // Deixa livre o lugar onde o jogador aparece e o primeiro passo dele para o quadro.
            if (pertoDaCaixa(xEntra, zEntra, dx - 0.27f, dz - 0.37f, dx + 0.75f, dz + 0.37f, 1.0f))
                continue;
            desenhaCarteira(dx, dz, (n++ % 3) != 1);
        }
    }
    glEndList();

    printf("Sala de aula: porta em (%d, %d), %d moveis\n", lPorta, cPorta, quantMoveis);
}

void desenhaSala() {
    if (temSala) glCallList(listaSala);
}

void atualizaSala(float dt) {
    if (tempoPassagem > 0.0f) {
        tempoPassagem -= dt;
        if (tempoPassagem < 0.0f) tempoPassagem = 0.0f;
    }
}

bool jogadorNaSala() {
    int l, c;
    celulaDoMundo(jogador.x, jogador.z, &l, &c);
    return celula(l, c) == CEL_SALA;
}

// Verdadeiro se o jogador está a até SALA_ALCANCE_PORTA da célula da porta (de qualquer lado).
static bool pertoDaPorta() {
    float x0 = cPorta * TAM_CELULA, x1 = x0 + TAM_CELULA;
    float z0 = lPorta * TAM_CELULA, z1 = z0 + TAM_CELULA;
    float px = jogador.x < x0 ? x0 : (jogador.x > x1 ? x1 : jogador.x);
    float pz = jogador.z < z0 ? z0 : (jogador.z > z1 ? z1 : jogador.z);
    float dx = jogador.x - px, dz = jogador.z - pz;
    return dx * dx + dz * dz < SALA_ALCANCE_PORTA * SALA_ALCANCE_PORTA;
}

// A porta só abre com todas as folhas de frequência coletadas (frequencia.cpp guarda o progresso
// em jogo.cadernosColetados/cadernosTotal). Sem folhas no mapa (cadernosTotal == 0), fica liberada.
static bool portaTrancada() {
    return jogo.cadernosTotal > 0 && !todosCadernosColetados();
}

// Leva o jogador ao centro da célula (linha, coluna), olhando na direção yaw.
static void levaJogador(int linha, int coluna, float yaw) {
    centroDaCelula(linha, coluna, &jogador.x, &jogador.z);
    jogador.yaw = yaw;
    jogador.pitch = 0.0f;
    tempoPassagem = SALA_TEMPO_PASSAGEM;
}

bool interagePortaSala() {
    if (!temSala || jogo.estado != JOGO_JOGANDO) return false;
    if (perseguidorAgarrouJogador()) return false;   // sendo arrastado, não abre porta
    if (!pertoDaPorta()) return false;

    if (jogadorNaSala()) {
        // Sai para o corredor, de costas para a porta.
        float yawFora = atan2f((float) (cFora - cPorta), (float) -(lFora - lPorta));
        levaJogador(lFora, cFora, yawFora);
        printf("Saiu da sala de aula\n");
    } else if (portaTrancada()) {
        printf("Porta trancada: frequencia %d/%d\n", jogo.cadernosColetados, jogo.cadernosTotal);
    } else {
        levaJogador(lDentro, cDentro, OLHA_PARA_OESTE);   // entra olhando para o quadro
        printf("Entrou na sala de aula\n");
    }
    return true;
}

bool colideComSala(float x, float z, float raio) {
    for (int i = 0; i < quantMoveis; i++) {
        const Caixa2D &m = moveis[i];
        float px = x < m.x0 ? m.x0 : (x > m.x1 ? m.x1 : x);   // ponto da caixa mais próximo
        float pz = z < m.z0 ? m.z0 : (z > m.z1 ? m.z1 : z);
        float dx = x - px, dz = z - pz;
        if (dx * dx + dz * dz < raio * raio) return true;
    }
    return false;
}

const char *dicaPortaSala() {
    if (!temSala || jogo.estado != JOGO_JOGANDO || !pertoDaPorta()) return NULL;
    if (perseguidorAgarrouJogador()) return NULL;
    if (jogadorNaSala()) return "[E] Sair da sala de aula";
    if (portaTrancada()) {
        static char dica[64];
        sprintf(dica, "Porta trancada: colete a frequencia (%d/%d)",
                jogo.cadernosColetados, jogo.cadernosTotal);
        return dica;
    }
    return "[E] Entrar na sala de aula";
}

float escurecimentoSala() {
    return tempoPassagem / SALA_TEMPO_PASSAGEM;
}
