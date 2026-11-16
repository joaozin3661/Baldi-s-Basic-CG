#include "perseguidor.h"
#include "caminho.h"
#include "mapa.h"
#include "jogador.h"
#include "jogo.h"
#include "gl_includes.h"
#include <cmath>
#include <cstdio>

Perseguidor perseguidor;

#define GRAUS_POR_RADIANO 57.29578f

static const char *nomeEstado(EstadoPerseguidor e) {
    switch (e) {
        case PERSEGUIDOR_VAGANDO:     return "vagando";
        case PERSEGUIDOR_PERSEGUINDO: return "perseguindo";
        case PERSEGUIDOR_ARRASTANDO:  return "arrastando";
        case PERSEGUIDOR_RECUPERANDO: return "recuperando";
    }
    return "?";
}

// ---------------------------------------------------------------- percepção

// Distância no chão até o jogador.
static float distanciaAoJogador() {
    float dx = jogador.x - perseguidor.x, dz = jogador.z - perseguidor.z;
    return sqrtf(dx * dx + dz * dz);
}

// Colisão esfera x esfera (aula 21): há contato quando a distância entre os centros
// é menor que a soma dos raios.
static bool encostaNoJogador() {
    return distanciaAoJogador() < PERSEGUIDOR_RAIO + JOGADOR_RAIO;
}

// Percebe o jogador se ele está dentro do alcance e nenhuma parede bloqueia a visão.
// Durante a trégua (logo depois de soltá-lo) finge que não vê, senão o agarraria de novo na hora.
static bool veJogador() {
    return perseguidor.tregua <= 0.0f
        && distanciaAoJogador() < PERSEGUIDOR_ALCANCE_VE
        && haLinhaDeVisao(perseguidor.x, perseguidor.z, jogador.x, jogador.z);
}

// ---------------------------------------------------------------- máquina de estados

static void trocaEstado(EstadoPerseguidor novo) {
    if (perseguidor.estado == novo) return;
    printf("Perseguidor: %s -> %s\n", nomeEstado(perseguidor.estado), nomeEstado(novo));
    perseguidor.estado = novo;
    perseguidor.tempoNoEstado = 0.0f;
}

// Mecânica de contato (DECISOES.md, 16/11/2026): o perseguidor não derrota o aluno,
// ele o agarra e o arrasta de volta para a entrada da faculdade. O progresso
// (cadernos, tempo) fica. Para se soltar, o aluno se debate (teclas de andar).
static void agarraJogador() {
    printf("Perseguidor agarrou o aluno! Aperte as teclas de andar para se soltar.\n");
    perseguidor.sacudidas = 0.0f;
    perseguidor.chegouAoInicio = false;
    // O aluno vira para o perseguidor: vê quem o pegou.
    jogador.yaw = atan2f(perseguidor.x - jogador.x, -(perseguidor.z - jogador.z));
}

static void soltaJogador(const char *motivo) {
    printf("%s\n", motivo);
    perseguidor.tregua = PERSEGUIDOR_TREGUA;
}

// Todas as transições ficam aqui, para serem fáceis de mostrar na apresentação.
static void transita() {
    switch (perseguidor.estado) {
        case PERSEGUIDOR_VAGANDO:
            if (veJogador()) trocaEstado(PERSEGUIDOR_PERSEGUINDO);
            break;

        case PERSEGUIDOR_PERSEGUINDO:
            if (encostaNoJogador()) {
                agarraJogador();
                trocaEstado(PERSEGUIDOR_ARRASTANDO);
            } else if (distanciaAoJogador() > PERSEGUIDOR_ALCANCE_PERDE) {
                trocaEstado(PERSEGUIDOR_VAGANDO);   // o aluno fugiu para longe
            }
            break;

        case PERSEGUIDOR_ARRASTANDO:
            if (perseguidor.sacudidas >= PERSEGUIDOR_SACUDIDAS) {
                soltaJogador("O aluno se debateu e se soltou!");
                trocaEstado(PERSEGUIDOR_RECUPERANDO);
            } else if (perseguidor.chegouAoInicio) {
                iniciaJogador();   // deixa o aluno na célula S; não mexe em jogo.*
                soltaJogador("Perseguidor levou o aluno de volta ao inicio.");
                trocaEstado(PERSEGUIDOR_RECUPERANDO);
            }
            break;

        case PERSEGUIDOR_RECUPERANDO:
            if (perseguidor.tempoNoEstado >= PERSEGUIDOR_RECUPERACAO)
                trocaEstado(PERSEGUIDOR_VAGANDO);
            break;
    }
}

