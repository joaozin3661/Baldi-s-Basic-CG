#include "gato.h"
#include "caminho.h"
#include "mapa.h"
#include "jogador.h"
#include "jogo.h"
#include "frequencia.h"
#include "gl_includes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

Gato gatos[NUM_GATOS];

#define GRAUS_POR_RADIANO 57.29578f

static const char *nomeTipo(TipoGato t) {
    return t == GATO_GUIA ? "Gato guia" : "Gato";
}

static const char *nomeEstado(EstadoGato e) {
    switch (e) {
        case GATO_VAGANDO:    return "vagando";
        case GATO_PARADO:     return "parado";
        case GATO_ACARICIADO: return "acariciado";
        case GATO_CORRENDO:   return "correndo";
        case GATO_INDO:       return "indo";
    }
    return "?";
}

// ---------------------------------------------------------------- percepção

static float distanciaAoJogador(const Gato &g) {
    float dx = jogador.x - g.x, dz = jogador.z - g.z;
    return sqrtf(dx * dx + dz * dz);
}

// Colisão esfera x esfera (aula 21): há contato quando a distância entre os centros
// é menor que a soma dos raios. Para o carinho, o "raio" do gato é o alcance da mão.
static bool jogadorAoAlcance(const Gato &g) {
    return distanciaAoJogador(g) < GATO_ALCANCE_CARINHO + JOGADOR_RAIO;
}

static bool encostaNoJogador(const Gato &g) {
    return distanciaAoJogador(g) < GATO_RAIO + JOGADOR_RAIO;
}

// O aluno vê o gato? Três testes, do mais barato ao mais caro:
//  1. distância: além de GATO_DIST_VISIVEL a lanterna (luz com atenuação) quase não o ilumina;
//  2. campo de visão: o ângulo entre a direção do olhar da câmera e a direção até o gato tem de
//     ser menor que meio campo de visão horizontal (produto escalar: cos do ângulo = f . d / |d|);
//  3. oclusão: nenhuma parede entre os dois (a mesma linha de visão do perseguidor).
static bool jogadorVeGato(const Gato &g) {
    float dx = g.x - jogador.x, dz = g.z - jogador.z;
    float d = sqrtf(dx * dx + dz * dz);
    if (d < 0.001f) return true;
    if (d > GATO_DIST_VISIVEL) return false;

    float frenteX = sinf(jogador.yaw), frenteZ = -cosf(jogador.yaw);   // mesma conta de aplicaCamera()
    float cosAngulo = (dx * frenteX + dz * frenteZ) / d;
    if (cosAngulo < cosf(GATO_MEIO_CAMPO_VISAO / GRAUS_POR_RADIANO)) return false;

    return haLinhaDeVisao(jogador.x, jogador.z, g.x, g.z);
}

// ---------------------------------------------------------------- frequência (gato guia)

// Escolhe a folha pendente mais próxima pelo caminho nos corredores (BFS), não em linha reta:
// uma folha logo atrás da parede pode estar a muitas células de distância.
static bool escolheFrequencia(Gato &g) {
    int l, c;
    celulaDoMundo(g.x, g.z, &l, &c);

    int melhor = -1, menorDistancia = 0;
    for (int i = 0; i < quantidadeFrequencias(); i++) {
        int lf, cf;
        if (!celulaFrequenciaPendente(i, &lf, &cf)) continue;
        int d = distanciaNaGrade(l, c, lf, cf);
        if (d < 0) continue;   // sem caminho
        if (melhor < 0 || d < menorDistancia) {
            melhor = i;
            menorDistancia = d;
            g.lDestino = lf;
            g.cDestino = cf;
        }
    }
    g.frequenciaAlvo = melhor;
    if (melhor >= 0)
        printf("Gato guia: frequencia mais proxima em (%d, %d), a %d celulas\n",
               g.lDestino, g.cDestino, menorDistancia);
    return melhor >= 0;
}

// A folha para onde o guia corre ainda está lá? O aluno pode tê-la pego antes dele, ou a
// partida pode ter recomeçado com folhas novas (por isso a célula também é conferida).
static bool destinoValido(const Gato &g) {
    int l, c;
    return celulaFrequenciaPendente(g.frequenciaAlvo, &l, &c) && l == g.lDestino && c == g.cDestino;
}

// ---------------------------------------------------------------- máquina de estados

