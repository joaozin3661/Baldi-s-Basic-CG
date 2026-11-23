#include "frequencia.h"
#include "mapa.h"
#include "caminho.h"
#include "jogo.h"
#include "jogador.h"
#include "texturas.h"
#include "gl_includes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

#define FREQ_LARGURA        0.30f   // folha A4 em pé: 0,30 x 0,42 unidades
#define FREQ_ALTURA         0.42f
#define FREQ_ALTURA_PAREDE  1.35f   // centro da folha presa na parede (olhos a 1,5)
#define FREQ_AFASTA         0.012f  // distância do chão/parede, contra z-fighting
#define FREQ_ALCANCE        0.90f   // distância do jogador à folha para coletar
#define FREQ_LONGE_INICIO   5       // distância mínima (em células, Manhattan) até o 'S'
#define FREQ_TEMPO_MENSAGEM 2.5f    // segundos da mensagem na tela

struct Folha {
    float x, y, z;      // centro da folha no mundo
    float angulo;       // giro em torno de y (graus)
    bool naParede;      // presa na parede (em pé) ou caída no chão (deitada)
    bool coletada;
    int linha, coluna;  // célula de corredor da folha (a da parede fica na borda dela)
};

static Folha folhas[FREQ_TOTAL];
static int quantFolhas = 0;
static GLuint listaFolha = 0;
static float tempoMensagem = 0.0f;
static char mensagem[64];
static float relogio = 0.0f;   // para o brilho pulsante

// ---------------------------------------------------------------- desenho

// Folha no plano local (x para a direita, y para cima, normal +z), centrada na origem:
// o papel com a textura (lista de presença) e "FREQUENCIA" escrito com a fonte stroke.
static void montaFolha() {
    float mx = FREQ_LARGURA / 2, my = FREQ_ALTURA / 2;
    glBindTexture(GL_TEXTURE_2D, idTexturas[TEX_FREQUENCIA]);
    glNormal3f(0, 0, 1);
    glBegin(GL_QUADS);   // anti-horário visto de frente
    glTexCoord2f(0, 0); glVertex3f(-mx, -my, 0);
    glTexCoord2f(1, 0); glVertex3f( mx, -my, 0);
    glTexCoord2f(1, 1); glVertex3f( mx,  my, 0);
    glTexCoord2f(0, 1); glVertex3f(-mx,  my, 0);
    glEnd();

    // Título, na faixa clara do topo da textura.
    glDisable(GL_TEXTURE_2D);
    GLfloat tinta[] = { 0.10f, 0.12f, 0.35f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, tinta);
    const char *titulo = "FREQUENCIA";
    float largura = 0;
    for (const char *p = titulo; *p; p++) largura += glutStrokeWidth(GLUT_STROKE_ROMAN, *p);
    float escala = (FREQ_LARGURA * 0.8f) / largura;
    glPushMatrix();
    glTranslatef(-largura * escala / 2, my - 0.065f, 0.002f);
    glScalef(escala, escala, 1.0f);   // z fica em 1 para não estragar a normal
    glLineWidth(2.0f);
    for (const char *p = titulo; *p; p++) glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
    glLineWidth(1.0f);
    glPopMatrix();
    GLfloat branco[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, branco);
    glEnable(GL_TEXTURE_2D);
}

void iniciaFrequencias() {
    listaFolha = glGenLists(1);
    glNewList(listaFolha, GL_COMPILE);
    montaFolha();
    glEndList();
}

void desenhaFrequencias() {
    // Brilho próprio (emissão) pulsando devagar: a folha aparece mesmo longe da lanterna.
    float brilho = 0.30f + 0.15f * sinf(relogio * 4.0f);
    GLfloat emissao[] = { brilho, brilho, brilho * 0.85f, 1.0f };
    GLfloat semEmissao[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, emissao);

    for (int i = 0; i < quantFolhas; i++) {
        const Folha &f = folhas[i];
        if (f.coletada) continue;
        glPushMatrix();
        glTranslatef(f.x, f.y, f.z);
        glRotatef(f.angulo, 0.0f, 1.0f, 0.0f);
        if (!f.naParede) glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);   // deita: a normal +z vira +y
        glCallList(listaFolha);
        glPopMatrix();
    }
    glMaterialfv(GL_FRONT, GL_EMISSION, semEmissao);
}

// ---------------------------------------------------------------- sorteio

static float sorteio01() { return (float) rand() / RAND_MAX; }

// Célula onde uma folha pode ficar: corredor comum, fora da sala da prova e longe da entrada.
static bool celulaCandidata(int l, int c, int lS, int cS) {
    if (celulaSolida(l, c)) return false;
    char ch = celula(l, c);
    if (ch == CEL_SALA || ch == CEL_INICIO) return false;
    return abs(l - lS) + abs(c - cS) >= FREQ_LONGE_INICIO;
}

// Parede onde dá para prender uma folha: tijolo ou armário (porta e placas ficam livres).
static bool paredeLisa(int l, int c) {
    char ch = celula(l, c);
    return ch == CEL_PAREDE || ch == CEL_ARMARIO;
}

