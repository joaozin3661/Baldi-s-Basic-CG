#include "texturas.h"
#include <cstdlib>

#define TEX_TAM 64   // largura e altura em texels (potência de dois, como recomenda a aula 23)

GLuint idTexturas[QUANT_TEX];

static unsigned char imagem[TEX_TAM * TEX_TAM * 3];

// Grava a cor (r, g, b) no texel (x, y), limitando cada canal a [0, 255].
static void pintaTexel(int x, int y, int r, int g, int b) {
    int canais[3] = { r, g, b };
    for (int c = 0; c < 3; c++) {
        if (canais[c] < 0)   canais[c] = 0;
        if (canais[c] > 255) canais[c] = 255;
        imagem[(y * TEX_TAM + x) * 3 + c] = (unsigned char) canais[c];
    }
}

// Pequena variação aleatória, para a textura não ficar "chapada".
static int ruido(int amplitude) {
    return rand() % (2 * amplitude + 1) - amplitude;
}

// Um texel de tijolo: fileiras de 16 texels, tijolos de 32, fileiras alternadas deslocadas.
// Também é usado em volta da porta de sala e acima dos armários, para emendar com a parede ao lado.
static void pintaTijolo(int x, int y) {
    int fileira = y / 16;
    int deslocamento = (fileira % 2) * 16;
    bool rejunte = (y % 16 < 2) || ((x + deslocamento) % 32 < 2);
    int n = ruido(12);
    if (rejunte) pintaTexel(x, y, 190 + n, 185 + n, 175 + n);
    else         pintaTexel(x, y, 165 + n, 80 + n, 60 + n);
}

// Parede de tijolos.
static void geraParede() {
    for (int y = 0; y < TEX_TAM; y++)
        for (int x = 0; x < TEX_TAM; x++)
            pintaTijolo(x, y);
}

// Piso em xadrez (mapa geométrico da aula 26): quatro ladrilhos de 32 x 32.
static void geraPiso() {
    for (int y = 0; y < TEX_TAM; y++) {
        for (int x = 0; x < TEX_TAM; x++) {
            bool claro = ((x / 32) + (y / 32)) % 2 == 0;
            bool borda = (x % 32 == 0) || (y % 32 == 0);
            int n = ruido(6);
            if (borda)      pintaTexel(x, y, 70 + n, 70 + n, 75 + n);
            else if (claro) pintaTexel(x, y, 205 + n, 205 + n, 195 + n);
            else            pintaTexel(x, y, 120 + n, 135 + n, 150 + n);
        }
    }
}

// Teto de forro: placas claras separadas por uma grade.
static void geraTeto() {
    for (int y = 0; y < TEX_TAM; y++) {
        for (int x = 0; x < TEX_TAM; x++) {
            bool grade = (x % 32 < 2) || (y % 32 < 2);
            int n = ruido(10);
            if (grade) pintaTexel(x, y, 140 + n, 140 + n, 140 + n);
            else       pintaTexel(x, y, 225 + n, 225 + n, 215 + n);
        }
    }
}

// Porta de madeira: tábuas verticais, moldura escura e uma maçaneta.
static void geraPorta() {
    for (int y = 0; y < TEX_TAM; y++) {
        for (int x = 0; x < TEX_TAM; x++) {
            bool moldura = x < 4 || x >= TEX_TAM - 4 || y >= TEX_TAM - 4;
            bool fresta = (x % 14 == 0);
            bool macaneta = (x - 52) * (x - 52) + (y - 30) * (y - 30) < 9;
            int n = ruido(8);
            if (macaneta)     pintaTexel(x, y, 220, 190, 60);
            else if (moldura) pintaTexel(x, y, 60 + n, 35 + n, 20 + n);
            else if (fresta)  pintaTexel(x, y, 80 + n, 50 + n, 25 + n);
            else              pintaTexel(x, y, 130 + n, 85 + n, 45 + n);
        }
    }
}

