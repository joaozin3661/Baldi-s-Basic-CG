// Texturas procedurais (geradas por código, sem arquivos de imagem).
#ifndef TEXTURAS_H
#define TEXTURAS_H

#include "gl_includes.h"

// TEX_ARMARIO: armários de escola (símbolo 'A' do mapa).
// TEX_PORTA_SALA: parede com uma porta de sala que não abre (portas de cenário, letras minúsculas do mapa).
// TEX_QUADRO e TEX_MADEIRA: quadro-negro e madeira dos móveis da sala de aula (sala.cpp).
enum { TEX_PAREDE, TEX_PISO, TEX_TETO, TEX_PORTA, TEX_ARMARIO, TEX_PORTA_SALA,
       TEX_QUADRO, TEX_MADEIRA, TEX_FREQUENCIA, QUANT_TEX };
// TEX_FREQUENCIA: folha da lista de frequência (frequencia.cpp).

extern GLuint idTexturas[QUANT_TEX];

// Gera as imagens na memória e envia para o OpenGL. Chamar uma vez, depois de criar a janela.
void iniciaTexturas();

#endif