// Põe a folha i na célula (l, c): presa numa parede vizinha (se houver e o sorteio mandar)
// ou caída no chão, num ponto e num ângulo aleatórios da célula.
static void posicionaFolha(Folha *f, int l, int c) {
    // Vizinhos N, S, O, L: posição da parede e ângulo que vira a normal da folha para o corredor.
    const int dl[4] = { -1, 1, 0, 0 }, dc[4] = { 0, 0, -1, 1 };
    const float angulo[4] = { 0.0f, 180.0f, 90.0f, -90.0f };
    int paredes[4], quant = 0;
    for (int d = 0; d < 4; d++)
        if (paredeLisa(l + dl[d], c + dc[d])) paredes[quant++] = d;

    float cx, cz;
    centroDaCelula(l, c, &cx, &cz);
    float desvio = (sorteio01() - 0.5f) * 0.8f * ESCALA_MAPA;   // até 0,4 x ESCALA_MAPA ao longo da parede
    f->coletada = false;
    f->linha = l;
    f->coluna = c;

    if (quant > 0 && sorteio01() < 0.5f) {
        int d = paredes[rand() % quant];
        float meio = TAM_CELULA / 2 - FREQ_AFASTA;
        f->naParede = true;
        f->angulo = angulo[d];
        f->y = FREQ_ALTURA_PAREDE;
        if (dl[d] != 0) { f->x = cx + desvio; f->z = cz + dl[d] * meio; }   // parede norte/sul
        else            { f->z = cz + desvio; f->x = cx + dc[d] * meio; }   // parede oeste/leste
    } else {
        f->naParede = false;
        f->angulo = sorteio01() * 360.0f;
        f->y = FREQ_AFASTA;
        f->x = cx + desvio;
        f->z = cz + (sorteio01() - 0.5f) * 0.8f * ESCALA_MAPA;
    }
}

void sorteiaFrequencias() {
    int lS = 1, cS = 1;
    for (int l = 0; l < MAPA_LINHAS; l++)
        for (int c = 0; c < MAPA_COLUNAS; c++)
            if (celula(l, c) == CEL_INICIO) { lS = l; cS = c; }

    // Candidatas em ordem aleatória (embaralhamento de Fisher-Yates).
    static int cand[MAPA_LINHAS * MAPA_COLUNAS][2];
    int n = 0;
    for (int l = 0; l < MAPA_LINHAS; l++)
        for (int c = 0; c < MAPA_COLUNAS; c++)
            if (celulaCandidata(l, c, lS, cS)) { cand[n][0] = l; cand[n][1] = c; n++; }
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tl = cand[i][0], tc = cand[i][1];
        cand[i][0] = cand[j][0]; cand[i][1] = cand[j][1];
        cand[j][0] = tl;         cand[j][1] = tc;
    }

    // Escolhe folhas afastadas umas das outras, para obrigar a atravessar o mapa.
    // Se o mapa não comportar a distância, ela diminui até caber.
    int escolhidas[FREQ_TOTAL][2];
    quantFolhas = 0;
    for (int distancia = 8; distancia >= 0 && quantFolhas < FREQ_TOTAL; distancia--) {
        quantFolhas = 0;
        for (int i = 0; i < n && quantFolhas < FREQ_TOTAL; i++) {
            bool longe = true;
            for (int k = 0; k < quantFolhas; k++)
                if (abs(cand[i][0] - escolhidas[k][0]) + abs(cand[i][1] - escolhidas[k][1]) < distancia)
                    longe = false;
            if (!longe) continue;
            escolhidas[quantFolhas][0] = cand[i][0];
            escolhidas[quantFolhas][1] = cand[i][1];
            quantFolhas++;
        }
    }

    for (int i = 0; i < quantFolhas; i++) {
        posicionaFolha(&folhas[i], escolhidas[i][0], escolhidas[i][1]);
        printf("Frequencia %d: celula (%d, %d), %s\n", i + 1, escolhidas[i][0], escolhidas[i][1],
               folhas[i].naParede ? "na parede" : "no chao");
    }
    jogo.cadernosTotal = quantFolhas;
    jogo.cadernosColetados = 0;
    tempoMensagem = 0.0f;
}

// ---------------------------------------------------------------- coleta

void atualizaFrequencias(float dt) {
    relogio += dt;
    if (tempoMensagem > 0.0f) tempoMensagem -= dt;
    if (jogo.estado != JOGO_JOGANDO) return;

    for (int i = 0; i < quantFolhas; i++) {
        Folha &f = folhas[i];
        if (f.coletada) continue;
        float dx = jogador.x - f.x, dz = jogador.z - f.z;   // distância no chão
        if (dx * dx + dz * dz > FREQ_ALCANCE * FREQ_ALCANCE) continue;

        f.coletada = true;
        jogo.cadernosColetados++;
        mudaFelicidade(-FELICIDADE_POR_FREQUENCIA);   // estudar cansa: a felicidade cai
        if (todosCadernosColetados())
            sprintf(mensagem, "Frequencia completa! A sala da prova esta liberada");
        else
            sprintf(mensagem, "Frequencia coletada! %d/%d", jogo.cadernosColetados, jogo.cadernosTotal);
        tempoMensagem = FREQ_TEMPO_MENSAGEM;
        printf("%s\n", mensagem);
    }
}

const char *mensagemFrequencia() {
    return tempoMensagem > 0.0f ? mensagem : NULL;
}

int quantidadeFrequencias() {
    return quantFolhas;
}

bool celulaFrequenciaPendente(int i, int *linha, int *coluna) {
    if (i < 0 || i >= quantFolhas || folhas[i].coletada) return false;
    *linha = folhas[i].linha;
    *coluna = folhas[i].coluna;
    return true;
}
