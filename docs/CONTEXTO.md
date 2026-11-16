# Contexto do projeto — Labirinto da Faculdade

Leia este arquivo, `DECISOES.md` e `TAREFAS.md` antes de trabalhar no projeto.

## Conceito

Jogo 3D em primeira pessoa (OpenGL + freeGLUT, C/C++) inspirado em Baldi's Basics, com tema universitário.
Um aluno percorre o labirinto (a faculdade), coleta todos os cadernos para "estudar" e chega à sala de aula
antes do tempo acabar para enfrentar o Professor em um boss fight que representa a prova final.

## Entidades

- **Jogador:** câmera FPS, anda pelo labirinto e coleta itens.
- **Perseguidor:** patrulha o labirinto e acelera a cada caderno coletado.
- **Professor (chefe):** aguarda na sala de aula final.
- **Cadernos:** espalhados pelo mapa; todos são obrigatórios para abrir a sala final.

## HUD

Temporizador regressivo, contador de cadernos (X/N), minimapa 2D com a posição do jogador,
alerta de proximidade do perseguidor e indicador de "nota".

## Mapa

Entrada da faculdade (spawn, sem perseguidor) → corredores e salas (cadernos, perseguidor) →
sala de aula final (trancada até coletar todos os cadernos, contém o Professor).

## Requisitos do professor (Trabalho Final)

- Jogo 3D em janela GLUT, tema livre, inspirado em um jogo existente.
- Mecânicas novas que não existem no jogo original.
- Detecção de colisões (em aula: esferas envolventes, AABB, OBB).
- Inteligência Artificial (em aula: IA reativa com máquina de estados finitos).
- Recursos do OpenGL: **iluminação, sombreamento, textura e visibilidade**.
- Bibliotecas complementares ("pode e deve"): áudio, modelos 3D (.OBJ) etc.

## Prazos e entrega

- **Entrega: até 08/12/2026 às 23h59 no SIGAA**, em um .zip com código-fonte, assets e o relatório em PDF.
  Executável não é necessário. Não há entrega atrasada.
- **Apresentações: 09/12/2026 e 14/12/2026.** Faltar a qualquer dia reduz a nota.
- Trabalho em dupla, 10,0 pontos. A nota pode ser individual conforme o desempenho na apresentação,
  então os dois precisam saber explicar todo o código.
- Cópia detectada (entre alunos, da internet etc.) zera a nota. Sem relatório ou sem apresentação, zero.

## Relatório técnico (obrigatório)

PDF no template de artigos da SBC, de 2 a 6 páginas, com: Título e autores (nomes e matrículas), Resumo,
Introdução, Materiais, Métodos, Resultados e Discussão (com capturas de tela), Conclusão e Referências.

## Técnicas vistas em aula que o jogo deve aproveitar

- Mapa como matriz, como no exemplo do Pacman 3D: 0 vazio, 1 parede, códigos para itens e personagens.
- As primitivas `glutSolid*` têm normais, mas não têm coordenadas de textura (exceto o Teapot).
  Paredes, chão e teto texturizados precisam ser quads próprios com `glNormal` e `glTexCoord`.
- Malhas com normais de face e de vértice; `glShadeModel(GL_FLAT)` e `glShadeModel(GL_SMOOTH)`.
- Luz ambiente, difusa e especular; materiais; texturas com `glGenTextures`, `glBindTexture`,
  `glTexImage2D`, filtros e `GL_MODULATE`.
- Visibilidade: z-buffer (`GL_DEPTH_TEST`, `GLUT_DEPTH`) e back-face culling (`GL_CULL_FACE`).
- Texto do HUD com `glutBitmapCharacter`.
- Blender para modelar e animar personagens.
