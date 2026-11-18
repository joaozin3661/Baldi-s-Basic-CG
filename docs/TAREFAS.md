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
- [x] (Raimundo, área do João) Cadernos: posicionamento no mapa, desenho e coleta (colisão por esferas)
  Feito em 18/11/2026 (Raimundo, com o Claude) como as 4 folhas de **frequência da faculdade**. Arquivos: `src/frequencia.h/.cpp`
  (`sorteiaFrequencias()` chamada por `comecaPartida()`: a cada partida sorteia 4 células de corredor longe da entrada e afastadas
  entre si; cada folha cai no chão, deitada e girada, ou fica presa numa parede de tijolo/armário; coleta automática a 0,9 unidade;
  brilho próprio pulsante via `GL_EMISSION`), `src/texturas.cpp` (`TEX_FREQUENCIA`), `src/telas.cpp` (indicador "Frequencia: n/4" e
  "4/4 - sala liberada!", aviso ao coletar), `src/main.cpp` (`srand(time)` depois das texturas). O progresso usa
  `jogo.cadernosColetados/cadernosTotal`, então a tranca da sala e a velocidade do perseguidor já reagem. Testado: compila sem avisos;
  teste fora do repositório mostrou posições diferentes em 3 partidas, coleta 1/4 a 4/4 sem pontos de coleta dentro de parede,
  porta trancada antes e liberada depois, e capturas das folhas no chão e na parede e do indicador. Não testado jogando.
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
- [x] (Raimundo, com o Claude) Gato guia e barra de felicidade
  Feito em 18/11/2026. Arquivos: `src/gato.h/.cpp` (agora `gatos[2]`: o laranja e o guia cinza de coleira; estados novos
  `CORRENDO` e `INDO`; teste de campo de visão do aluno; `iniciaGatos/atualizaGatos/desenhaGatos`), `src/caminho.h/.cpp`
  (`distanciaNaGrade()`, a BFS foi separada em `buscaEmLargura()`), `src/frequencia.h/.cpp` (`quantidadeFrequencias()`,
  `celulaFrequenciaPendente()`; coleta tira felicidade), `src/jogo.h/.cpp` (`jogo.felicidade`, `mudaFelicidade()`; folha tira 30, carinho dá 15),
  `src/telas.cpp` (barra de felicidade no HUD), `src/mapa.h/.cpp` (símbolo `C` em (5, 4)), `src/main.cpp` (chamadas renomeadas).
  Testado: compila sem avisos com `-Wall`, `timeout 4` mostra os três NPCs nascendo; teste de console fora do repositório
  conferiu a felicidade (inicial, coleta, carinho, limites, recomeço), o gato laranja e 6 corridas do guia com sementes
  diferentes (folha mais próxima pela BFS, 7,0/s do carinho até a folha, mesmo fora de vista, chegada na célula certa), aluno de costas,
  `E` ignorado na corrida, folha pega antes pelo aluno, nenhuma folha pendente e 30 s sem entrar em parede.
  **Não testado jogando:** a barra no HUD e o desenho do guia não foram vistos na janela.
- [ ] (João) Perseguidor: spawn seguro na entrada (`jogo.perseguidorAtivo`) e ajuste de alcance/velocidade
- [ ] (Dupla) HUD 2D sobre a cena: temporizador, contador de cadernos, alerta de proximidade
- [~] (Raimundo) Telas de início, game over e vitória (estados do jogo)
  Tela de início feita em 16/11/2026 (Raimundo). Arquivos: `src/telas.h/.cpp` (projeção 2D com gluOrtho2D, fundo translúcido, texto com
  glutBitmapCharacter): mostra história e comandos e pergunta se o jogador entendeu; ENTER, ESPAÇO ou S confirmam e chamam `comecaPartida()`.
  O jogo agora abre em `JOGO_MENU` com o mouse livre; `main.cpp` chama `desenhaTelas()` e `teclaTelas()`. Derrota e vitória têm uma tela
  mínima (título e ENTER para recomeçar), ainda sem reposicionar os NPCs. Testado: compila sem avisos; captura do menu e da partida após ENTER.

## Fase 3 — Conteúdo

- [x] (João) Porta da sala abre ao coletar todos os cadernos (as 4 folhas de frequência, ver Fase 2)
  18/11/2026 (Raimundo, com o Claude): `src/sala.h/.cpp` faz a sala de aula atrás da `P` (quadro, mesa do professor,
  8 carteiras, colisão com os móveis) e a passagem com `E` (entra e sai, escurecimento, dica na tela). A tranca já
  consulta `todosCadernosColetados()`; com as folhas de frequência, fica trancada até 4/4.
  Testado: compila sem avisos, `timeout 4` carrega ("Sala de aula: porta em (14, 17), 9 moveis") e capturas fora do
  repositório mostram a sala, o quadro e a placa da porta. A passagem com `E` não foi testada jogando.
- [ ] (João) Boss fight com o Professor (mecânica a decidir pelos dois)
- [ ] (Dupla) Minimapa 2D com a posição do jogador
- [ ] (Dupla) Áudio: passos, coleta, alerta do perseguidor, música

## Fase 4 — Acabamento

- [ ] (Dupla) Modelos 3D (.OBJ) para perseguidor e Professor, modelados no Blender
- [ ] (João) Ajuste de dificuldade: tempo, número de cadernos, velocidades
- [ ] (Quem tiver tempo) Itens auxiliares, se entrarem
- [ ] (Quem tiver tempo) Trocar texturas procedurais por imagens
- [x] (Raimundo, com o Claude) Enfeitar o cenário: armários de escola e portas de sala (não abrem) com placa do nome
  Feito em 18/11/2026. Arquivos: `src/mapa.h` (`CEL_ARMARIO 'A'`, `CEL_SALA_DECOR_PRIMEIRA/ULTIMA` = letras minúsculas),
  `src/mapa.cpp` (matriz com 18 armários e 12 portas de sala em células que já eram parede; tabela `salasDecorativas` com nome e face;
  `desenhaBloco` refeito por faces, `desenhaPlaca` com `glutStrokeCharacter` na display list; `celulaSolida()` trata os símbolos novos
  como parede), `src/texturas.h/.cpp` (`TEX_ARMARIO`, `TEX_PORTA_SALA`; tijolo virou `pintaTijolo()` para emendar com a parede),
  `.claude/agents/cartografo.md` (verificador aceita `A` e minúsculas). As células andáveis são as mesmas de antes (mesma
  alcançabilidade, 2 becos). Testado: verificador ok, compila sem avisos com `-Wall`, `timeout 4` sem "Erro no mapa"; capturas
  fora do repositório (programa de teste com `mapa.cpp` e `texturas.cpp`) mostraram portas, placas legíveis e armários.
  Nenhum outro módulo precisou mudar: NPCs, BFS e colisão usam `celulaSolida()`. O minimapa, quando for feito, deve pintar
  como parede tudo o que `celulaSolida()` diz ser sólido, não só `#`.

## Relatório e entrega

- [ ] (Os dois) Relatório no template da SBC (2 a 6 páginas); cada um escreve os Métodos do que implementou
- [ ] (Os dois) Capturas de tela do jogo (já há três em `docs/img/`)
- [ ] (João) .zip final enviado no SIGAA até 08/12/2026
- [ ] (Os dois) Ensaio da apresentação (09/12 e 14/12): cada um explica a sua parte e entende a do outro
