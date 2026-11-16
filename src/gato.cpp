#include "gato.h"
#include "caminho.h"
#include "mapa.h"
#include "jogador.h"
#include "jogo.h"
#include "gl_includes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

Gato gato;

#define GRAUS_POR_RADIANO 57.29578f

static const char *nomeEstado(EstadoGato e) {
    switch (e) {
        case GATO_VAGANDO:    return "vagando";
        case GATO_PARADO:     return "parado";
        case GATO_ACARICIADO: return "acariciado";
    }
    return "?";
}

// ---------------------------------------------------------------- percepção

static float distanciaAoJogador() {
    float dx = jogador.x - gato.x, dz = jogador.z - gato.z;
    return sqrtf(dx * dx + dz * dz);
}

// Colisão esfera x esfera (aula 21): há contato quando a distância entre os centros
// é menor que a soma dos raios. Para o carinho, o "raio" do gato é o alcance da mão.
static bool jogadorAoAlcance() {
    return distanciaAoJogador() < GATO_ALCANCE_CARINHO + JOGADOR_RAIO;
}

static bool encostaNoJogador() {
    return distanciaAoJogador() < GATO_RAIO + JOGADOR_RAIO;
}

// ---------------------------------------------------------------- máquina de estados

static void trocaEstado(EstadoGato novo) {
    if (gato.estado == novo) return;
    printf("Gato: %s -> %s\n", nomeEstado(gato.estado), nomeEstado(novo));
    gato.estado = novo;
    gato.tempoNoEstado = 0.0f;
}

// Todas as transições por tempo ficam aqui; a transição para ACARICIADO vem da tecla,
// em interageComGato(), porque depende de um evento e não do relógio.
static void transita() {
    switch (gato.estado) {
        case GATO_VAGANDO:
            break;   // sai daqui ao chegar numa célula (em move) ou pela tecla

        case GATO_PARADO:
            if (gato.tempoNoEstado >= gato.tempoParado) trocaEstado(GATO_VAGANDO);
            break;

        case GATO_ACARICIADO:
            if (gato.tempoNoEstado >= GATO_TEMPO_CARINHO) trocaEstado(GATO_VAGANDO);
            break;
    }
}

void interageComGato() {
    if (jogo.estado != JOGO_JOGANDO) return;
    if (!jogadorAoAlcance()) return;
    if (gato.estado == GATO_ACARICIADO) { gato.tempoNoEstado = 0.0f; return; }   // prolonga

    gato.carinhos++;
    printf("Miau! Gato acariciado (%d)\n", gato.carinhos);
    gato.yaw = atan2f(jogador.x - gato.x, -(jogador.z - gato.z));   // vira para o jogador
    trocaEstado(GATO_ACARICIADO);
}

// ---------------------------------------------------------------- movimento

// Ao chegar numa célula: às vezes senta, senão sorteia uma direção (estado 1 do Pacman).
static void escolheProximaCelula() {
    int l, c;
    celulaDoMundo(gato.x, gato.z, &l, &c);

    if ((float) rand() / RAND_MAX < GATO_CHANCE_PARAR) {
        gato.tempoParado = 1.0f + 2.0f * ((float) rand() / RAND_MAX);   // de 1 a 3 segundos
        trocaEstado(GATO_PARADO);
        gato.lAlvo = l;
        gato.cAlvo = c;
        return;
    }

    int lProx, cProx;
    if (sorteiaVizinho(l, c, gato.lAnterior, gato.cAnterior, &lProx, &cProx)) {
        gato.lAnterior = l;
        gato.cAnterior = c;
        gato.lAlvo = lProx;
        gato.cAlvo = cProx;
    }
}

// Anda em direção a um ponto; testa cada eixo separado para deslizar na parede.
// Retorna verdadeiro se chegou.
static bool andaPara(float xAlvo, float zAlvo, float dt) {
    float dx = xAlvo - gato.x, dz = zAlvo - gato.z;
    float distancia = sqrtf(dx * dx + dz * dz);
    float passo = gato.velocidade * dt;

    if (distancia <= passo) {
        gato.x = xAlvo;
        gato.z = zAlvo;
        return true;
    }

    dx *= passo / distancia;
    dz *= passo / distancia;
    gato.yaw = atan2f(dx, -dz);
    gato.faseAnimacao += passo * 8.0f;

    if (!colideComMapa(gato.x + dx, gato.z, GATO_RAIO)) gato.x += dx;
    if (!colideComMapa(gato.x, gato.z + dz, GATO_RAIO)) gato.z += dz;
    return false;
}

static void move(float dt) {
    if (gato.estado != GATO_VAGANDO) return;   // sentado ou recebendo carinho: fica parado
    if (encostaNoJogador()) return;            // não atravessa o jogador: espera ele sair

    float xAlvo, zAlvo;
    centroDaCelula(gato.lAlvo, gato.cAlvo, &xAlvo, &zAlvo);
    if (andaPara(xAlvo, zAlvo, dt)) escolheProximaCelula();
}

// ---------------------------------------------------------------- interface

