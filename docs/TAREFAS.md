# Tarefas

Atualize ao fim de cada sessão. Estados: `[ ]` a fazer, `[~]` em andamento, `[x]` feito.
Coloque o nome de quem pegou a tarefa.

## Fase 1 — Estrutura (com os requisitos gráficos desde o início)

- [x] Labirinto em matriz, com paredes, chão e teto em quads com normais e coordenadas de textura
- [x] Câmera FPS com teclado e mouse
- [x] Colisão do jogador com as paredes
- [x] Z-buffer e back-face culling
- [x] Iluminação básica e materiais
- [x] Texturas em parede, chão e teto

Feito em 15/11/2026 (João). Arquivos: `src/main.cpp`, `mapa`, `jogador`, `texturas`. Testado no Linux e no Windows (g++ do Falcon C++, com `-lglut32`).
As texturas são procedurais (geradas em `texturas.cpp`); trocar por imagens fica para a Fase 4, se der tempo.

## Divisão da dupla (proposta de 15/11/2026, ajustar se quiserem)

Cada um trabalha em arquivos próprios, para evitar conflito no Git. O `main.cpp` só recebe as chamadas
(`inicia...`, `atualiza...`, `desenha...`); quem mexer nele avisa o outro e dá push logo.

| Quem | Área | Arquivos |
|---|---|---|
| João | Jogabilidade e IA: cadernos, perseguidor, porta, boss fight | `cadernos`, `perseguidor`, `chefe`, `mapa` |
| Dupla (nome a preencher) | Interface e acabamento: HUD, telas, áudio, modelos 3D, minimapa | `hud`, `telas`, `audio`, `modelos` |
| Os dois | Estado compartilhado do jogo, relatório, apresentação | `jogo.h`, `relatorio/` |

O `jogo.h` guarda o que os dois lados precisam: cadernos coletados e total, tempo restante,
estado do jogo (menu, jogando, prova, vitória, derrota) e a posição do perseguidor. Ele deve ser criado primeiro.

## Fase 2 — Gameplay

- [ ] (Os dois) `jogo.h` com o estado compartilhado
- [ ] (João) Cadernos: posicionamento no mapa, desenho e coleta (colisão por esferas)
- [ ] (João) Perseguidor: máquina de estados (patrulha e perseguição) e busca de caminho na grade
- [ ] (João) Velocidade do perseguidor aumenta por caderno
- [ ] (Dupla) HUD 2D sobre a cena: temporizador, contador de cadernos, alerta de proximidade
- [ ] (Dupla) Telas de início, game over e vitória (estados do jogo)

## Fase 3 — Conteúdo

- [ ] (João) Porta da sala abre ao coletar todos os cadernos
- [ ] (João) Boss fight com o Professor (mecânica a decidir pelos dois)
- [ ] (Dupla) Minimapa 2D com a posição do jogador
- [ ] (Dupla) Áudio: passos, coleta, alerta do perseguidor, música

## Fase 4 — Acabamento

- [ ] (Dupla) Modelos 3D (.OBJ) para perseguidor e Professor, modelados no Blender
- [ ] (João) Ajuste de dificuldade: tempo, número de cadernos, velocidades
- [ ] (Quem tiver tempo) Itens auxiliares, se entrarem
- [ ] (Quem tiver tempo) Trocar texturas procedurais por imagens

## Relatório e entrega

- [ ] (Os dois) Relatório no template da SBC (2 a 6 páginas); cada um escreve os Métodos do que implementou
- [ ] (Os dois) Capturas de tela do jogo (já há três em `docs/img/`)
- [ ] (João) .zip final enviado no SIGAA até 08/12/2026
- [ ] (Os dois) Ensaio da apresentação (09/12 e 14/12): cada um explica a sua parte e entende a do outro
