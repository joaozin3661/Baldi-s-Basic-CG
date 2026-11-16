#include "jogador.h"
#include "mapa.h"
#include "gl_includes.h"
#include <cmath>

Jogador jogador;

void iniciaJogador() {
    posicaoInicial(&jogador.x, &jogador.z);
    jogador.yaw = 1.5708f;   // 90 graus: olhando para +x, ao longo do primeiro corredor
    jogador.pitch = 0.0f;
}

void moveJogador(float frente, float lado, float dt) {
    if (frente == 0.0f && lado == 0.0f) return;

    // Na diagonal o vetor (frente, lado) tem tamanho raiz de 2; normaliza para não andar mais rápido.
    float tamanho = sqrtf(frente * frente + lado * lado);
    frente /= tamanho;
    lado /= tamanho;

    // Vetores no plano do chão: para onde o jogador olha e a sua direita.
    float frenteX = sinf(jogador.yaw), frenteZ = -cosf(jogador.yaw);
    float direitaX = cosf(jogador.yaw), direitaZ = sinf(jogador.yaw);

    float passo = JOGADOR_VELOCIDADE * dt;
    float dx = (frenteX * frente + direitaX * lado) * passo;
    float dz = (frenteZ * frente + direitaZ * lado) * passo;

    // Testa cada eixo separadamente: se um está bloqueado, o outro ainda anda,
    // e o jogador desliza ao longo da parede em vez de travar.
    if (!colideComMapa(jogador.x + dx, jogador.z, JOGADOR_RAIO)) jogador.x += dx;
    if (!colideComMapa(jogador.x, jogador.z + dz, JOGADOR_RAIO)) jogador.z += dz;
}

void giraJogador(float deltaYaw, float deltaPitch) {
    const float LIMITE = 1.2f;   // cerca de 69 graus: impede a câmera de virar de ponta-cabeça
    jogador.yaw += deltaYaw;
    jogador.pitch += deltaPitch;
    if (jogador.pitch > LIMITE)  jogador.pitch = LIMITE;
    if (jogador.pitch < -LIMITE) jogador.pitch = -LIMITE;
}

void aplicaCamera() {
    // Direção do olhar em 3D, a partir dos dois ângulos.
    float dirX = sinf(jogador.yaw) * cosf(jogador.pitch);
    float dirY = sinf(jogador.pitch);
    float dirZ = -cosf(jogador.yaw) * cosf(jogador.pitch);

    gluLookAt(jogador.x, JOGADOR_ALTURA, jogador.z,                        // olho
              jogador.x + dirX, JOGADOR_ALTURA + dirY, jogador.z + dirZ,   // ponto observado
              0.0, 1.0, 0.0);                                              // "para cima"
}
