// Estado compartilhado do jogo: o que a jogabilidade (cadernos, perseguidor, chefe)
// e a interface (HUD, telas, áudio) precisam ler e escrever.
// Quem mexer neste arquivo avisa o outro e dá push logo.
#ifndef JOGO_H
#define JOGO_H

#define TEMPO_INICIAL 180.0f   // segundos para chegar à sala da prova (provisório; ajustar na Fase 4)

// Fases do jogo. A interface decide o que desenhar por aqui; a jogabilidade só anda em JOGO_JOGANDO.
enum EstadoJogo {
    JOGO_MENU,      // tela inicial
    JOGO_JOGANDO,   // andando pelo labirinto
    JOGO_PROVA,     // boss fight com o Professor
    JOGO_VITORIA,
    JOGO_DERROTA
};

struct Jogo {
    EstadoJogo estado;
    float tempoRestante;        // segundos; chega a zero = derrota

    int cadernosColetados;
    int cadernosTotal;          // definido por quem posiciona os cadernos no mapa

    bool perseguidorAtivo;      // falso enquanto o jogador está na entrada (spawn seguro)
    bool perseguidorPerseguindo;   // verdadeiro quando ele viu o jogador (alerta do HUD)
    float perseguidorX, perseguidorZ;   // posição no chão, nas mesmas unidades do jogador
};

extern Jogo jogo;

// Zera tudo e coloca o jogo no menu. Chamar uma vez, no início.
void iniciaJogo();

// Começa (ou recomeça) uma partida: zera contadores e tempo e vai para JOGO_JOGANDO.
void comecaPartida();

// Troca o estado. Centralizado para a interface e a jogabilidade reagirem no mesmo lugar.
void mudaEstado(EstadoJogo novo);

// Avança o relógio da partida; dt em segundos. Só conta em JOGO_JOGANDO.
void atualizaJogo(float dt);

// Verdadeiro quando todos os cadernos foram coletados (abre a porta da sala da prova).
bool todosCadernosColetados();

// Distância no chão entre o jogador e o perseguidor (para o alerta de proximidade e a captura).
float distanciaAoPerseguidor();

#endif
