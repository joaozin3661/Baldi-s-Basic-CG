// "Frequência da faculdade": as folhas que o aluno precisa coletar para a sala da prova abrir.
// A cada partida, FREQ_TOTAL folhas são sorteadas pelo labirinto, no chão ou presas na parede.
// O progresso fica em jogo.cadernosColetados / jogo.cadernosTotal (jogo.h), que a porta da
// sala (sala.cpp) e a velocidade do perseguidor já consultam.
#ifndef FREQUENCIA_H
#define FREQUENCIA_H

#define FREQ_TOTAL 4

// Monta a display list da folha. Chamar uma vez, depois de iniciaTexturas().
void iniciaFrequencias();

// Sorteia novas posições para as folhas e zera o progresso. Chamada a cada partida (comecaPartida).
void sorteiaFrequencias();

// Coleta as folhas que o jogador alcançou e conta o tempo da mensagem na tela. dt em segundos.
void atualizaFrequencias(float dt);

// Desenha as folhas que ainda não foram coletadas.
void desenhaFrequencias();

// Mensagem curta para o centro da tela logo depois de uma coleta; NULL quando não há.
const char *mensagemFrequencia();

// Quantas folhas foram sorteadas nesta partida (os índices vão de 0 a este valor - 1).
int quantidadeFrequencias();

// Se a folha i ainda não foi coletada, devolve a célula de corredor onde ela está e retorna
// verdadeiro. Usada pelo gato guia (gato.cpp) para correr até a folha mais próxima.
bool celulaFrequenciaPendente(int i, int *linha, int *coluna);

#endif
