// O labirinto: uma matriz de caracteres, como no exemplo do Pacman 3D da aula 21.
#ifndef MAPA_H
#define MAPA_H

#define MAPA_LINHAS   19
#define MAPA_COLUNAS  20

#define TAM_CELULA    2.0f   // lado de cada célula, em unidades do mundo
#define ALTURA_PAREDE 2.5f

// Símbolos do mapa
#define CEL_PAREDE '#'
#define CEL_VAZIO  '.'
#define CEL_INICIO 'S'   // onde o jogador nasce
#define CEL_PORTA  'P'   // porta da sala de aula da prova (E perto dela entra; ver sala.cpp)
#define CEL_SALA   'F'   // chão da sala da prova
// Onde os NPCs nascem. São chão: celulaSolida() não os trata como parede.
// ('P' já é a porta, por isso o perseguidor usa 'X'.)
#define CEL_GATO        'G'
#define CEL_GATO_GUIA   'C'   // o gato cinza que, acariciado, corre até a frequência mais próxima
#define CEL_PERSEGUIDOR 'X'
// Enfeites do cenário. São paredes (sólidas): só mudam o desenho.
#define CEL_ARMARIO     'A'   // armários de escola
// Portas de sala que não abrem: cada letra minúscula ('a', 'b', 'c'...) é uma sala, e a letra
// indexa a tabela de nomes em mapa.cpp ('a' = primeira sala). A porta e a placa com o nome são
// desenhadas numa só face da célula, a que dá para o corredor.
#define CEL_SALA_DECOR_PRIMEIRA 'a'
#define CEL_SALA_DECOR_ULTIMA   'z'

// Conteúdo da célula (linha, coluna). Fora do mapa conta como parede.
char celula(int linha, int coluna);

// Verdadeiro se a célula bloqueia a passagem (parede, porta, armário ou porta de sala).
bool celulaSolida(int linha, int coluna);

// Monta a geometria do labirinto (com armários, portas de sala e placas).
// Chamar uma vez, depois de iniciaTexturas() e de glutInit() (as placas usam a fonte do GLUT).
void iniciaMapa();

// Desenha o labirinto inteiro.
void desenhaMapa();

// Centro da célula marcada com 'S', em coordenadas do mundo.
void posicaoInicial(float *x, float *z);

// Verdadeiro se um círculo de centro (x, z) e raio dado encosta em alguma célula sólida.
bool colideComMapa(float x, float z, float raio);

#endif
