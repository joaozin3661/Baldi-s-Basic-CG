// Labirinto da Faculdade - Trabalho Final de Computação Gráfica (UFPI)
// Fase 1: labirinto 3D, câmera FPS, colisão, iluminação, texturas e visibilidade.
//
// Controles: W A S D ou setas = andar/girar (agarrado: apertar várias vezes para se soltar)
//            mouse = olhar | E = entrar/sair da sala de aula (perto da porta) ou acariciar um gato
//            M = soltar/prender o mouse | ESC = sair

#include "gl_includes.h"
#include "mapa.h"
#include "jogador.h"
#include "texturas.h"
#include "jogo.h"
#include "perseguidor.h"
#include "gato.h"
#include "telas.h"
#include "sala.h"
#include "frequencia.h"
#include <cctype>
#include <cstdlib>
#include <ctime>

static int larguraJanela = 1024, alturaJanela = 600;

static bool teclas[256];          // teclas comuns pressionadas no momento
static bool especiais[256];       // setas e outras teclas especiais
static bool mousePreso = true;    // mouse controla a câmera
static int tempoAnterior = 0;     // em milissegundos

#define SENSIBILIDADE_MOUSE 0.0025f   // radianos por pixel

// ---------------------------------------------------------------- iluminação

static void iniciaIluminacao() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);   // sombreamento de Gouraud: interpola a cor entre os vértices

    // Luz ambiente fraca: o que a lanterna não alcança fica escuro, mas não preto.
    GLfloat ambienteGlobal[] = { 0.12f, 0.12f, 0.14f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambienteGlobal);

    // GL_LIGHT0 é a luz que acompanha o jogador.
    GLfloat difusa[]    = { 1.0f, 0.95f, 0.85f, 1.0f };
    GLfloat especular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, difusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, especular);

    // Atenuação: a luz enfraquece com a distância d, por 1 / (kc + kl*d + kq*d*d).
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
    // Divididos pela escala do mapa: com blocos maiores, a luz alcança as mesmas células de antes.
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.10f / ESCALA_MAPA);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.04f / (ESCALA_MAPA * ESCALA_MAPA));

    // Material único para o cenário: branco, para a textura definir a cor (GL_MODULATE).
    GLfloat matDifusa[]    = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat matEspecular[] = { 0.15f, 0.15f, 0.15f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matDifusa);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matEspecular);
    glMaterialf(GL_FRONT, GL_SHININESS, 16.0f);
}

// ---------------------------------------------------------------- desenho

static void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   // limpa a imagem e o z-buffer

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    aplicaCamera();

    // A posição da luz é transformada pela matriz atual, por isso é definida depois da câmera.
    GLfloat posicaoLuz[] = { jogador.x, JOGADOR_ALTURA, jogador.z, 1.0f };   // w = 1: luz pontual
    glLightfv(GL_LIGHT0, GL_POSITION, posicaoLuz);

    desenhaMapa();
    desenhaSala();
    desenhaFrequencias();
    desenhaPerseguidor();
    desenhaGatos();
    desenhaTelas(larguraJanela, alturaJanela);   // menu e telas de fim, por cima da cena

    glutSwapBuffers();
}

