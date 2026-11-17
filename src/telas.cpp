#include "telas.h"
#include "jogo.h"
#include "jogador.h"
#include "gl_includes.h"

// As fontes bitmap do GLUT só têm os caracteres ASCII (32 a 126), por isso os textos
// das telas são escritos sem acento.
#define FONTE_TITULO  GLUT_BITMAP_TIMES_ROMAN_24
#define FONTE_TEXTO   GLUT_BITMAP_HELVETICA_18
#define FONTE_COMANDO GLUT_BITMAP_9_BY_15   // monoespaçada: alinha a tabela de comandos

// ---------------------------------------------------------------- texto

static void escreve(float x, float y, const char *texto, void *fonte) {
    glRasterPos2f(x, y);
    for (const char *p = texto; *p; p++) glutBitmapCharacter(fonte, *p);
}

static void escreveCentrado(float y, const char *texto, void *fonte, int largura) {
    int w = glutBitmapLength(fonte, (const unsigned char *) texto);
    escreve((largura - w) / 2.0f, y, texto, fonte);
}

// ---------------------------------------------------------------- projeção 2D

// Troca a projeção em perspectiva por uma ortográfica em pixels (origem no canto inferior
// esquerdo) e desliga o que atrapalha texto plano: z-buffer, iluminação, textura e culling.
// Tudo é guardado nas pilhas e devolvido em fim2D(), para a cena 3D não notar.
static void inicio2D(int largura, int altura) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, largura, 0, altura);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);   // para o fundo translúcido
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void fim2D() {
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// Retângulo escuro translúcido cobrindo a janela: o labirinto fica visível ao fundo.
static void escureceFundo(int largura, int altura, float alfa) {
    glColor4f(0.0f, 0.0f, 0.0f, alfa);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f((float) largura, 0);
    glVertex2f((float) largura, (float) altura);
    glVertex2f(0, (float) altura);
    glEnd();
}

// ---------------------------------------------------------------- telas

static void desenhaTelaInicio(int largura, int altura) {
    escureceFundo(largura, altura, 0.78f);

    float y = altura - 70.0f;
    glColor3f(1.0f, 0.9f, 0.4f);
    escreveCentrado(y, "LABIRINTO DA FACULDADE", FONTE_TITULO, largura);

    y -= 40.0f;
    glColor3f(1.0f, 1.0f, 1.0f);
    escreveCentrado(y, "Voce esta atrasado para a prova. Pegue todos os cadernos,", FONTE_TEXTO, largura);
    y -= 24.0f;
    escreveCentrado(y, "fuja de quem quer te atrasar e chegue a sala antes do tempo acabar.", FONTE_TEXTO, largura);

    y -= 46.0f;
    glColor3f(0.75f, 0.85f, 1.0f);
    escreveCentrado(y, "COMANDOS", FONTE_TEXTO, largura);

    // Tabela de comandos alinhada à esquerda de um bloco centralizado.
    static const char *comandos[] = {
        "W A S D ou setas   andar e girar",
        "Mouse              olhar em volta",
        "E                  fazer carinho no gato (perto dele)",
        "M                  soltar ou prender o mouse",
        "ESC                sair do jogo",
    };
    const int quant = (int) (sizeof(comandos) / sizeof(comandos[0]));
    int maior = 0;
    for (int i = 0; i < quant; i++) {
        int w = glutBitmapLength(FONTE_COMANDO, (const unsigned char *) comandos[i]);
        if (w > maior) maior = w;
    }
    float x = (largura - maior) / 2.0f;
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < quant; i++) {
        y -= 22.0f;
        escreve(x, y, comandos[i], FONTE_COMANDO);
    }

    y -= 50.0f;
    glColor3f(1.0f, 0.9f, 0.4f);
    escreveCentrado(y, "Entendeu os comandos?", FONTE_TEXTO, largura);
    y -= 26.0f;
    escreveCentrado(y, "Aperte ENTER para confirmar e comecar", FONTE_TEXTO, largura);
}

static void desenhaTelaFim(int largura, int altura, const char *titulo, const char *linha) {
    escureceFundo(largura, altura, 0.7f);
    float y = altura / 2.0f + 30.0f;
    glColor3f(1.0f, 0.9f, 0.4f);
    escreveCentrado(y, titulo, FONTE_TITULO, largura);
    y -= 34.0f;
    glColor3f(1.0f, 1.0f, 1.0f);
    escreveCentrado(y, linha, FONTE_TEXTO, largura);
    y -= 30.0f;
    escreveCentrado(y, "Aperte ENTER para jogar de novo", FONTE_TEXTO, largura);
}

// ---------------------------------------------------------------- interface

void desenhaTelas(int larguraJanela, int alturaJanela) {
    if (jogo.estado == JOGO_JOGANDO || jogo.estado == JOGO_PROVA) return;

    inicio2D(larguraJanela, alturaJanela);
    switch (jogo.estado) {
        case JOGO_MENU:
            desenhaTelaInicio(larguraJanela, alturaJanela);
            break;
        case JOGO_DERROTA:
            desenhaTelaFim(larguraJanela, alturaJanela, "TEMPO ESGOTADO",
                           "Voce nao chegou a prova a tempo.");
            break;
        case JOGO_VITORIA:
            desenhaTelaFim(larguraJanela, alturaJanela, "APROVADO!",
                           "Voce chegou a prova com todos os cadernos.");
            break;
        default:
            break;
    }
    fim2D();
}

bool teclaTelas(unsigned char tecla) {
    bool enter = (tecla == 13 || tecla == 10);

    switch (jogo.estado) {
        case JOGO_MENU:
            if (enter || tecla == ' ' || tecla == 's') {   // o jogador confirmou que entendeu
                comecaPartida();
                return true;
            }
            return true;   // no menu, nenhuma outra tecla chega ao jogo

        case JOGO_DERROTA:
        case JOGO_VITORIA:
            if (enter) {
                iniciaJogador();   // volta para a entrada da faculdade
                comecaPartida();
            }
            return true;

        default:
            return false;   // jogando: as teclas são dos controles
    }
}
