# Decisões

Registre aqui cada decisão fechada, com data. A mais recente fica no topo.

| Data | Decisão |
|---|---|
| 16/11/2026 | Estado compartilhado em `jogo.h`: enum `EstadoJogo` (menu, jogando, prova, vitória, derrota), tempo restante, cadernos coletados/total e posição e situação do perseguidor. Trocas de estado passam por `mudaEstado()`. |
| 16/11/2026 | Divisão da dupla confirmada: João na jogabilidade e IA, Raimundo na interface e acabamento. |
| 15/11/2026 | A dupla se atualiza por este repositório no GitHub; o contexto fica em `docs/`. |
| 15/11/2026 | Todos os cadernos são obrigatórios para abrir a sala da prova. |
| 15/11/2026 | Jogo inspirado em Baldi's Basics, câmera FPS, OpenGL + freeGLUT em C/C++. |
| 15/11/2026 | O perseguidor acelera a cada caderno coletado; há um temporizador regressivo. |

## Em aberto

- Tempo inicial da partida: `TEMPO_INICIAL` em `jogo.h` está em 180 s, provisório até o ajuste de dificuldade.
- Mecânica do boss fight (a "prova").
- Número de cadernos no mapa.
- Itens auxiliares (energético, mapa parcial, distração): entram ou não.
- Desenho do labirinto.
- Papel do indicador de "nota": com todos os cadernos obrigatórios, ele sempre chega ao máximo.
- Bibliotecas de áudio, de carregamento de imagem e de modelos 3D.

## Propostas de NPC (Raimundo, 16/11/2026, a decidir com o João)

- **Gato da faculdade:** anda pelos corredores e pode ser acariciado ao chegar perto e apertar uma tecla.
  Por enquanto só carinho; fica como gancho para uma mecânica futura (por exemplo, o gato distrair o perseguidor).
- **Aluno do bar:** segundo perseguidor, um colega que quer te levar para o bar. Se te alcança, te agarra e começa a te
  arrastar na direção do bar; para se soltar, o jogador tem que se mexer várias vezes (apertar as teclas de andar
  repetidamente). Contaria como mecânica nova em relação ao Baldi's Basics.
- Os dois NPCs entram na área de IA. Como ela é do João, a divisão precisa ser ajustada se o Raimundo for implementá-los.
