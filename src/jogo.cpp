#include "jogo.h"
#include "jogador.h"
#include "frequencia.h"
#include <cmath>
#include <cstdio>

Jogo jogo;

static const char *nomeEstado(EstadoJogo e) {
    switch (e) {
        case JOGO_MENU:    return "menu";
        case JOGO_JOGANDO: return "jogando";
        case JOGO_PROVA:   return "prova";
        case JOGO_VITORIA: return "vitoria";
        case JOGO_DERROTA: return "derrota";
    }
    return "?";
}

void iniciaJogo() {
    jogo.estado = JOGO_MENU;
    jogo.tempoRestante = TEMPO_INICIAL;
    jogo.cadernosColetados = 0;
    jogo.cadernosTotal = 0;
    jogo.felicidade = FELICIDADE_INICIAL;
    jogo.perseguidorAtivo = false;
    jogo.perseguidorPerseguindo = false;
    jogo.perseguidorX = jogo.perseguidorZ = 0.0f;
}

void comecaPartida() {
    jogo.tempoRestante = TEMPO_INICIAL;
    jogo.felicidade = FELICIDADE_INICIAL;
    jogo.perseguidorAtivo = false;
    jogo.perseguidorPerseguindo = false;
    sorteiaFrequencias();   // novas folhas de frequência a cada partida (zera cadernosColetados)
    mudaEstado(JOGO_JOGANDO);
}

void mudaEstado(EstadoJogo novo) {
    if (jogo.estado == novo) return;
    printf("Estado: %s -> %s\n", nomeEstado(jogo.estado), nomeEstado(novo));   // útil para depurar
    jogo.estado = novo;
}

void atualizaJogo(float dt) {
    if (jogo.estado != JOGO_JOGANDO) return;

    jogo.tempoRestante -= dt;
    if (jogo.tempoRestante <= 0.0f) {
        jogo.tempoRestante = 0.0f;
        mudaEstado(JOGO_DERROTA);   // o tempo acabou antes de chegar à prova
    }
}

void mudaFelicidade(float delta) {
    float antes = jogo.felicidade;
    jogo.felicidade += delta;
    if (jogo.felicidade < 0.0f) jogo.felicidade = 0.0f;
    if (jogo.felicidade > FELICIDADE_MAX) jogo.felicidade = FELICIDADE_MAX;
    printf("Felicidade: %.0f -> %.0f\n", antes, jogo.felicidade);
}

bool todosCadernosColetados() {
    return jogo.cadernosTotal > 0 && jogo.cadernosColetados >= jogo.cadernosTotal;
}

float distanciaAoPerseguidor() {
    float dx = jogador.x - jogo.perseguidorX;
    float dz = jogador.z - jogo.perseguidorZ;
    return sqrtf(dx * dx + dz * dz);
}