// Armários de escola, de metal azul. A face da célula (2 x 2,5 unidades) recebe a imagem inteira:
// a linha y = 0 da imagem fica no chão e x = 0 fica à esquerda de quem olha a face de frente.
// Quatro portinhas de 16 texels (0,5 unidade) separadas por frestas, cada uma com frestas de
// ventilação em cima, plaquinha de número, puxador e, em portinhas alternadas, um cadeado.
// Embaixo, um rodapé escuro; acima dos armários (y >= 56), tijolos como os da parede.
static void geraArmario() {
    for (int y = 0; y < TEX_TAM; y++) {
        for (int x = 0; x < TEX_TAM; x++) {
            if (y >= 56) { pintaTijolo(x, y); continue; }

            int lx = x % 16;          // posição dentro da portinha
            bool comCadeado = (x / 16) % 2 == 1;
            int n = ruido(5);

            if (y < 4)        { pintaTexel(x, y, 45 + n, 45 + n, 50 + n); continue; }   // rodapé
            if (y >= 52)      { pintaTexel(x, y, 55 + n, 80 + n, 115 + n); continue; }  // tampo

            // Brilho metálico: a chapa é um pouco mais clara no meio de cada portinha.
            int brilho = (lx < 8 ? lx : 15 - lx) * 3;
            int r = 60 + brilho + n, g = 100 + brilho + n, b = 150 + brilho + n;

            bool fresta     = (lx == 0);
            bool bordaClara = (lx == 1 || y == 51);
            bool bordaEscura = (lx == 15 || y == 4);
            bool ventilacao = (y >= 42 && y <= 49 && y % 2 == 0 && lx >= 4 && lx <= 11);
            bool numero     = (y >= 37 && y <= 39 && lx >= 6 && lx <= 9);
            bool puxador    = (y >= 24 && y <= 31 && lx >= 12 && lx <= 13);
            bool cadeado    = comCadeado && y >= 19 && y <= 22 && lx >= 11 && lx <= 14;
            bool argola     = comCadeado && y == 23 && (lx == 11 || lx == 14);

            if (fresta)                 pintaTexel(x, y, 20, 25, 35);
            else if (puxador)           pintaTexel(x, y, 205 + n, 205 + n, 210 + n);
            else if (cadeado || argola) pintaTexel(x, y, 190 + n, 155 + n, 60 + n);
            else if (ventilacao)        pintaTexel(x, y, 25, 30, 40);
            else if (numero)            pintaTexel(x, y, 225 + n, 225 + n, 215 + n);
            else if (bordaClara)        pintaTexel(x, y, r + 40, g + 40, b + 40);
            else if (bordaEscura)       pintaTexel(x, y, r - 30, g - 30, b - 30);
            else                        pintaTexel(x, y, r, g, b);
        }
    }
}

// Porta de sala de aula (só cenário, não abre), no meio de uma parede de tijolos.
// Diferente de TEX_PORTA (tábuas e maçaneta redonda): folha lisa de madeira clara com duas
// almofadas, batente escuro em volta e maçaneta de alavanca. Mesma orientação de geraArmario():
// a porta vai de x = 13 a 50 (pouco mais de 1 unidade de largura) e do chão até y = 52 (~2 unidades).
static void geraPortaSala() {
    for (int y = 0; y < TEX_TAM; y++) {
        for (int x = 0; x < TEX_TAM; x++) {
            if (x < 13 || x > 50 || y > 52) { pintaTijolo(x, y); continue; }

            int n = ruido(6);
            bool batente = (x <= 15 || x >= 48 || y >= 50);
            if (batente) {
                // Quina interna do batente mais escura, para dar profundidade.
                bool quina = (x == 15 || x == 48 || y == 50);
                if (quina) pintaTexel(x, y, 45 + n, 30 + n, 18 + n);
                else       pintaTexel(x, y, 85 + n, 55 + n, 32 + n);
                continue;
            }

            // Folha da porta: madeira clara com veios verticais.
            bool veio = ((x * 5 + y / 8) % 7 == 0);
            int r = 180 + n, g = 130 + n, b = 80 + n;
            if (veio) { r -= 18; g -= 14; b -= 10; }

            // Almofadas (painéis rebaixados): borda de cima na sombra, borda de baixo iluminada.
            bool almofadaBaixo = (x >= 20 && x <= 43 && y >= 5 && y <= 20);
            bool almofadaCima  = (x >= 20 && x <= 43 && y >= 27 && y <= 46);
            bool sombra = (almofadaBaixo && (y == 20 || x == 20)) || (almofadaCima && (y == 46 || x == 20));
            bool luz    = (almofadaBaixo && (y == 5 || x == 43))  || (almofadaCima && (y == 27 || x == 43));

            // Maçaneta de alavanca do lado direito, com espelho redondo e fechadura embaixo.
            bool espelho   = (x - 44) * (x - 44) + (y - 24) * (y - 24) <= 4;
            bool alavanca  = (y >= 24 && y <= 25 && x >= 37 && x <= 44);
            bool fechadura = (x == 44 && y >= 19 && y <= 20);

            if (fechadura)                 pintaTexel(x, y, 20, 20, 20);
            else if (alavanca)             pintaTexel(x, y, 215 + n, 215 + n, 220 + n);
            else if (espelho)              pintaTexel(x, y, 150 + n, 150 + n, 155 + n);
            else if (sombra)               pintaTexel(x, y, r - 45, g - 40, b - 30);
            else if (luz)                  pintaTexel(x, y, r + 25, g + 22, b + 18);
            else if (almofadaBaixo || almofadaCima) pintaTexel(x, y, r - 10, g - 10, b - 8);
            else                           pintaTexel(x, y, r, g, b);
        }
    }
}