// ---------------------------------------------------------------- movimento

// Escolhe a próxima célula a partir da célula atual, conforme o estado.
// O caminho (até o jogador ou até a entrada) é recalculado a cada célula alcançada
// (meio segundo, mais ou menos), não a cada quadro.
static void escolheProximaCelula() {
    int l, c;
    celulaDoMundo(perseguidor.x, perseguidor.z, &l, &c);

    int lProx = l, cProx = c;
    bool achou = false;

    if (perseguidor.estado == PERSEGUIDOR_PERSEGUINDO) {
        int lJog, cJog;
        celulaDoMundo(jogador.x, jogador.z, &lJog, &cJog);
        achou = proximoPasso(l, c, lJog, cJog, &lProx, &cProx);
    } else if (perseguidor.estado == PERSEGUIDOR_ARRASTANDO) {
        achou = proximoPasso(l, c, perseguidor.lInicio, perseguidor.cInicio, &lProx, &cProx);
        if (!achou) {   // já está na célula S (ou ela é inalcançável): fica nela
            perseguidor.lAnterior = l;
            perseguidor.cAnterior = c;
            perseguidor.lAlvo = l;
            perseguidor.cAlvo = c;
            return;
        }
    }
    if (!achou)   // vagando, ou sem caminho: sorteia uma direção (estado 1 do Pacman)
        achou = sorteiaVizinho(l, c, perseguidor.lAnterior, perseguidor.cAnterior, &lProx, &cProx);

    if (achou) {
        perseguidor.lAnterior = l;
        perseguidor.cAnterior = c;
        perseguidor.lAlvo = lProx;
        perseguidor.cAlvo = cProx;
    }
}

// Anda em direção a um ponto; testa cada eixo separado para deslizar na parede.
// Retorna verdadeiro se chegou.
static bool andaPara(float xAlvo, float zAlvo, float dt) {
    float dx = xAlvo - perseguidor.x, dz = zAlvo - perseguidor.z;
    float distancia = sqrtf(dx * dx + dz * dz);
    float passo = perseguidor.velocidade * dt;

    if (distancia <= passo) {   // chega neste quadro
        perseguidor.x = xAlvo;
        perseguidor.z = zAlvo;
        return true;
    }

    dx *= passo / distancia;
    dz *= passo / distancia;
    perseguidor.yaw = atan2f(dx, -dz);   // mesma convenção do jogador: 0 = olhando para -z
    perseguidor.faseAnimacao += passo * 4.0f;

    if (!colideComMapa(perseguidor.x + dx, perseguidor.z, PERSEGUIDOR_RAIO)) perseguidor.x += dx;
    if (!colideComMapa(perseguidor.x, perseguidor.z + dz, PERSEGUIDOR_RAIO)) perseguidor.z += dz;
    return false;
}

// Arrastando: o aluno vai preso logo atrás do perseguidor (ele o puxa pelo braço).
// Se atrás dele for parede, o aluno fica em cima dele mesmo, para nunca entrar na parede.
static void carregaJogador() {
    float atrasX = perseguidor.x - sinf(perseguidor.yaw) * PERSEGUIDOR_DIST_ARRASTO;
    float atrasZ = perseguidor.z + cosf(perseguidor.yaw) * PERSEGUIDOR_DIST_ARRASTO;
    if (!colideComMapa(atrasX, atrasZ, JOGADOR_RAIO)) {
        jogador.x = atrasX;
        jogador.z = atrasZ;
    } else {
        jogador.x = perseguidor.x;
        jogador.z = perseguidor.z;
    }
}

static void move(float dt) {
    if (perseguidor.estado == PERSEGUIDOR_RECUPERANDO) return;   // fica parado

    int l, c;
    celulaDoMundo(perseguidor.x, perseguidor.z, &l, &c);

    // Perseguindo e já na célula do jogador: vai reto nele (não há parede no meio).
    if (perseguidor.estado == PERSEGUIDOR_PERSEGUINDO) {
        int lJog, cJog;
        celulaDoMundo(jogador.x, jogador.z, &lJog, &cJog);
        if (l == lJog && c == cJog) {
            andaPara(jogador.x, jogador.z, dt);
            perseguidor.lAlvo = l;
            perseguidor.cAlvo = c;
            return;
        }
    }

    float xAlvo, zAlvo;
    centroDaCelula(perseguidor.lAlvo, perseguidor.cAlvo, &xAlvo, &zAlvo);
    bool chegou = andaPara(xAlvo, zAlvo, dt);

    if (perseguidor.estado == PERSEGUIDOR_ARRASTANDO) {
        carregaJogador();
        if (chegou && perseguidor.lAlvo == perseguidor.lInicio && perseguidor.cAlvo == perseguidor.cInicio)
            perseguidor.chegouAoInicio = true;   // transita() solta o aluno no próximo quadro
    }
    if (chegou) escolheProximaCelula();
}

