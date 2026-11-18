// Sala de aula atrás da porta 'P' do mapa (a sala da prova, células 'F'):
// quadro-negro, mesa do professor, carteiras e a passagem pela porta com a tecla E.
#ifndef SALA_H
#define SALA_H

// Acha a porta e a sala no mapa e monta a display list dos móveis.
// Chamar uma vez, depois de iniciaTexturas() e iniciaMapa().
void iniciaSala();

// Desenha os móveis da sala (as paredes, o piso e o teto são do mapa).
void desenhaSala();

// Conta o tempo do escurecimento da passagem pela porta. dt em segundos.
void atualizaSala(float dt);

// Tecla de interação: perto da porta, entra na sala de aula (ou sai dela).
// Devolve verdadeiro se a tecla foi usada aqui (e não deve chegar ao gato).
bool interagePortaSala();

// Verdadeiro se o círculo de centro (x, z) e raio dado encosta em algum móvel da sala.
bool colideComSala(float x, float z, float raio);

// Verdadeiro se o jogador está dentro da sala de aula.
bool jogadorNaSala();

// Texto de dica para a tela quando o jogador está perto da porta; NULL longe dela.
const char *dicaPortaSala();

// De 0 (nada) a 1 (preto): quanto a tela escurece logo depois de passar pela porta.
float escurecimentoSala();

#endif