static void reshape(int largura, int altura) {
    if (altura == 0) altura = 1;
    larguraJanela = largura;
    alturaJanela = altura;

    glViewport(0, 0, largura, altura);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(65.0, (double) largura / altura, 0.05, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// ---------------------------------------------------------------- atualização

static void atualiza() {
    int agora = glutGet(GLUT_ELAPSED_TIME);
    float dt = (agora - tempoAnterior) / 1000.0f;   // segundos desde o último quadro
    tempoAnterior = agora;
    if (dt > 0.05f) dt = 0.05f;   // evita saltos grandes se o jogo travar por um instante

    atualizaJogo(dt);
    atualizaSala(dt);   // escurecimento da passagem pela porta
    atualizaFrequencias(dt);   // coleta das folhas de frequência
    if (jogo.estado != JOGO_JOGANDO) {   // nas telas de menu, prova e fim o jogador não anda
        glutPostRedisplay();
        return;
    }

    // Nos braços do perseguidor o aluno não anda: é ele quem leva o aluno
    // (atualizaPerseguidor move os dois); as teclas de andar viram "se debater".
    if (!perseguidorAgarrouJogador()) {
        float frente = 0.0f, lado = 0.0f;
        if (teclas['w'] || especiais[GLUT_KEY_UP])   frente += 1.0f;
        if (teclas['s'] || especiais[GLUT_KEY_DOWN]) frente -= 1.0f;
        if (teclas['d']) lado += 1.0f;
        if (teclas['a']) lado -= 1.0f;
        moveJogador(frente, lado, dt);
    }

    if (especiais[GLUT_KEY_RIGHT]) giraJogador(JOGADOR_GIRO * dt, 0.0f);
    if (especiais[GLUT_KEY_LEFT])  giraJogador(-JOGADOR_GIRO * dt, 0.0f);

    // NPCs só andam durante a partida (o return acima os segura nas telas).
    atualizaPerseguidor(dt);
    atualizaGatos(dt);

    glutPostRedisplay();
}

// ---------------------------------------------------------------- entrada

static void prendeMouse(bool prender) {
    mousePreso = prender;
    glutSetCursor(prender ? GLUT_CURSOR_NONE : GLUT_CURSOR_INHERIT);
    if (prender) glutWarpPointer(larguraJanela / 2, alturaJanela / 2);
}

static void teclaPressionada(unsigned char tecla, int, int) {
    tecla = (unsigned char) tolower(tecla);
    teclas[tecla] = true;
    if (tecla == 27) exit(0);                  // ESC
    if (teclaTelas(tecla)) {                   // uma tela está aberta: ela decide o que fazer
        prendeMouse(jogo.estado == JOGO_JOGANDO);   // ao começar a partida, o mouse vira câmera
        return;
    }
    if (tecla == 'm') prendeMouse(!mousePreso);
    // Tecla de interação: perto da porta da sala de aula entra/sai; senão, carinho no gato.
    if (tecla == 'e' && !interagePortaSala()) interageComGato();
    if (tecla == 'w' || tecla == 'a' || tecla == 's' || tecla == 'd')
        sacodeJogador();                       // agarrado: cada aperto conta para se soltar
}

static void teclaSolta(unsigned char tecla, int, int) {
    teclas[(unsigned char) tolower(tecla)] = false;
}

static void especialPressionada(int tecla, int, int) {
    if (tecla >= 0 && tecla < 256) especiais[tecla] = true;
    if (tecla == GLUT_KEY_UP || tecla == GLUT_KEY_DOWN) sacodeJogador();
}

static void especialSolta(int tecla, int, int) {
    if (tecla >= 0 && tecla < 256) especiais[tecla] = false;
}

// O mouse é devolvido ao centro da janela a cada movimento; o deslocamento
// em relação ao centro diz quanto a câmera deve girar.
static void mouseMovido(int x, int y) {
    if (!mousePreso || jogo.estado != JOGO_JOGANDO) return;   // nas telas o mouse fica livre
    int centroX = larguraJanela / 2, centroY = alturaJanela / 2;
    int dx = x - centroX, dy = y - centroY;
    if (dx == 0 && dy == 0) return;   // é o evento gerado pelo próprio glutWarpPointer

    giraJogador(dx * SENSIBILIDADE_MOUSE, -dy * SENSIBILIDADE_MOUSE);
    glutWarpPointer(centroX, centroY);
}

// ---------------------------------------------------------------- principal

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);   // GLUT_DEPTH aloca o z-buffer
    glutInitWindowSize(larguraJanela, alturaJanela);
    glutCreateWindow("Labirinto da Faculdade");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Visibilidade (aula 25)
    glEnable(GL_DEPTH_TEST);   // z-buffer: só o fragmento mais próximo aparece
    glEnable(GL_CULL_FACE);    // back-face culling: descarta faces de costas para a câmera
    glCullFace(GL_BACK);

    iniciaIluminacao();
    iniciaTexturas();
    iniciaMapa();
    iniciaSala();          // depois do mapa: procura a porta 'P' e as células 'F'
    iniciaFrequencias();
    // As texturas usam semente fixa (srand(7)); daqui em diante, o sorteio muda a cada execução
    // (posição das folhas de frequência, passeio do gato e do perseguidor).
    srand((unsigned) time(NULL));
    iniciaJogador();
    iniciaJogo();
    iniciaPerseguidor();   // depois de iniciaJogo(): escreve em jogo.perseguidor*
    iniciaGatos();
    // A partida só começa quando o jogador confirma os comandos na tela de início (telas.cpp).

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(atualiza);
    glutKeyboardFunc(teclaPressionada);
    glutKeyboardUpFunc(teclaSolta);
    glutSpecialFunc(especialPressionada);
    glutSpecialUpFunc(especialSolta);
    glutPassiveMotionFunc(mouseMovido);
    glutMotionFunc(mouseMovido);
    glutIgnoreKeyRepeat(1);   // sem repetição automática: só importam "apertou" e "soltou"

    prendeMouse(false);   // o jogo abre no menu, com o mouse livre
    tempoAnterior = glutGet(GLUT_ELAPSED_TIME);

    glutMainLoop();
    return 0;
}
