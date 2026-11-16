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

// Parede de tijolos: fileiras de 16 texels, tijolos de 32, fileiras alternadas deslocadas.
static void geraParede() {
    for (int y = 0; y < TEX_TAM; y++) {
        for (int x = 0; x < TEX_TAM; x++) {
            int fileira = y / 16;
            int deslocamento = (fileira % 2) * 16;
            bool rejunte = (y % 16 < 2) || ((x + deslocamento) % 32 < 2);
            int n = ruido(12);
            if (rejunte) pintaTexel(x, y, 190 + n, 185 + n, 175 + n);
            else         pintaTexel(x, y, 165 + n, 80 + n, 60 + n);
        }
    }
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

    // GL_MODULATE: a cor da textura é multiplicada pela cor calculada pela iluminação.
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);
}