static void trocaEstado(Gato &g, EstadoGato novo) {
    if (g.estado == novo) return;
    printf("%s: %s -> %s\n", nomeTipo(g.tipo), nomeEstado(g.estado), nomeEstado(novo));
    g.estado = novo;
    g.tempoNoEstado = 0.0f;
}

// Faz o gato ir primeiro ao centro da célula em que está; ao chegar, move() decide o resto.
static void miraCelulaAtual(Gato &g) {
    celulaDoMundo(g.x, g.z, &g.lAlvo, &g.cAlvo);
}

static void voltaAVagar(Gato &g) {
    g.frequenciaAlvo = -1;
    trocaEstado(g, GATO_VAGANDO);
    miraCelulaAtual(g);
}

// Todas as transições por tempo ou percepção ficam aqui. A transição para ACARICIADO vem da
// tecla, em interageComGato(), e a chegada à frequência vem do movimento, em move().
static void transita(Gato &g) {
    switch (g.estado) {
        case GATO_VAGANDO:
            break;   // sai daqui ao chegar numa célula (em move) ou pela tecla

        case GATO_PARADO:
            if (g.tempoNoEstado >= g.tempoParado) trocaEstado(g, GATO_VAGANDO);
            break;

        case GATO_ACARICIADO:
            if (g.tipo == GATO_COMUM) {
                if (g.tempoNoEstado >= GATO_TEMPO_CARINHO) trocaEstado(g, GATO_VAGANDO);
            } else if (g.tempoNoEstado >= GATO_GUIA_TEMPO_CARINHO) {
                if (escolheFrequencia(g)) {   // dispara até a folha mais próxima
                    trocaEstado(g, GATO_CORRENDO);
                    miraCelulaAtual(g);
                } else {
                    printf("Gato guia: nenhuma frequencia pendente\n");
                    voltaAVagar(g);
                }
            }
            break;

        case GATO_CORRENDO:
        case GATO_INDO:
            if (!destinoValido(g) && !escolheFrequencia(g)) {   // a folha sumiu e não há outra
                printf("Gato guia: nenhuma frequencia pendente\n");
                voltaAVagar(g);
                break;
            }
            // Sumiu da vista do aluno: muda de estado (aparece no console), mas segue em disparada.
            if (g.estado == GATO_CORRENDO && !jogadorVeGato(g)) trocaEstado(g, GATO_INDO);
            break;
    }
}

bool interageComGato() {
    if (jogo.estado != JOGO_JOGANDO) return false;

    // O gato mais próximo ao alcance. O guia em disparada (ou já acariciado) não para.
    Gato *escolhido = NULL;
    float menor = 0.0f;
    for (int i = 0; i < NUM_GATOS; i++) {
        Gato &g = gatos[i];
        if (!jogadorAoAlcance(g)) continue;
        if (g.tipo == GATO_GUIA && g.estado != GATO_VAGANDO && g.estado != GATO_PARADO) continue;
        float d = distanciaAoJogador(g);
        if (!escolhido || d < menor) { escolhido = &g; menor = d; }
    }
    if (!escolhido) return false;

    Gato &g = *escolhido;
    if (g.estado == GATO_ACARICIADO) {   // gato comum já no colo: só prolonga, sem felicidade extra
        g.tempoNoEstado = 0.0f;
        return true;
    }

    g.carinhos++;
    printf("Miau! %s acariciado (%d)\n", nomeTipo(g.tipo), g.carinhos);
    mudaFelicidade(FELICIDADE_POR_CARINHO);
    g.yaw = atan2f(jogador.x - g.x, -(jogador.z - g.z));   // vira para o jogador
    trocaEstado(g, GATO_ACARICIADO);
    return true;
}

// ---------------------------------------------------------------- movimento

// Vagando, ao chegar numa célula: às vezes senta, senão sorteia uma direção (estado 1 do Pacman).
static void escolheProximaCelula(Gato &g) {
    int l, c;
    celulaDoMundo(g.x, g.z, &l, &c);

    if ((float) rand() / RAND_MAX < GATO_CHANCE_PARAR) {
        g.tempoParado = 1.0f + 2.0f * ((float) rand() / RAND_MAX);   // de 1 a 3 segundos
        trocaEstado(g, GATO_PARADO);
        g.lAlvo = l;
        g.cAlvo = c;
        return;
    }

    int lProx, cProx;
    if (sorteiaVizinho(l, c, g.lAnterior, g.cAnterior, &lProx, &cProx)) {
        g.lAnterior = l;
        g.cAnterior = c;
        g.lAlvo = lProx;
        g.cAlvo = cProx;
    }
}

