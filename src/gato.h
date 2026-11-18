// Os gatos da faculdade: NPCs pacíficos que passeiam pelos corredores e aceitam carinho.
// Cada carinho deixa o aluno mais feliz (jogo.felicidade). Há dois gatos:
//   - o gato comum (laranja), que só aproveita o carinho;
//   - o gato guia (cinza, de coleira azul), que depois do carinho dispara correndo até a folha
//     de frequência mais próxima, mostrando o caminho ao aluno.
// Mesma estrutura do perseguidor: máquina de estados finitos (aula 21), colisão por
// esferas envolventes e modelagem hierárquica com primitivas GLUT.
#ifndef GATO_H
#define GATO_H

#include "mapa.h"   // ESCALA_MAPA

#define NUM_GATOS 2

#define GATO_RAIO            0.30f   // raio da esfera envolvente
#define GATO_VELOCIDADE      (1.20f * ESCALA_MAPA)   // unidades do mundo por segundo (anda devagar)
#define GATO_ALCANCE_CARINHO 1.20f   // distância máxima do jogador para acariciar
#define GATO_TEMPO_CARINHO   3.00f   // segundos que o gato comum fica parado recebendo carinho
#define GATO_CHANCE_PARAR    0.20f   // chance de sentar ao chegar numa célula (0 a 1)

// Gato guia
#define GATO_GUIA_TEMPO_CARINHO 0.60f   // ronrona um instante antes de disparar
#define GATO_GUIA_VEL_CORRIDA   (7.00f * ESCALA_MAPA)   // o dobro do aluno, do carinho até a folha

// Campo de visão do aluno, para saber se ele ainda vê o gato guia correndo.
// A câmera usa gluPerspective(65, 1024/600): meio ângulo horizontal = atan(tan(32,5°) * 1,7) ≈ 47°.
#define GATO_MEIO_CAMPO_VISAO 47.0f   // graus, para cada lado do centro da tela
#define GATO_DIST_VISIVEL     12.0f   // mais longe que isso a lanterna quase não ilumina o gato

enum TipoGato {
    GATO_COMUM,   // laranja: só aceita carinho
    GATO_GUIA     // cinza: acariciado, corre até a frequência mais próxima
};

enum EstadoGato {
    GATO_VAGANDO,      // sorteia uma direção da grade e anda até a próxima célula
    GATO_PARADO,       // senta por alguns segundos, depois volta a vagar
    GATO_ACARICIADO,   // o jogador fez carinho: fica parado, fecha os olhos e balança o rabo
    GATO_CORRENDO,     // (guia) dispara até a frequência, enquanto o aluno ainda o vê
    GATO_INDO          // (guia) saiu da vista do aluno: continua em disparada até a frequência
};

struct Gato {
    TipoGato tipo;
    float x, z;                 // posição no chão
    float yaw;                  // para onde olha, em radianos (0 = -z, como o jogador)
    EstadoGato estado;
    float velocidade;

    int lAlvo, cAlvo;           // célula para onde está indo agora (vizinha da atual)
    int lAnterior, cAnterior;   // célula de onde veio (para não voltar ao vagar)
    int frequenciaAlvo;         // (guia) índice da folha para onde corre; -1 se nenhuma
    int lDestino, cDestino;     // (guia) célula dessa folha
    float tempoNoEstado;        // segundos desde a última transição
    float tempoParado;          // quanto tempo vai ficar sentado desta vez
    float faseAnimacao;         // acumula o movimento para mexer as patas
    int carinhos;               // quantas vezes já foi acariciado (só para o console)
};

extern Gato gatos[NUM_GATOS];   // gatos[0] = comum, gatos[1] = guia

// Coloca cada gato na sua célula do mapa (CEL_GATO e CEL_GATO_GUIA), no estado VAGANDO.
void iniciaGatos();

// Atualiza máquina de estados e movimento de todos; dt em segundos. Só chamar em JOGO_JOGANDO.
void atualizaGatos(float dt);

// Desenha os gatos (esferas e cones). De olhos fechados enquanto são acariciados.
void desenhaGatos();

// Chamar quando o jogador aperta a tecla de interação (E). Acaricia o gato mais próximo que
// estiver ao alcance (esfera x esfera) e dá FELICIDADE_POR_CARINHO. Retorna verdadeiro se
// acariciou algum; longe de todos, não acontece nada.
bool interageComGato();

#endif
