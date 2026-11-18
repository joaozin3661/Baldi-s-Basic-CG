#include "telas.h"
#include "jogo.h"
#include "jogador.h"
#include "sala.h"
#include "gl_includes.h"
#include "frequencia.h"
#include <cstdio>

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
    escreveCentrado(y, "Voce esta atrasado para a prova. Ache as 4 folhas da frequencia da faculdade", FONTE_TEXTO, largura);
    y -= 24.0f;
    escreveCentrado(y, "(no chao ou nas paredes) para liberar a sala, fuja de quem quer te atrasar", FONTE_TEXTO, largura);
    y -= 24.0f;
    escreveCentrado(y, "e chegue a prova antes do tempo acabar.", FONTE_TEXTO, largura);

    y -= 46.0f;
    glColor3f(0.75f, 0.85f, 1.0f);
    escreveCentrado(y, "COMANDOS", FONTE_TEXTO, largura);

    // Tabela de comandos alinhada à esquerda de um bloco centralizado.
    static const char *comandos[] = {
        "W A S D ou setas   andar e girar",
        "Mouse              olhar em volta",
        "E                  entrar na sala / carinho no gato (o cinza te guia)",
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

// Tarja escura translúcida atrás de um texto.
static void tarja(float x0, float y0, float x1, float y1) {
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glBegin(GL_QUADS);
    glVertex2f(x0, y0); glVertex2f(x1, y0); glVertex2f(x1, y1); glVertex2f(x0, y1);
    glEnd();
}

// Texto centrado na horizontal, na altura y, na cor (r, g, b), com a tarja atrás.
// A cor vem depois da tarja: glColor4f da tarja deixaria o texto preto.
static void textoComTarja(float y, const char *texto, int largura, float r, float g, float b) {
    int w = glutBitmapLength(FONTE_TEXTO, (const unsigned char *) texto);
    tarja((largura - w) / 2.0f - 12.0f, y - 10.0f, (largura + w) / 2.0f + 12.0f, y + 22.0f);
    glColor3f(r, g, b);
    escreveCentrado(y, texto, FONTE_TEXTO, largura);
}

// Indicador da frequência no canto de cima, à esquerda: "Frequencia: 1/4" ...
// "Frequencia: 4/4 - sala liberada!" (em verde) quando a porta da prova já abre.
static void desenhaIndicadorFrequencia(int altura) {
    if (jogo.cadernosTotal <= 0) return;
    char texto[64];
    bool completa = todosCadernosColetados();
    if (completa) sprintf(texto, "Frequencia: %d/%d - sala liberada!", jogo.cadernosColetados, jogo.cadernosTotal);
    else          sprintf(texto, "Frequencia: %d/%d", jogo.cadernosColetados, jogo.cadernosTotal);

    int w = glutBitmapLength(FONTE_TEXTO, (const unsigned char *) texto);
    float y = altura - 36.0f;
    tarja(12.0f, y - 10.0f, 36.0f + w, y + 22.0f);
    if (completa) glColor3f(0.45f, 1.0f, 0.45f);
    else          glColor3f(1.0f, 1.0f, 1.0f);
    escreve(24.0f, y, texto, FONTE_TEXTO);
}

// ---------------------------------------------------------------- barra de felicidade

#define BARRA_LARGURA 180.0f
#define BARRA_ALTURA   14.0f
#define TEMPO_DELTA     1.5f   // segundos que o "+15" / "-30" fica ao lado da barra

static float felicidadeExibida = -1.0f;   // valor desenhado: persegue jogo.felicidade (animação)
static float felicidadeVista = -1.0f;     // último valor lido, para perceber as mudanças
static float ultimoDelta = 0.0f, tempoDelta = 0.0f;
static int relogioBarra = -1;             // milissegundos do último quadro desenhado

// Cor da barra pela fração cheia: vermelho (0) -> amarelo (0,5) -> verde (1).
static void corFelicidade(float f) {
    if (f < 0.5f) {
        float t = f / 0.5f;
        glColor3f(0.90f + 0.05f * t, 0.20f + 0.65f * t, 0.15f + 0.05f * t);
    } else {
        float t = (f - 0.5f) / 0.5f;
        glColor3f(0.95f - 0.60f * t, 0.85f + 0.05f * t, 0.20f + 0.15f * t);
    }
}

static void retangulo(float x0, float y0, float x1, float y1) {
    glBegin(GL_QUADS);
    glVertex2f(x0, y0); glVertex2f(x1, y0); glVertex2f(x1, y1); glVertex2f(x0, y1);
    glEnd();
}

// "Felicidade [=======   ]" abaixo do indicador da frequência. O preenchimento anda até o
// valor novo em vez de pular, e a mudança aparece por um instante ao lado ("+15" ou "-30").
static void desenhaBarraFelicidade(int altura) {
    int agora = glutGet(GLUT_ELAPSED_TIME);
    float dt = (relogioBarra < 0) ? 0.0f : (agora - relogioBarra) / 1000.0f;
    relogioBarra = agora;

    if (felicidadeVista < 0.0f) felicidadeExibida = felicidadeVista = jogo.felicidade;
    if (jogo.felicidade != felicidadeVista) {
        ultimoDelta = jogo.felicidade - felicidadeVista;
        tempoDelta = TEMPO_DELTA;
        felicidadeVista = jogo.felicidade;
    }
    float passo = 6.0f * dt;   // aproxima ~6x a diferença por segundo
    if (passo > 1.0f) passo = 1.0f;
    felicidadeExibida += (jogo.felicidade - felicidadeExibida) * passo;
    if (tempoDelta > 0.0f) tempoDelta -= dt;

    const char *rotulo = "Felicidade";
    int wRotulo = glutBitmapLength(FONTE_TEXTO, (const unsigned char *) rotulo);
    float y = altura - 76.0f;                         // uma linha abaixo da frequência
    float x0 = 24.0f + wRotulo + 12.0f, x1 = x0 + BARRA_LARGURA;
    float fracao = felicidadeExibida / FELICIDADE_MAX;

    tarja(12.0f, y - 10.0f, x1 + 12.0f, y + 22.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    escreve(24.0f, y, rotulo, FONTE_TEXTO);

    glColor3f(0.15f, 0.15f, 0.15f);                   // fundo da barra (parte vazia)
    retangulo(x0, y - 1.0f, x1, y - 1.0f + BARRA_ALTURA);
    corFelicidade(fracao);                            // parte cheia
    retangulo(x0, y - 1.0f, x0 + BARRA_LARGURA * fracao, y - 1.0f + BARRA_ALTURA);
    glColor3f(0.85f, 0.85f, 0.85f);                   // contorno
    glBegin(GL_LINE_LOOP);
    glVertex2f(x0, y - 1.0f); glVertex2f(x1, y - 1.0f);
    glVertex2f(x1, y - 1.0f + BARRA_ALTURA); glVertex2f(x0, y - 1.0f + BARRA_ALTURA);
    glEnd();

    if (tempoDelta > 0.0f) {
        char texto[16];
        sprintf(texto, "%+.0f", ultimoDelta);
        if (ultimoDelta > 0.0f) glColor3f(0.45f, 1.0f, 0.45f);
        else                    glColor3f(1.0f, 0.45f, 0.40f);
        escreve(x1 + 22.0f, y, texto, FONTE_TEXTO);
    }
}

// Durante a partida: indicador da frequência, aviso de folha coletada, escurecimento ao
// passar pela porta da sala de aula e a dica "[E] Entrar na sala de aula" perto dela.
static void desenhaDicasJogo(int largura, int altura) {
    float escuro = escurecimentoSala();
    const char *dica = dicaPortaSala();
    const char *aviso = mensagemFrequencia();

    inicio2D(largura, altura);
    if (escuro > 0.0f) escureceFundo(largura, altura, escuro);
    desenhaIndicadorFrequencia(altura);
    desenhaBarraFelicidade(altura);
    if (aviso) textoComTarja(altura - 110.0f, aviso, largura, 0.45f, 1.0f, 0.45f);
    if (dica)  textoComTarja(70.0f, dica, largura, 1.0f, 0.9f, 0.4f);
    fim2D();
}

void desenhaTelas(int larguraJanela, int alturaJanela) {
    if (jogo.estado == JOGO_JOGANDO) { desenhaDicasJogo(larguraJanela, alturaJanela); return; }
    felicidadeVista = -1.0f;   // fora da partida: ao recomeçar, a barra parte do valor novo sem "+60"
    relogioBarra = -1;
    if (jogo.estado == JOGO_PROVA) return;

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
                           "Voce chegou a prova com a frequencia completa.");
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