void iniciaGato() {
    gato.x = gato.z = -1.0f;
    for (int l = 0; l < MAPA_LINHAS && gato.x < 0.0f; l++)
        for (int c = 0; c < MAPA_COLUNAS; c++)
            if (celula(l, c) == CEL_GATO) {
                centroDaCelula(l, c, &gato.x, &gato.z);
                break;
            }
    if (gato.x < 0.0f) {   // mapa sem símbolo: nasce na célula (1, 1)
        centroDaCelula(1, 1, &gato.x, &gato.z);
        printf("Aviso: mapa sem '%c'; gato nasce em (1, 1)\n", CEL_GATO);
    }

    gato.yaw = 0.0f;
    gato.estado = GATO_VAGANDO;
    gato.velocidade = GATO_VELOCIDADE;
    gato.tempoNoEstado = 0.0f;
    gato.tempoParado = 0.0f;
    gato.faseAnimacao = 0.0f;
    gato.carinhos = 0;
    gato.lAnterior = gato.cAnterior = -1;
    celulaDoMundo(gato.x, gato.z, &gato.lAlvo, &gato.cAlvo);
    escolheProximaCelula();
    printf("Gato: nasce em (%.1f, %.1f), estado %s\n", gato.x, gato.z, nomeEstado(gato.estado));
}

void atualizaGato(float dt) {
    gato.tempoNoEstado += dt;
    transita();
    move(dt);
}

// ---------------------------------------------------------------- desenho

// Esfera esticada: centro em (0, 0, 0), raios (rx, ry, rz).
static void elipsoide(float rx, float ry, float rz) {
    glPushMatrix();
    glScalef(rx, ry, rz);
    glutSolidSphere(1.0, 14, 14);
    glPopMatrix();
}

// Orelha: cone apontando para cima, preso no topo da cabeça.
static void orelha(float x) {
    glPushMatrix();
    glTranslatef(x, 0.12f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);   // o cone do GLUT aponta para +z; vira para +y
    glutSolidCone(0.05, 0.12, 8, 2);
    glPopMatrix();
}

// Olho: esfera pequena; fechado, vira um traço (esfera achatada em y).
static void olho(float x, bool fechado) {
    glPushMatrix();
    glTranslatef(x, 0.04f, -0.14f);
    if (fechado) glScalef(1.0f, 0.25f, 1.0f);
    glutSolidSphere(0.03, 8, 8);
    glPopMatrix();
}

// Pata: cilindro curto (esfera esticada), balança para a frente e para trás ao andar.
static void pata(float x, float z, float anguloGraus) {
    glPushMatrix();
    glTranslatef(x, 0.22f, z);
    glRotatef(anguloGraus, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.11f, 0.0f);
    elipsoide(0.045f, 0.11f, 0.045f);
    glPopMatrix();
}

void desenhaGato() {
    GLfloat corPelo[]   = { 0.90f, 0.50f, 0.15f, 1.0f };   // laranja
    GLfloat corClaro[]  = { 0.95f, 0.80f, 0.60f, 1.0f };   // focinho e orelha
    GLfloat corOlho[]   = { 0.05f, 0.05f, 0.05f, 1.0f };
    GLfloat corFeliz[]  = { 1.00f, 0.65f, 0.30f, 1.0f };   // mais claro ao ser acariciado
    GLfloat branco[]    = { 1.0f, 1.0f, 1.0f, 1.0f };

    float tempo = (float) glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    bool acariciado = (gato.estado == GATO_ACARICIADO);
    bool andando = (gato.estado == GATO_VAGANDO);

    // Patas balançam só ao andar; o rabo balança sempre, mais rápido no carinho.
    float passada = andando ? 30.0f * sinf(gato.faseAnimacao) : 0.0f;
    float rabo = acariciado ? 40.0f * sinf(tempo * 10.0f) : 15.0f * sinf(tempo * 3.0f);
    float cabeca = acariciado ? 12.0f * sinf(tempo * 6.0f) : 0.0f;   // balança a cabeça

    glDisable(GL_TEXTURE_2D);   // as primitivas GLUT não têm coordenadas de textura
    glEnable(GL_NORMALIZE);     // o glScalef dos elipsoides esticaria as normais e escureceria a luz

    glPushMatrix();
    glTranslatef(gato.x, 0.0f, gato.z);
    glRotatef(-gato.yaw * GRAUS_POR_RADIANO, 0.0f, 1.0f, 0.0f);   // ver comentário no perseguidor

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, acariciado ? corFeliz : corPelo);

    // Corpo: elipsoide deitado ao longo de z (frente = -z)
    glPushMatrix();
    glTranslatef(0.0f, 0.32f, 0.0f);
    elipsoide(0.16f, 0.15f, 0.28f);
    glPopMatrix();

    // Patas
    pata(-0.09f, -0.16f,  passada);
    pata( 0.09f, -0.16f, -passada);
    pata(-0.09f,  0.16f, -passada);
    pata( 0.09f,  0.16f,  passada);

    // Rabo: cone preso atrás, gira em torno da base
    glPushMatrix();
    glTranslatef(0.0f, 0.38f, 0.26f);
    glRotatef(rabo, 0.0f, 1.0f, 0.0f);
    glRotatef(-30.0f, 1.0f, 0.0f, 0.0f);   // levantado
    glutSolidCone(0.04, 0.30, 8, 2);
    glPopMatrix();

    // Cabeça (com orelhas, olhos e focinho dentro do mesmo sistema de coordenadas)
    glPushMatrix();
    glTranslatef(0.0f, 0.50f, -0.26f);
    glRotatef(cabeca, 0.0f, 0.0f, 1.0f);
    glutSolidSphere(0.15, 14, 14);

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corClaro);
    orelha(-0.08f);
    orelha( 0.08f);
    glPushMatrix();                      // focinho
    glTranslatef(0.0f, -0.02f, -0.14f);
    glutSolidSphere(0.05, 8, 8);
    glPopMatrix();

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corOlho);
    olho(-0.06f, acariciado);
    olho( 0.06f, acariciado);
    glPopMatrix();

    glPopMatrix();

    // Restaura o que o cenário usa: material branco (a textura dá a cor) e textura ligada.
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, branco);
    glDisable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);
}
