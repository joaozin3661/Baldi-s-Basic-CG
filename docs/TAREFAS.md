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
| João | Jogabilidade e IA: cadernos, perseguidor, porta, boss fight | `cadernos`, `perseguidor`, `caminho`, `chefe`, `mapa` |
| Raimundo | Interface e acabamento: HUD, telas, áudio, modelos 3D, minimapa, gato | `hud`, `telas`, `audio`, `modelos`, `gato` |
| Os dois | Estado compartilhado do jogo, relatório, apresentação | `jogo.h`, `relatorio/` |

O `jogo.h` guarda o que os dois lados precisam: cadernos coletados e total, tempo restante,
estado do jogo (menu, jogando, prova, vitória, derrota) e a posição do perseguidor. Ele deve ser criado primeiro.

## Fase 2 — Gameplay

- [x] (Os dois) `jogo.h` com o estado compartilhado
  Feito em 16/11/2026 (Raimundo). Arquivos: `src/jogo.h`, `src/jogo.cpp`. O `main.cpp` chama `iniciaJogo()`, `comecaPartida()` e `atualizaJogo(dt)`,
  e só move o jogador em `JOGO_JOGANDO`. Enquanto não há telas, a partida começa direto. Compila sem avisos com `-Wall`.
- [ ] (João) Cadernos: posicionamento no mapa, desenho e coleta (colisão por esferas)
- [x] (Raimundo, área do João) Perseguidor: máquina de estados (patrulha e perseguição) e busca de caminho na grade
  Feito em 16/11/2026 (Raimundo, com o Claude). Arquivos: `src/perseguidor.h/.cpp` (estados `VAGANDO`, `PERSEGUINDO`, `ARRASTANDO`,
  `RECUPERANDO`; percepção por raio + linha de visão; contato esfera x esfera agarra o jogador e o arrasta por BFS até `S`, o jogador
  se solta apertando as teclas de andar várias vezes: `perseguidorAgarrouJogador()` e `sacodeJogador()` chamadas pelo `main.cpp`), `src/caminho.h/.cpp` (BFS `proximoPasso`,
  `sorteiaVizinho`, `haLinhaDeVisao`, célula <-> mundo, compartilhado por todos os NPCs), `src/mapa.h/.cpp` (símbolo `X` e célula no mapa),
  `src/main.cpp` (chamadas `inicia`/`atualiza`/`desenha`). Testado: compila sem avisos com `-Wall`; `timeout 4` mostra as transições no
  console; teste de console fora do repositório simulou quadros e conferiu BFS, linha de visão, perseguição, contato, arrasto com o
  jogador junto (sem entrar em parede), sacudida lenta que não solta e rápida que solta, entrega na célula `S`, trégua e
  que ninguém entra em parede. **O João precisa revisar.**
- [x] (Raimundo, área do João) Velocidade do perseguidor aumenta por caderno
  Feito em 16/11/2026: `velocidade = PERSEGUIDOR_VEL_BASE + PERSEGUIDOR_VEL_POR_CADERNO * jogo.cadernosColetados`, lida a cada quadro.
- [x] (Raimundo) Gato da faculdade: vaga, senta e aceita carinho com `E`
  Feito em 16/11/2026. Arquivos: `src/gato.h/.cpp` (estados `VAGANDO`, `PARADO`, `ACARICIADO`; `interageComGato()` chamada pelo `main.cpp`
  na tecla `E`), símbolo `G` em `mapa.h/.cpp`. Mesmo teste do perseguidor.
- [ ] (João) Perseguidor: spawn seguro na entrada (`jogo.perseguidorAtivo`) e ajuste de alcance/velocidade
- [ ] (Dupla) HUD 2D sobre a cena: temporizador, contador de cadernos, alerta de proximidade
- [~] (Raimundo) Telas de início, game over e vitória (estados do jogo)
  Tela de início feita em 16/11/2026 (Raimundo). Arquivos: `src/telas.h/.cpp` (projeção 2D com gluOrtho2D, fundo translúcido, texto com
  glutBitmapCharacter): mostra história e comandos e pergunta se o jogador entendeu; ENTER, ESPAÇO ou S confirmam e chamam `comecaPartida()`.
  O jogo agora abre em `JOGO_MENU` com o mouse livre; `main.cpp` chama `desenhaTelas()` e `teclaTelas()`. Derrota e vitória têm uma tela
  mínima (título e ENTER para recomeçar), ainda sem reposicionar os NPCs. Testado: compila sem avisos; captura do menu e da partida após ENTER.

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
