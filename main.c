/*

Para compilar de dentro do VS Code:

Windows/Linux: CTRL+SHIFT+B (Terminal -> Run Build Task)
macOS: COMMAND+SHIFT+B

Para executar de dentro do VS Code:

Executar normalmente: CTRL+F5 (Run -> Run Without Debugging)
Debugar: F5 (Run -> Start Debugging)

Pelo terminal:

Windows: mingw32-make
Linux/macOS: make

Para executar:

./bagunceitor [arquivo com a imagem de entrada]

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Para usar strings
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>
#include <stb_image_write.h>

// Um pixel Pixel (24 bits)
typedef struct {
  unsigned char r, g, b;
} Pixel;

// Uma imagem Pixel
typedef struct {
  int width, height;  // largura, altura
  int channels;       // qtd de canais (geralmente 3, RGB)
  Pixel* pixels;
} Img;

// As 2 imagens
Img in, out;

// =====================================================================
// PROTÓTIPOS
// =====================================================================
void load(char* name, Img* pic);

// Confusão - Camada 1: Rotação de bits
unsigned char rotacionar_esquerda(unsigned char byte, int n);
unsigned char rotacionar_direita(unsigned char byte, int n);
void confusao_rotacao(unsigned char *dados, int total_bytes);
void confusao_rotacao_inversa(unsigned char *dados, int total_bytes);

// Confusão - Camada 2: XOR por posição
void confusao_xor(unsigned char *dados, int total_bytes);

// Difusão - Camada 3: Arnold Cat Map
void difusao_arnold(Pixel *entrada, Pixel *saida, int width, int height);
void difusao_arnold_inversa(Pixel *entrada, Pixel *saida, int width, int height);

// Verificação
int verificar_recuperacao(unsigned char *original, unsigned char *recuperada, int total_bytes);

// =====================================================================
// MAIN
// =====================================================================
int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("bagunceitor [origem]\n");
    exit(1);
  }

  // Carrega a imagem original
  load(argv[1], &in);

  // Exibe as dimensões na tela, para conferência
  printf("Origem   : %s %d x %d\n", argv[1], in.width, in.height);

  printf("Processando...\n");

  // Cria imagem de saída e "zera" ela
  int tam = in.width * in.height;
  out = in;
  out.pixels = malloc(tam * sizeof(Pixel));
  memset(out.pixels, 0, tam * sizeof(Pixel));

  int total_bytes = tam * 3;

  // Converte para interpretar como matrizes
  Pixel(*pin)[in.width] = (Pixel(*)[in.height])in.pixels;
  Pixel(*pout)[in.width] = (Pixel(*)[in.height])out.pixels;

  // =====================================================================
  // Guarda uma cópia da imagem original para verificação depois
  // =====================================================================
  unsigned char *copia_original = malloc(total_bytes);
  unsigned char *p_src = (unsigned char *)in.pixels;
  unsigned char *p_dst = copia_original;
  for(int i=0; i<total_bytes; i++){
    *p_dst = *p_src;
    p_src++;
    p_dst++;
  }

  // =====================================================================
  // Bangunçar
  // =====================================================================
  printf("\n=== BAGUNÇANDO ===\n");

  // Passo 1: Confusão - Camada 1 (Rotação de bits)
  printf("[1] Aplicando rotação de bits...\n");
  unsigned char *p_in = (unsigned char *)in.pixels;
  confusao_rotacao(p_in, total_bytes);

  // Passo 2: Confusão - Camada 2 (XOR por posição)
  printf("[2] Aplicando XOR por posição...\n");
  confusao_xor(p_in, total_bytes);

  // Passo 3: Difusão - Camada 3 (Arnold Cat Map)
  // Copia os resultados da confusão para out, depois aplica Arnold de out -> in
  printf("[3] Aplicando Arnold Cat Map...\n");
  Pixel *temp = malloc(tam * sizeof(Pixel));
  difusao_arnold(in.pixels, temp, in.width, in.height);

  // Copia os resultados de volta para in
  Pixel *p_pixel_src = temp;
  Pixel *p_pixel_dst = in.pixels;
  for(int i=0; i<tam; i++){
    *p_pixel_dst = *p_pixel_src;
    p_pixel_src++;
    p_pixel_dst++;
  }
  free(temp);

  // Salvar a imagem bagunçada
  stbi_write_png("baguncada.png", in.width, in.height, 3, in.pixels, in.width * 3);
  printf("-> Imagem bagunçada salva em 'baguncada.png'\n");

  // =====================================================================
  // Desbagunçar (ordem inversa: 2⁻¹, 1⁻¹)
  // =====================================================================
  printf("\n=== DESBAGUNÇANDO ===\n");

  // Copia a imagem bagunçada pra out (usando ponteiros)
  p_src = (unsigned char *)in.pixels;
  p_dst = (unsigned char *)out.pixels;
  for(int i = 0; i<total_bytes; i++){
    *p_dst = *p_src;
    p_src++;
    p_dst++;
  }

  // Passo 3⁻¹: Difusão inversa - Camada 3 (Arnold Cat Map inverso)
  printf("[3⁻¹] Desfazendo Arnold Cat Map...\n");
  temp = malloc(tam * sizeof(Pixel));
  difusao_arnold_inversa(out.pixels, temp, out.width, out.height);

  p_pixel_src = temp;
  p_pixel_dst = out.pixels;
  for(int i=0; i<tam; i++){
    *p_pixel_dst = *p_pixel_src;
    p_pixel_src++;
    p_pixel_dst++;
  }
  free(temp);
  
  unsigned char *p_out = (unsigned char *)out.pixels;
  
  // Passo 2⁻¹: Confusão inversa - Camada 2 (XOR por posição)
  // XOR é auto-inverso, então aplicamos a mesma operação
  printf("[2⁻¹] Desfazendo XOR por posição...\n");
  confusao_xor(p_out, total_bytes);

  // Passo 1⁻¹: Confusão inversa - Camada 1 (Rotação de bits inversa)
  printf("[1⁻¹] Desfazendo rotação de bits...\n");
  confusao_rotacao_inversa(p_out, total_bytes);

  // Salvar a imagem recuperada
  stbi_write_png("recuperada.png", out.width, out.height, 3, out.pixels, out.width * 3);
  printf("-> Imagem recuperada salva em 'recuperada.png'\n");

  // =====================================================================
  // Verificação de Recuperação
  // =====================================================================
  printf("\n=== VERIFICAÇÃO ===\n");
  p_out = (unsigned char *)out.pixels;
  int erros = verificar_recuperacao(copia_original, p_out, total_bytes);

  if(erros == 0){
    printf("SUCESSO! Recuperação 100%% exata. 0 bytes diferentes. \n");
    } else {
    printf("FALHA! %d bytes diferentes de %d totais.\n", erros, total_bytes);
    }

    // Libera memória
    free(copia_original);
    free(in.pixels);
    free(out.pixels);
    
    return 0;
  }


// =====================================================================
// CONFUSÃO - CAMADA 1: ROTAÇÃO DE BITS
// =====================================================================

// Rotaciona os bits de um byte N posições pra esquerda
unsigned char rotacionar_esquerda(unsigned char byte, int n){
  n = n % 8;
  return (byte << n | (byte >> (8-n)));
}

// Rotaciona os bits de um byte N posições pra direita (inversa)
unsigned char rotacionar_direita(unsigned char byte, int n){
  n = n % 8;
  return (byte >> n | (byte << (8-n)));
}

// Aplica rotação de bits em todos os bytes da imagem
// A quantidade de rotação varia com a posição (REGRA AUTORAL)
void confusao_rotacao(unsigned char *dados, int total_bytes){
  unsigned char *p = dados;
  for(int i=0; i<total_bytes; i++){
    int n = (i%7)+1; // rotação de 1 a 7, dependendo da posição
    *p = rotacionar_esquerda(*p, n);
    p++;
  }
}

void confusao_rotacao_inversa(unsigned char *dados, int total_bytes){
  unsigned char *p = dados;
  for(int i=0; i<total_bytes; i++){
    int n = (i%7)+1; // rotação de 1 a 7, dependendo da posição
    *p = rotacionar_direita(*p, n);
    p++;
  }
}

// =====================================================================
// CONFUSÃO - CAMADA 2: XOR POR POSIÇÃO
// =====================================================================

// Aplicamos XOR em cada byte usando uma chave que depende da posição
// A chave é gerada pela fórmula: (i * 31 + 17) % 256
// E como dito anteriormente, XOR é auto-inverso, então usamos a mesma 
// funçar para bagunçar e desbagunçar
void confusao_xor(unsigned char *dados, int total_bytes){
  unsigned char *p = dados;
  for(int i=0; i<total_bytes; i++){
    unsigned char chave = (unsigned char)((i * 31 + 17) % 256);
    *p = *p ^ chave;
    p++;
  }
}

// =====================================================================
// DIFUSÃO - CAMADA 3: ARNOLD CAT MAP
// =====================================================================

void difusao_arnold(Pixel *entrada, Pixel *saida, int width, int height){
  int tam = width * height;

  Pixel *temp = malloc(tam * sizeof(Pixel));

  // Passo 1: cisalhamento horizontal
  for(int y=0; y<height; y++){
    for(int x=0; x<width; x++){
      int novo_x = (x+y*2) % width;

      Pixel *p_in = entrada + (y * width + x);
      Pixel *p_out = temp + (y * width + novo_x);

      *p_out = *p_in;
    }
  }
  
  // Passo 2: cisalhamento vertical
  for(int y=0; y<height; y++){
    for(int x=0; x<width; x++){
      int novo_y = (y+x*3) % height;

      Pixel *p_in = temp + (y * width + x);
      Pixel *p_out = saida + (novo_y * width + x);

      *p_out = *p_in;
    }
  }

  free(temp);

}

// Inversa
void difusao_arnold_inversa(Pixel *entrada, Pixel *saida, int width, int height){
  int tam = width * height;
  Pixel *temp = malloc(tam * sizeof(Pixel));

  for(int y=0; y<height; y++){
    for(int x=0; x<width; x++){
      int orig_y = ((y - x * 3) % height + height)%height; 

      Pixel *p_in = entrada + (y * width + x);
      Pixel *p_out = temp + (orig_y * width + x);

      *p_out = *p_in;
    }
  }  

  for(int y=0; y<height; y++){
    for(int x=0; x<width; x++){
      int orig_x = ((x - y * 2) % width + width)%width; 

      Pixel *p_in = temp + (y * width + x); 
      Pixel *p_out = saida + (y * width + orig_x);

      *p_out = *p_in;
    }
  }  

  free(temp);
}

// =====================================================================
// VERIFICAÇÃO
// =====================================================================

int verificar_recuperacao(unsigned char *original, unsigned char *recuperada, int total_bytes){
  int erros = 0;
  unsigned char *po = original;
  unsigned char *pr = recuperada;

  for(int i=0; i<total_bytes; i++){
    if(*po != *pr){
      erros++;
      if(erros <= 10){
        printf("Erro no byte %d: original=%d, recuperado=%d\n", i, *po, *pr);
      }
    }
    po++;
    pr++; 
  }
  return erros;
}

// =====================================================================
// CARREGAMENTO DA IMAGEM
// =====================================================================

void load(char* name, Img* pic) {
  pic->pixels =
      (Pixel*)stbi_load(name, &pic->width, &pic->height, &pic->channels, 0);
  if (!pic->pixels) {
    printf("Erro de leitura: %s\n", stbi_failure_reason());
    exit(1);
  }
  printf("Load: %d x %d x %d\n", pic->width, pic->height, pic->channels);
}
