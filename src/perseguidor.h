// O perseguidor: NPC que patrulha o labirinto e corre atrás do aluno quando o vê.
// IA reativa com máquina de estados finitos (aula 21), colisão por esferas envolventes
// e modelagem hierárquica com primitivas GLUT.
#ifndef PERSEGUIDOR_H
#define PERSEGUIDOR_H

#define PERSEGUIDOR_RAIO            0.40f   // raio da esfera envolvente (colisão com o jogador)
#define PERSEGUIDOR_VEL_BASE        2.00f   // unidades do mundo por segundo, sem cadernos
#define PERSEGUIDOR_VEL_POR_CADERNO 0.35f   // acréscimo por caderno coletado
#define PERSEGUIDOR_VEL_ARRASTO     1.40f   // velocidade ao arrastar o aluno (carregar pesa)
#define PERSEGUIDOR_ALCANCE_VE      7.00f   // distância em que passa a perseguir (se houver linha de visão)
#define PERSEGUIDOR_ALCANCE_PERDE  11.00f   // distância em que desiste e volta a vagar
#define PERSEGUIDOR_RECUPERACAO     3.00f   // segundos parado depois de soltar o jogador
#define PERSEGUIDOR_TREGUA          5.00f   // segundos em que ignora o jogador depois de soltá-lo
#define PERSEGUIDOR_SACUDIDAS       6.00f   // "força" acumulada para se soltar (1 por tecla apertada)
#define PERSEGUIDOR_SACUDIDA_DECAI  2.00f   // força perdida por segundo: tem que apertar rápido
#define PERSEGUIDOR_DIST_ARRASTO    0.65f   // o aluno vai atrás do perseguidor, a esta distância

enum EstadoPerseguidor {
    PERSEGUIDOR_VAGANDO,       // sorteia uma direção da grade e anda até a próxima célula
    PERSEGUIDOR_PERSEGUINDO,   // segue o jogador pelo caminho mais curto (BFS)
    PERSEGUIDOR_ARRASTANDO,    // agarrou o jogador e o leva pelo caminho mais curto até a célula 'S'
    PERSEGUIDOR_RECUPERANDO    // soltou o jogador: fica parado uns segundos antes de voltar a vagar
};

struct Perseguidor {
    float x, z;                 // posição no chão
    float yaw;                  // para onde olha, em radianos (0 = -z, como o jogador)
    EstadoPerseguidor estado;
    float velocidade;           // recalculada a cada quadro a partir dos cadernos

    int lAlvo, cAlvo;           // célula para onde está indo
    int lAnterior, cAnterior;   // célula de onde veio (para não voltar ao vagar)
    int lInicio, cInicio;       // célula 'S' do mapa, destino do arrasto
    float tempoNoEstado;        // segundos desde a última transição
    float tregua;               // segundos que faltam ignorando o jogador (depois de soltá-lo)
    float faseAnimacao;         // acumula o movimento para balançar braços e pernas

    float sacudidas;            // força acumulada pelo jogador para se soltar (decai com o tempo)
    bool chegouAoInicio;        // arrastando: já pisou no centro da célula 'S'
};

extern Perseguidor perseguidor;

// Coloca o perseguidor na célula CEL_PERSEGUIDOR do mapa, no estado VAGANDO.
void iniciaPerseguidor();

// Atualiza percepção, máquina de estados e movimento; dt em segundos.
// Só chamar em JOGO_JOGANDO. Mantém jogo.perseguidorX/Z e jogo.perseguidorPerseguindo.
// No estado ARRASTANDO também move o jogador, que vai preso atrás dele.
void atualizaPerseguidor(float dt);

// Verdadeiro enquanto o jogador está nos braços do perseguidor (estado ARRASTANDO).
// O main.cpp não deve chamar moveJogador() enquanto isso for verdadeiro.
bool perseguidorAgarrouJogador();

// O jogador se debate: chamada a cada tecla de andar apertada (não segurada) enquanto
// está agarrado. Juntando força suficiente em pouco tempo, ele se solta.
void sacodeJogador();

// Desenha o boneco (cubos e esferas). A cabeça fica vermelha ao perseguir.
void desenhaPerseguidor();

#endif
