// Texturas procedurais (geradas por código, sem arquivos de imagem).
#ifndef TEXTURAS_H
#define TEXTURAS_H

#include "gl_includes.h"

enum { TEX_PAREDE, TEX_PISO, TEX_TETO, TEX_PORTA, QUANT_TEX };

extern GLuint idTexturas[QUANT_TEX];

// Gera as imagens na memória e envia para o OpenGL. Chamar uma vez, depois de criar a janela.
void iniciaTexturas();

#endif
