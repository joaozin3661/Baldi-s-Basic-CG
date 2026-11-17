// Telas 2D desenhadas por cima da cena: início (comandos e confirmação), derrota e vitória.
// Texto com glutBitmapCharacter, como o HUD visto em aula.
#ifndef TELAS_H
#define TELAS_H

// Desenha a tela do estado atual do jogo, se houver uma (em JOGO_JOGANDO não desenha nada).
// Recebe o tamanho da janela em pixels para montar a projeção 2D.
void desenhaTelas(int larguraJanela, int alturaJanela);

// Trata uma tecla enquanto uma tela está aberta. Devolve verdadeiro se a tecla foi consumida
// (e, portanto, não deve chegar aos controles do jogo).
//   - Tela de início: ENTER, ESPAÇO ou S confirmam que o jogador entendeu e começam a partida.
//   - Derrota e vitória: ENTER recomeça.
bool teclaTelas(unsigned char tecla);

#endif
