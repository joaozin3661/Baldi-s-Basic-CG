// Estado compartilhado do jogo: o que a jogabilidade (cadernos, perseguidor, chefe)
// e a interface (HUD, telas, áudio) precisam ler e escrever.
// Quem mexer neste arquivo avisa o outro e dá push logo.
#ifndef JOGO_H
#define JOGO_H

#define TEMPO_INICIAL 180.0f   // segundos para chegar à sala da prova (provisório; ajustar na Fase 4)

// Felicidade do aluno, de 0 a FELICIDADE_MAX (barra no HUD, telas.cpp).
// Cai a cada folha de frequência coletada (estudar cansa) e sobe a cada carinho num gato.
#define FELICIDADE_MAX            100.0f
#define FELICIDADE_INICIAL        100.0f
#define FELICIDADE_POR_FREQUENCIA  30.0f   // perde ao coletar uma folha (4 folhas = 120: sem carinho, zera)
#define FELICIDADE_POR_CARINHO     15.0f   // ganha ao acariciar um gato

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

    // "Cadernos" = folhas de frequência da faculdade (frequencia.cpp sorteia FREQ_TOTAL por partida).
    int cadernosColetados;
    int cadernosTotal;          // definido por quem posiciona os cadernos no mapa

    float felicidade;           // de 0 a FELICIDADE_MAX; mude sempre por mudaFelicidade()

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

// Soma delta à felicidade (negativo diminui), presa entre 0 e FELICIDADE_MAX.
void mudaFelicidade(float delta);

// Verdadeiro quando todos os cadernos foram coletados (abre a porta da sala da prova).
bool todosCadernosColetados();

// Distância no chão entre o jogador e o perseguidor (para o alerta de proximidade e a captura).
float distanciaAoPerseguidor();

#endif