// Guia, ao chegar numa célula: se é a da folha, terminou; senão, o próximo passo da BFS.
static void avancaParaFrequencia(Gato &g) {
    int l, c;
    celulaDoMundo(g.x, g.z, &l, &c);

    if (l == g.lDestino && c == g.cDestino) {
        printf("Gato guia: chegou na frequencia (%d, %d)\n", l, c);
        voltaAVagar(g);
        escolheProximaCelula(g);
        return;
    }

    int lProx, cProx;
    if (proximoPasso(l, c, g.lDestino, g.cDestino, &lProx, &cProx)) {
        g.lAnterior = l;
        g.cAnterior = c;
        g.lAlvo = lProx;
        g.cAlvo = cProx;
    } else {
        voltaAVagar(g);   // sem caminho (não deveria acontecer num labirinto conexo)
    }
}

// Anda em direção a um ponto; testa cada eixo separado para deslizar na parede.
// Retorna verdadeiro se chegou.
static bool andaPara(Gato &g, float xAlvo, float zAlvo, float dt) {
    float dx = xAlvo - g.x, dz = zAlvo - g.z;
    float distancia = sqrtf(dx * dx + dz * dz);
    float passo = g.velocidade * dt;

    if (distancia <= passo) {
        g.x = xAlvo;
        g.z = zAlvo;
        return true;
    }

    dx *= passo / distancia;
    dz *= passo / distancia;
    g.yaw = atan2f(dx, -dz);
    g.faseAnimacao += passo * 8.0f;

    if (!colideComMapa(g.x + dx, g.z, GATO_RAIO)) g.x += dx;
    if (!colideComMapa(g.x, g.z + dz, GATO_RAIO)) g.z += dz;
    return false;
}

static void move(Gato &g, float dt) {
    float xAlvo, zAlvo;
    centroDaCelula(g.lAlvo, g.cAlvo, &xAlvo, &zAlvo);

    switch (g.estado) {
        case GATO_VAGANDO:
            if (encostaNoJogador(g)) return;   // não atravessa o jogador: espera ele sair
            g.velocidade = GATO_VELOCIDADE;
            if (andaPara(g, xAlvo, zAlvo, dt)) escolheProximaCelula(g);
            break;

        case GATO_CORRENDO:
        case GATO_INDO:   // em disparada não desvia do aluno: passa por ele
            g.velocidade = GATO_GUIA_VEL_CORRIDA;
            if (andaPara(g, xAlvo, zAlvo, dt)) avancaParaFrequencia(g);
            break;

        default:
            break;   // sentado ou recebendo carinho: fica parado
    }
}

// ---------------------------------------------------------------- interface

static void iniciaUmGato(Gato &g, TipoGato tipo, char simbolo, int lReserva, int cReserva) {
    g.tipo = tipo;
    g.x = g.z = -1.0f;
    for (int l = 0; l < MAPA_LINHAS && g.x < 0.0f; l++)
        for (int c = 0; c < MAPA_COLUNAS; c++)
            if (celula(l, c) == simbolo) {
                centroDaCelula(l, c, &g.x, &g.z);
                break;
            }
    if (g.x < 0.0f) {   // mapa sem o símbolo: nasce na célula de reserva
        centroDaCelula(lReserva, cReserva, &g.x, &g.z);
        printf("Aviso: mapa sem '%c'; %s nasce em (%d, %d)\n", simbolo, nomeTipo(tipo), lReserva, cReserva);
    }

    g.yaw = 0.0f;
    g.estado = GATO_VAGANDO;
    g.velocidade = GATO_VELOCIDADE;
    g.tempoNoEstado = 0.0f;
    g.tempoParado = 0.0f;
    g.faseAnimacao = 0.0f;
    g.carinhos = 0;
    g.frequenciaAlvo = -1;
    g.lDestino = g.cDestino = -1;
    g.lAnterior = g.cAnterior = -1;
    miraCelulaAtual(g);
    escolheProximaCelula(g);
    printf("%s: nasce em (%.1f, %.1f), estado %s\n", nomeTipo(tipo), g.x, g.z, nomeEstado(g.estado));
}