// ---------------------------------------------------------------- interface

void iniciaPerseguidor() {
    perseguidor.x = perseguidor.z = -1.0f;
    for (int l = 0; l < MAPA_LINHAS && perseguidor.x < 0.0f; l++)
        for (int c = 0; c < MAPA_COLUNAS; c++)
            if (celula(l, c) == CEL_PERSEGUIDOR) {
                centroDaCelula(l, c, &perseguidor.x, &perseguidor.z);
                break;
            }
    if (perseguidor.x < 0.0f) {   // mapa sem símbolo: nasce no canto oposto ao do jogador
        centroDaCelula(MAPA_LINHAS - 2, MAPA_COLUNAS - 2, &perseguidor.x, &perseguidor.z);
        printf("Aviso: mapa sem '%c'; perseguidor nasce em (%d, %d)\n",
               CEL_PERSEGUIDOR, MAPA_LINHAS - 2, MAPA_COLUNAS - 2);
    }

    // Célula S: para onde ele arrasta o aluno.
    float xInicio, zInicio;
    posicaoInicial(&xInicio, &zInicio);
    celulaDoMundo(xInicio, zInicio, &perseguidor.lInicio, &perseguidor.cInicio);

    perseguidor.yaw = 0.0f;
    perseguidor.estado = PERSEGUIDOR_VAGANDO;
    perseguidor.velocidade = PERSEGUIDOR_VEL_BASE;
    perseguidor.tempoNoEstado = 0.0f;
    perseguidor.tregua = 0.0f;
    perseguidor.faseAnimacao = 0.0f;
    perseguidor.sacudidas = 0.0f;
    perseguidor.chegouAoInicio = false;
    perseguidor.lAnterior = perseguidor.cAnterior = -1;
    celulaDoMundo(perseguidor.x, perseguidor.z, &perseguidor.lAlvo, &perseguidor.cAlvo);
    escolheProximaCelula();

    jogo.perseguidorAtivo = true;
    jogo.perseguidorPerseguindo = false;
    jogo.perseguidorX = perseguidor.x;
    jogo.perseguidorZ = perseguidor.z;
    printf("Perseguidor: nasce em (%.1f, %.1f), estado %s\n",
           perseguidor.x, perseguidor.z, nomeEstado(perseguidor.estado));
}

void atualizaPerseguidor(float dt) {
    // Acelera a cada caderno coletado (lido a cada quadro, como no jogo original).
    // Arrastando o aluno, anda devagar, para dar tempo de ele se debater.
    if (perseguidor.estado == PERSEGUIDOR_ARRASTANDO)
        perseguidor.velocidade = PERSEGUIDOR_VEL_ARRASTO;
    else
        perseguidor.velocidade = PERSEGUIDOR_VEL_BASE + PERSEGUIDOR_VEL_POR_CADERNO * jogo.cadernosColetados;
    perseguidor.tempoNoEstado += dt;
    if (perseguidor.tregua > 0.0f) perseguidor.tregua -= dt;

    // A força da sacudida escorre com o tempo: apertar devagar não solta.
    if (perseguidor.sacudidas > 0.0f) {
        perseguidor.sacudidas -= PERSEGUIDOR_SACUDIDA_DECAI * dt;
        if (perseguidor.sacudidas < 0.0f) perseguidor.sacudidas = 0.0f;
    }

    transita();
    move(dt);

    // Estado compartilhado, para o HUD (alerta de proximidade) e o áudio.
    jogo.perseguidorPerseguindo = (perseguidor.estado == PERSEGUIDOR_PERSEGUINDO
                                || perseguidor.estado == PERSEGUIDOR_ARRASTANDO);
    jogo.perseguidorX = perseguidor.x;
    jogo.perseguidorZ = perseguidor.z;
}

bool perseguidorAgarrouJogador() {
    return perseguidor.estado == PERSEGUIDOR_ARRASTANDO;
}

void sacodeJogador() {
    if (perseguidor.estado != PERSEGUIDOR_ARRASTANDO) return;
    perseguidor.sacudidas += 1.0f;
    printf("Aluno se debate: %.0f/%.0f\n", perseguidor.sacudidas, PERSEGUIDOR_SACUDIDAS);
}