// Quadro-negro verde-escuro de escola, com rastros claros de giz apagado.
// A imagem se repete ao longo do quadro (GL_REPEAT).
static void geraQuadro() {
    for (int y = 0; y < TEX_TAM; y++) {
        for (int x = 0; x < TEX_TAM; x++) {
            int n = ruido(5);
            bool rastro = ((x + 2 * y) % 23 < 6) && ((x / 9 + y / 13) % 3 == 0);   // apagador
            int giz = rastro ? 14 : 0;
            pintaTexel(x, y, 30 + n + giz, 62 + n + giz, 45 + n + giz);
        }
    }
}

// Madeira clara dos tampos das carteiras e da mesa do professor: veios ondulados ao longo de x.
static void geraMadeira() {
    for (int y = 0; y < TEX_TAM; y++) {
        for (int x = 0; x < TEX_TAM; x++) {
            int n = ruido(6);
            int onda = (x / 6) % 3;                     // o veio sobe e desce devagar
            bool veio = ((y + onda) % 8 == 0);
            bool anel = ((y + onda) % 8 == 1) && (x % 21 < 9);
            int r = 190 + n, g = 145 + n, b = 95 + n;
            if (veio)      { r -= 35; g -= 30; b -= 22; }
            else if (anel) { r -= 15; g -= 12; b -= 10; }
            pintaTexel(x, y, r, g, b);
        }
    }
}

// Folha da lista de frequência: papel claro com a faixa do título em cima (o texto
// "FREQUENCIA" é escrito por cima, em frequencia.cpp), uma tabela de nomes e assinaturas
// e um carimbo vermelho da coordenação no canto. y = 0 é o pé da folha.
static void geraFrequencia() {
    for (int y = 0; y < TEX_TAM; y++) {
        for (int x = 0; x < TEX_TAM; x++) {
            int n = ruido(4);
            int r = 245 + n, g = 242 + n, b = 228 + n;   // papel
            bool borda   = (x < 2 || x > 61 || y < 2 || y > 61);
            bool tabela  = (y >= 6 && y <= 50) && (x >= 5 && x <= 58);
            bool linhaH  = tabela && (y - 6) % 5 == 0;
            bool linhaV  = tabela && (x == 5 || x == 13 || x == 40 || x == 58);
            bool nome    = tabela && !linhaH && x > 14 && x < 38 && ((y - 6) % 5 == 2)
                           && ((x * 7 + y * 3) % 5 != 0);                   // "nomes" a lápis
            bool assina  = tabela && !linhaH && x > 42 && x < 56 && ((y - 6) % 5 == 2 || (y - 6) % 5 == 3)
                           && ((x + y) % 4 < 2) && y < 36;                  // rubricas (falta gente)
            int dx = x - 50, dy = y - 12;
            bool carimbo = dx * dx + dy * dy <= 49 && dx * dx + dy * dy >= 30;

            if (borda)             pintaTexel(x, y, r - 40, g - 40, b - 40);
            else if (carimbo)      pintaTexel(x, y, 200, 40 + n, 40 + n);
            else if (linhaH || linhaV) pintaTexel(x, y, 120, 140, 190);
            else if (nome)         pintaTexel(x, y, 90 + n, 90 + n, 100 + n);
            else if (assina)       pintaTexel(x, y, 30, 40, 120);
            else                   pintaTexel(x, y, r, g, b);
        }
    }
}

// Envia a imagem atual para o OpenGL (passos da aula 23).
static void enviaTextura(GLuint id) {
    glBindTexture(GL_TEXTURE_2D, id);
    // Mip-mapping (aula 26): reduz o efeito moiré nas paredes distantes.
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, TEX_TAM, TEX_TAM,
                      GL_RGB, GL_UNSIGNED_BYTE, imagem);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // GL_REPEAT: coordenadas fora de [0, 1] repetem a imagem.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void iniciaTexturas() {
    srand(7);   // semente fixa: as texturas saem iguais em toda execução
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(QUANT_TEX, idTexturas);

    geraParede(); enviaTextura(idTexturas[TEX_PAREDE]);
    geraPiso();   enviaTextura(idTexturas[TEX_PISO]);
    geraTeto();   enviaTextura(idTexturas[TEX_TETO]);
    geraPorta();  enviaTextura(idTexturas[TEX_PORTA]);
    // As novas vêm depois das antigas: assim a sequência de rand() das antigas não muda.
    geraArmario();   enviaTextura(idTexturas[TEX_ARMARIO]);
    geraPortaSala(); enviaTextura(idTexturas[TEX_PORTA_SALA]);
    geraQuadro();    enviaTextura(idTexturas[TEX_QUADRO]);
    geraMadeira();   enviaTextura(idTexturas[TEX_MADEIRA]);
    geraFrequencia(); enviaTextura(idTexturas[TEX_FREQUENCIA]);

    // GL_MODULATE: a cor da textura é multiplicada pela cor calculada pela iluminação.
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);
}
