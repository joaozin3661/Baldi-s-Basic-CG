// O gato da faculdade: NPC pacífico que passeia pelos corredores e aceita carinho.
// Mesma estrutura do perseguidor: máquina de estados finitos (aula 21), colisão por
// esferas envolventes e modelagem hierárquica com primitivas GLUT.
#ifndef GATO_H
#define GATO_H

#define GATO_RAIO            0.30f   // raio da esfera envolvente
#define GATO_VELOCIDADE      1.20f   // unidades do mundo por segundo (anda devagar)
#define GATO_ALCANCE_CARINHO 1.20f   // distância máxima do jogador para acariciar
#define GATO_TEMPO_CARINHO   3.00f   // segundos que fica parado recebendo carinho
#define GATO_CHANCE_PARAR    0.20f   // chance de sentar ao chegar numa célula (0 a 1)

enum EstadoGato {
    GATO_VAGANDO,      // sorteia uma direção da grade e anda até a próxima célula
    GATO_PARADO,       // senta por alguns segundos, depois volta a vagar
    GATO_ACARICIADO    // o jogador fez carinho: fica parado, fecha os olhos e balança o rabo
};

struct Gato {
    float x, z;                 // posição no chão
    float yaw;                  // para onde olha, em radianos (0 = -z, como o jogador)
    EstadoGato estado;
    float velocidade;

    int lAlvo, cAlvo;           // célula para onde está indo
    int lAnterior, cAnterior;   // célula de onde veio (para não voltar ao vagar)
    float tempoNoEstado;        // segundos desde a última transição
    float tempoParado;          // quanto tempo vai ficar sentado desta vez
    float faseAnimacao;         // acumula o movimento para mexer as patas
    int carinhos;               // quantas vezes já foi acariciado (só para o console)
};

extern Gato gato;

// Coloca o gato na célula CEL_GATO do mapa, no estado VAGANDO.
void iniciaGato();

// Atualiza máquina de estados e movimento; dt em segundos. Só chamar em JOGO_JOGANDO.
void atualizaGato(float dt);

// Desenha o gato (esferas e cones, laranja). De olhos fechados enquanto é acariciado.
void desenhaGato();

// Chamar quando o jogador aperta a tecla de interação (E). Se o gato está ao alcance
// (esfera x esfera), entra em GATO_ACARICIADO; longe, não acontece nada.
void interageComGato();

#endif