// ---------------------------------------------------------------- desenho

// Caixa com o centro em (0, 0, 0) e lados (lx, ly, lz).
static void caixa(float lx, float ly, float lz) {
    glPushMatrix();
    glScalef(lx, ly, lz);
    glutSolidCube(1.0);
    glPopMatrix();
}

// Um membro preso no ombro ou no quadril: gira em torno do ponto de apoio e pende para baixo.
static void membro(float x, float yApoio, float comprimento, float grossura, float anguloGraus) {
    glPushMatrix();
    glTranslatef(x, yApoio, 0.0f);
    glRotatef(anguloGraus, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -comprimento * 0.5f, 0.0f);
    caixa(grossura, comprimento, grossura);
    glPopMatrix();
}

void desenhaPerseguidor() {
    GLfloat corRoupa[]   = { 0.20f, 0.25f, 0.55f, 1.0f };   // azul escuro
    GLfloat corPele[]    = { 0.90f, 0.75f, 0.60f, 1.0f };
    GLfloat corBrava[]   = { 0.90f, 0.15f, 0.10f, 1.0f };   // cabeça vermelha ao perseguir
    GLfloat corAgarra[]  = { 0.70f, 0.10f, 0.60f, 1.0f };   // roxa ao arrastar o aluno
    GLfloat corCansada[] = { 0.55f, 0.55f, 0.60f, 1.0f };   // cinza ao recuperar
    GLfloat branco[]     = { 1.0f, 1.0f, 1.0f, 1.0f };

    // Braços e pernas balançam em oposição, como num passo; parado, ficam retos.
    float balanco = 0.0f;
    if (perseguidor.estado != PERSEGUIDOR_RECUPERANDO) balanco = 35.0f * sinf(perseguidor.faseAnimacao);

    // Arrastando: os braços ficam esticados para trás, segurando o aluno.
    float bracoEsq = -balanco, bracoDir = balanco;
    if (perseguidor.estado == PERSEGUIDOR_ARRASTANDO) bracoEsq = bracoDir = -60.0f;

    glDisable(GL_TEXTURE_2D);   // as primitivas GLUT não têm coordenadas de textura
    glEnable(GL_NORMALIZE);     // o glScalef das caixas esticaria as normais e escureceria a luz

    glPushMatrix();
    glTranslatef(perseguidor.x, 0.0f, perseguidor.z);
    // glRotatef gira no sentido anti-horário visto de cima; o yaw do jogo cresce no sentido
    // horário (de -z para +x), por isso o sinal negativo.
    glRotatef(-perseguidor.yaw * GRAUS_POR_RADIANO, 0.0f, 1.0f, 0.0f);

    // Tronco
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corRoupa);
    glPushMatrix();
    glTranslatef(0.0f, 1.05f, 0.0f);
    caixa(0.50f, 0.70f, 0.28f);
    glPopMatrix();

    // Pernas (presas no quadril, y = 0.70)
    membro(-0.13f, 0.70f, 0.70f, 0.18f,  balanco);
    membro( 0.13f, 0.70f, 0.70f, 0.18f, -balanco);

    // Braços (presos no ombro, y = 1.35)
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corPele);
    membro(-0.33f, 1.35f, 0.65f, 0.14f, bracoEsq);
    membro( 0.33f, 1.35f, 0.65f, 0.14f, bracoDir);

    // Cabeça: a cor indica o estado (ajuda a ver a máquina de estados funcionando)
    if (perseguidor.estado == PERSEGUIDOR_PERSEGUINDO)      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corBrava);
    else if (perseguidor.estado == PERSEGUIDOR_ARRASTANDO)  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corAgarra);
    else if (perseguidor.estado == PERSEGUIDOR_RECUPERANDO) glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, corCansada);
    glPushMatrix();
    glTranslatef(0.0f, 1.65f, 0.0f);
    glutSolidSphere(0.24, 16, 16);
    glPopMatrix();

    // Nariz: um cone apontando para a frente (-z), para se ver para onde ele olha
    glPushMatrix();
    glTranslatef(0.0f, 1.62f, -0.22f);
    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);   // o cone do GLUT aponta para +z
    glutSolidCone(0.06, 0.12, 8, 2);
    glPopMatrix();

    glPopMatrix();

    // Restaura o que o cenário usa: material branco (a textura dá a cor) e textura ligada.
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, branco);
    glDisable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);
}
