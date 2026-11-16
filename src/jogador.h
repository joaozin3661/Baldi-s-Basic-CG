// O jogador: posição, direção do olhar (câmera FPS) e movimento com colisão.
#ifndef JOGADOR_H
#define JOGADOR_H

#define JOGADOR_RAIO       0.30f   // raio do círculo de colisão
#define JOGADOR_ALTURA     1.50f   // altura dos olhos
#define JOGADOR_VELOCIDADE 3.50f   // unidades do mundo por segundo
#define JOGADOR_GIRO       2.20f   // radianos por segundo (setas esquerda/direita)

struct Jogador {
    float x, z;     // posição no chão
    float yaw;      // giro horizontal, em radianos (0 = olhando para -z)
    float pitch;    // inclinação vertical, em radianos (positivo = para cima)
};

extern Jogador jogador;

// Coloca o jogador na célula 'S' do mapa.
void iniciaJogador();

// Anda. "frente" e "lado" vão de -1 a 1; dt é o tempo do quadro em segundos.
void moveJogador(float frente, float lado, float dt);

// Gira a câmera (valores em radianos).
void giraJogador(float deltaYaw, float deltaPitch);

// Posiciona a câmera nos olhos do jogador (chama gluLookAt).
void aplicaCamera();

#endif
