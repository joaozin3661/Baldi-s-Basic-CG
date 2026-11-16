# Labirinto da Faculdade

Jogo 3D em primeira pessoa feito com OpenGL + GLUT (freeGLUT) em C/C++, inspirado em Baldi's Basics.
Trabalho Final da disciplina de Computação Gráfica (UFPI, Prof. Laurindo de Sousa Britto Neto).

O jogador é um aluno atrasado para a prova: precisa coletar todos os cadernos do labirinto,
fugir do perseguidor (que acelera a cada caderno) e chegar à sala de aula antes do tempo acabar
para enfrentar o Professor.

## Estrutura

- `src/` — código-fonte
- `assets/` — texturas, modelos 3D e áudio
- `docs/CONTEXTO.md` — conceito do jogo, requisitos do professor e prazos
- `docs/DECISOES.md` — decisões já tomadas, com data
- `docs/TAREFAS.md` — o que está feito, em andamento e com quem

## Como a dupla se mantém atualizada

1. Antes de começar: `git pull`.
2. Ao abrir uma sessão com o Claude, pedir: "leia a pasta `docs/` antes de qualquer coisa".
3. Ao terminar: pedir para atualizar `docs/DECISOES.md` e `docs/TAREFAS.md`, depois `git add`, `git commit` e `git push`.

## Compilação

Linux:

    g++ -o labirinto src/*.cpp -lGL -lGLU -lglut

Windows (MinGW / Falcon C++), parâmetros do linker:

    -lfreeglut -lglu32 -lopengl32 -lwinmm -lgdi32
