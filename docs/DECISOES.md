# Decisões

Registre aqui cada decisão fechada, com data. A mais recente fica no topo.

| Data | Decisão |
|---|---|
| 16/11/2026 | **Perseguidor agarra e arrasta** (substitui a volta instantânea ao início). Ao encostar no aluno (esfera x esfera), entra em `ARRASTANDO`: o aluno fica preso logo atrás dele e é levado pelo caminho mais curto (BFS) até a célula `S`, a 1,4 unidade/s. Enquanto isso o aluno não anda; cada tecla de andar apertada (`W A S D` ou setas, sem repetição automática) soma 1 de "força", que decai 2 por segundo; com 6 ele se solta onde está. Sem se soltar, é deixado na célula `S` (`iniciaJogador()`, sem mexer em cadernos nem no tempo). Nos dois casos o perseguidor fica 3 s parado (`RECUPERANDO`) e ignora o aluno por 5 s de trégua. A derrota continua vindo só do tempo esgotado. |
| 16/11/2026 | **Gato da faculdade entra no jogo** (`src/gato`): vaga pela grade, às vezes senta, e aceita carinho com a tecla `E` quando o jogador está a até 1,2 unidade (esfera x esfera). Sem punição e sem efeito no progresso; fica como gancho para mecânica futura. |
| 16/11/2026 | **Perseguidor** (`src/perseguidor`): máquina de estados `VAGANDO` / `PERSEGUINDO` / `ARRASTANDO` / `RECUPERANDO`. Percebe o jogador a até 7 unidades **com linha de visão** (não vê através de parede); desiste a mais de 11 unidades. Segue por BFS na grade (`src/caminho`), recalculada a cada célula alcançada. Velocidade = 2,0 + 0,35 por caderno coletado. |
| 16/11/2026 | Símbolos de NPC no mapa: `G` gato, `X` perseguidor (`P` já era a porta). Os dois contam como chão em `celulaSolida()`. |
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
- Spawn seguro: `jogo.perseguidorAtivo` hoje fica sempre verdadeiro; falta decidir se a entrada é zona sem perseguidor.
- Valores de alcance, velocidade, tempo de recuperação, trégua e força/decaimento da sacudida do perseguidor (ajuste de dificuldade, Fase 4). Do canto oposto do mapa, o arrasto até `S` leva uns 40 s: se for longo demais, subir `PERSEGUIDOR_VEL_ARRASTO`.
- Feedback visual da sacudida no HUD (hoje só sai no console "Aluno se debate: n/6").
- O jogador atravessa os NPCs (só o mapa bloqueia o jogador); o gato é quem para ao encostar nele.
- Mecânica futura do gato (por exemplo, distrair o perseguidor).

## Propostas de NPC (Raimundo, 16/11/2026)

- **Gato da faculdade:** implementado em 16/11/2026 (ver tabela acima).
- **Aluno do bar:** segundo perseguidor, um colega que quer te levar para o bar. Se te alcança, te agarra e começa a te
  arrastar na direção do bar; para se soltar, o jogador tem que se mexer várias vezes (apertar as teclas de andar
  repetidamente). Contaria como mecânica nova em relação ao Baldi's Basics. Em 16/11/2026 essa mecânica de agarrar e
  arrastar foi adotada pelo próprio perseguidor (ver tabela); se o aluno do bar for feito, pode reaproveitar o estado
  `ARRASTANDO` com outro destino.
- Os NPCs entram na área de IA, que é do João. O perseguidor e o gato foram implementados pelo Raimundo em
  16/11/2026 (com o Claude); a divisão precisa ser ajustada e o João precisa revisar `perseguidor`, `caminho` e `mapa`.