void iniciaGatos() {
    iniciaUmGato(gatos[0], GATO_COMUM, CEL_GATO, 1, 1);
    iniciaUmGato(gatos[1], GATO_GUIA, CEL_GATO_GUIA, 1, 2);
}

void atualizaGatos(float dt) {
    for (int i = 0; i < NUM_GATOS; i++) {
        gatos[i].tempoNoEstado += dt;
        transita(gatos[i]);
        move(gatos[i], dt);
    }
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

static void desenhaGato(const Gato &g) {
    // Cores por tipo: o comum é laranja; o guia é cinza, com coleira azul.
    GLfloat laranja[]      = { 0.90f, 0.50f, 0.15f, 1.0f };
    GLfloat laranjaClaro[] = { 0.95f, 0.80f, 0.60f, 1.0f };   // focinho e orelha
    GLfloat laranjaFeliz[] = { 1.00f, 0.65f, 0.30f, 1.0f };   // mais claro ao ser acariciado
    GLfloat cinza[]        = { 0.45f, 0.45f, 0.50f, 1.0f };
    GLfloat cinzaClaro[]   = { 0.90f, 0.90f, 0.92f, 1.0f };
    GLfloat cinzaFeliz[]   = { 0.62f, 0.62f, 0.68f, 1.0f };
    GLfloat corColeira[]   = { 0.15f, 0.35f, 0.95f, 1.0f };
    GLfloat corOlho[]      = { 0.05f, 0.05f, 0.05f, 1.0f };
    GLfloat branco[]       = { 1.0f, 1.0f, 1.0f, 1.0f };

    bool guia = (g.tipo == GATO_GUIA);
    GLfloat *corPelo  = guia ? cinza : laranja;
    GLfloat *corClaro = guia ? cinzaClaro : laranjaClaro;
    GLfloat *corFeliz = guia ? cinzaFeliz : laranjaFeliz;

    float tempo = (float) glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    bool acariciado = (g.estado == GATO_ACARICIADO);
    bool correndo = (g.estado == GATO_CORRENDO || g.estado == GATO_INDO);
    bool andando = (g.estado == GATO_VAGANDO) || correndo;

    // Patas balançam ao andar (mais abertas na corrida); o rabo balança parado, mais rápido no
    // carinho, e na corrida fica esticado para trás.
    float passada = andando ? (correndo ? 45.0f : 30.0f) * sinf(g.faseAnimacao) : 0.0f;
    float rabo = acariciado ? 40.0f * sinf(tempo * 10.0f) : 15.0f * sinf(tempo * 3.0f);
    float raboErguido = correndo ? -5.0f : -30.0f;
    if (correndo) rabo = 5.0f * sinf(g.faseAnimacao);
    float cabeca = acariciado ? 12.0f * sinf(tempo * 6.0f) : 0.0f;   // balança a cabeça

    glDisable(GL_TEXTURE_2D);   // as primitivas GLUT não têm coordenadas de textura
    glEnable(GL_NORMALIZE);     // o glScalef dos elipsoides esticaria as normais e escureceria a luz

    glPushMatrix();
    glTranslatef(g.x, 0.0f, g.z);
    glRotatef(-g.yaw * GRAUS_POR_RADIANO, 0.0f, 1.0f, 0.0f);   // ver comentário no perseguidor

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, acariciado ? corFeliz : corPelo);

    // Corpo: elipsoide deitado ao longo de z (frente = -z); na corrida, mais esticado e baixo
    glPushMatrix();
    glTranslatef(0.0f, correndo ? 0.28f : 0.32f, 0.0f);
    if (correndo) elipsoide(0.14f, 0.13f, 0.32f);
    else          elipsoide(0.16f, 0.15f, 0.28f);
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
    glRotatef(raboErguido, 1.0f, 0.0f, 0.0f);
    glutSolidCone(0.04, 0.30, 8, 2);
    glPopMatrix();

    // Coleira do guia: um toro em volta do pescoço (o toro do GLUT fica em volta do eixo z,
    // que aqui é o comprimento do corpo).
    if (guia) {
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corColeira);
        glPushMatrix();
        glTranslatef(0.0f, 0.44f, -0.22f);
        glutSolidTorus(0.025, 0.10, 8, 16);
        glPopMatrix();
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, acariciado ? corFeliz : corPelo);
    }

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

void desenhaGatos() {
    for (int i = 0; i < NUM_GATOS; i++) desenhaGato(gatos[i]);
}
