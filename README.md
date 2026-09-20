# Bagunceitor — Confusão e Difusão em Imagens

Trabalho Prático 1 de Programação de Baixo Nível (PBN) — PUCRS, 2026/1.

Professor: Edson Moreno

Estudantes: Eduardo Alves e Elisa Rigotti

## O que é

O Bagunceitor aplica técnicas de **confusão** (alteração dos valores dos bytes) e **difusão** (alteração das posições dos pixels) para embaralhar completamente uma imagem, tornando-a irreconhecível. Em seguida, desfaz o processo na ordem inversa, recuperando a imagem original com **100% de exatidão** — sem perder um único byte.

## Como compilar e executar

```bash
cd bagunceitor
make
./bagunceitor predio32.jpg
```

O programa gera dois arquivos:
- `baguncada.png` — imagem embaralhada
- `recuperada.png` — imagem recuperada (idêntica à original)

## Técnicas escolhidas

### Confusão — Camada 1: Rotação de Bits

#### O que é rotação de bits

Rotação de bits é uma operação bitwise que desloca todos os bits de um byte em uma direção. Os bits que "caem" de um lado reentram pelo outro — nenhum bit é perdido. Isso é diferente do shift (`<<`), onde os bits que saem são descartados.

#### Exemplo — rotação de 3 posições pra esquerda

```
Byte original:      1 0 1 1 0 0 0 1    (177 em decimal)
                    ─┬─┬─┬─────────
                     │ │ │
                     ▼ ▼ ▼
Saem pela esquerda:  1 0 1

                            ─────────┬─┬─┬
                                     │ │ │
                                     ▼ ▼ ▼
Reentram pela direita:                1 0 1

Resultado:           1 0 0 0 1 1 0 1    (141 em decimal)
```

Na implementação, isso é feito com dois operadores bitwise combinados:

```c
return (byte << n) | (byte >> (8 - n));
//      ─────────    ─────────────────
//      empurra n     pega os n bits
//      posições      que "caíram" e
//      pra esquerda  coloca na direita
```

#### Regra autoral — rotação variável por posição

A rotação de bits por si só é uma técnica conhecida. O que torna a nossa implementação única é a **regra que define quantas posições rotacionar cada byte**:

```c
int n = (i % 7) + 1;
```

Onde `i` é a posição do byte na imagem. Isso cria um ciclo de 7 valores:

| Posição do byte | `i % 7` | `+ 1` | Rotação aplicada |
|-----------------|---------|-------|------------------|
| 0               | 0       | 1     | 1 posição        |
| 1               | 1       | 2     | 2 posições       |
| 2               | 2       | 3     | 3 posições       |
| 3               | 3       | 4     | 4 posições       |
| 4               | 4       | 5     | 5 posições       |
| 5               | 5       | 6     | 6 posições       |
| 6               | 6       | 7     | 7 posições       |
| 7               | 0       | 1     | 1 posição (recomeça) |

Cada byte é transformado de forma diferente dependendo da sua posição. Isso evita padrões repetitivos e torna a confusão mais eficaz do que uma rotação fixa (que seria fácil de reverter por tentativa e erro com apenas 7 possibilidades).

#### Por que escolhemos rotação de bits

- Diferente de operações simples como inversão (`255 - byte`) ou NOT (`~byte`), a rotação com deslocamento variável cria um padrão complexo que destrói completamente a informação de cor
- A implementação usa operadores bitwise (`<<`, `>>`, `|`), demonstrando manipulação em nível de bits
- É 100% reversível: rotacionar pra direita com a mesma quantidade desfaz a operação

#### Inversa

Rotaciona pra direita com a mesma regra. Como nenhum bit é perdido, a operação é perfeitamente reversível:

```
rotacionar_direita(rotacionar_esquerda(byte, n), n) == byte
```

#### Implementação com ponteiros

Percorre a imagem como um bloco contínuo de bytes usando um ponteiro `unsigned char *p` que avança byte a byte com `p++`:

```c
void confusao_rotacao(unsigned char *dados, int total_bytes) {
  unsigned char *p = dados;
  for (int i = 0; i < total_bytes; i++) {
    int n = (i % 7) + 1;
    *p = rotacionar_esquerda(*p, n);
    p++;
  }
}
```

O cast `(unsigned char *)in.pixels` permite tratar o array de `Pixel` (structs de 3 bytes) como uma sequência de bytes individuais — a confusão trabalha byte a byte, não pixel a pixel.

### Confusão — Camada 2: XOR por Posição
 
#### O que é XOR
 
XOR (exclusive or) é uma operação bitwise que compara dois bits e retorna 1 apenas quando eles são **diferentes**:
 
```
A   B   A ^ B
0   0     0
0   1     1
1   0     1
1   1     0
```
 
Quando aplicamos XOR entre um byte e uma "chave", cada bit do byte é invertido (ou não) dependendo do bit correspondente da chave.
 
#### Exemplo — XOR entre um byte e uma chave
 
```
Byte original:   1 0 1 1 0 0 0 1    (177)
Chave:           0 1 1 0 1 0 1 0    (106)
                 ─ ─ ─ ─ ─ ─ ─ ─
Resultado XOR:   1 1 0 1 1 0 1 1    (219)
                 │ │ │ │ │ │ │ │
                 = ≠ ≠ = ≠ = ≠ =
                 ↑
          onde chave=0, bit fica igual
          onde chave=1, bit é invertido
```
#### Propriedade fundamental: XOR é auto-inverso
 
A grande vantagem do XOR é que ele **desfaz a si mesmo**. Aplicar XOR com a mesma chave duas vezes recupera o valor original:
 
```
byte ^ chave ^ chave == byte
```
 
Demonstrando com o exemplo acima:
 
```
Original:        1 0 1 1 0 0 0 1    (177)
XOR com chave:   1 1 0 1 1 0 1 1    (219)   ← bagunçado
XOR com chave:   1 0 1 1 0 0 0 1    (177)   ← voltou ao original!
```
 
Isso significa que **a mesma função** serve tanto pra bagunçar quanto pra desbagunçar, ou seja, não precisa de uma função inversa separada.
 
#### Regra autoral — chave variável por posição
 
O XOR por si só é uma técnica conhecida. O que torna a nossa implementação única é a **fórmula que gera uma chave diferente para cada byte**:
 
```c
unsigned char chave = (unsigned char)((i * 31 + 17) % 256);
```
 
Onde `i` é a posição do byte na imagem. Essa fórmula gera uma sequência pseudo-aleatória de chaves:
 
| Posição do byte | `i * 31 + 17` | `% 256` | Chave gerada |
|-----------------|---------------|---------|--------------|
| 0               | 17            | 17      | 17           |
| 1               | 48            | 48      | 48           |
| 2               | 79            | 79      | 79           |
| 3               | 110           | 110     | 110          |
| 4               | 141           | 141     | 141          |
| 5               | 172           | 172     | 172          |
| 6               | 203           | 203     | 203          |
| 7               | 234           | 234     | 234          |
| 8               | 265           | 9       | 9            |
 
Os números 31 e 17 foram escolhidos porque:
- **31 é primo** — multiplicar por um primo garante que a sequência demora o máximo possível pra se repetir (ciclo de 256 valores antes de recomeçar)
- **17 é o deslocamento (offset)** — evita que a posição 0 tenha chave 0 (que não alteraria o byte, já que `byte ^ 0 == byte`)
#### Por que escolhemos XOR
 
- O XOR combinado com a rotação de bits (Camada 1) cria uma confusão de **duas camadas**: primeiro os bits são reorganizados dentro de cada byte, depois cada byte é misturado com uma chave única
- A implementação usa o operador bitwise `^`, demonstrando mais uma operação em nível de bits
- A auto-inversibilidade (`a ^ b ^ b == a`) garante recuperação 100% exata
#### Inversa
 
Como XOR é auto-inverso, a mesma função `confusao_xor()` serve para aplicar e desfazer. Não precisamos de uma função inversa separada.
 
#### Implementação com ponteiros
 
Percorre a imagem byte a byte usando um ponteiro `unsigned char *p` com `p++`, da mesma forma que a Camada 1:
 
```c
void confusao_xor(unsigned char *dados, int total_bytes) {
  unsigned char *p = dados;
  for (int i = 0; i < total_bytes; i++) {
    unsigned char chave = (unsigned char)((i * 31 + 17) % 256);
    *p = *p ^ chave;
    p++;
  }
}
```
 
### Difusão — Camada 3: Arnold Cat Map
 
As Camadas 1 e 2 são técnicas de **confusão**: alteram os *valores* dos bytes, mas cada pixel continua na mesma posição. Isso significa que, mesmo com os valores completamente alterados, a **silhueta** da imagem original ainda pode ser visível — áreas claras continuam claras e áreas escuras continuam escuras, só que com cores erradas.
 
A Camada 3 é uma técnica de **difusão**: ela **embaralha as posições** dos pixels, distribuindo-os por toda a imagem. Combinada com a confusão, a imagem se torna completamente irreconhecível.
 
#### O que é o Arnold Cat Map
 
O Arnold Cat Map é uma transformação matemática criada pelo matemático Vladimir Arnold em 1968. Originalmente aplicada a imagens quadradas (NxN), ela rearranja os pixels de forma determinística e reversível — como embaralhar um baralho sempre na mesma ordem.
 
O nome "Cat Map" vem do fato de Arnold ter demonstrado a transformação usando a imagem de um gato.
 
#### O problema: imagens retangulares
 
O Arnold Cat Map clássico usa a fórmula:
 
```
novo_x = (x + y) mod N
novo_y = (x + 2y) mod N
```
 
Essa fórmula só funciona para imagens **quadradas** (NxN), porque usa o mesmo módulo N para ambas as coordenadas. Em uma imagem retangular (como 1280×960), aplicar a fórmula diretamente causa **colisões** — dois pixels diferentes podem ser mapeados para a mesma posição, destruindo informação.
 
#### Solução: decomposição por cisalhamento (shearing)
 
Para funcionar com qualquer tamanho, decompomos a transformação em **dois cisalhamentos separados**:
 
```
Passo A — cisalhamento horizontal: novo_x = (x + y * 2) % width
Passo B — cisalhamento vertical:   novo_y = (y + x * 3) % height
```
 
Cada cisalhamento opera em uma dimensão de cada vez:
- O Passo A desloca pixels **horizontalmente** dentro de cada linha — usa apenas `% width`
- O Passo B desloca pixels **verticalmente** dentro de cada coluna — usa apenas `% height`
Como cada passo desloca pixels apenas dentro da sua dimensão, **não há colisões**. Dois pixels na mesma linha com `x` diferentes produzem `novo_x` diferentes (porque a soma com `y*2` é diferente para cada `x`, e o módulo por `width` preserva a bijeção dentro da linha). O mesmo vale para o cisalhamento vertical.
 
#### Exemplo — cisalhamento horizontal (Passo A)
 
Considerando uma imagem 5×4. O Passo A aplica `novo_x = (x + y * 2) % 5`:
 
```
Linha y=0: cada pixel se desloca 0*2 = 0 posições (fica no lugar)
Linha y=1: cada pixel se desloca 1*2 = 2 posições pra direita
Linha y=2: cada pixel se desloca 2*2 = 4 posições pra direita
Linha y=3: cada pixel se desloca 3*2 = 6 ≡ 1 posição pra direita (mod 5)
 
Antes:          Depois:
A B C D E       A B C D E       (y=0: sem deslocamento)
F G H I J       I J F G H       (y=1: +2 posições, circular)
K L M N O       O K L M N       (y=2: +4 posições, circular)
P Q R S T       T P Q R S       (y=3: +1 posição, circular)
```
 
Cada linha é deslocada circularmente por uma quantidade diferente. Nenhum pixel é perdido — apenas reposicionado.
 
#### Regra autoral — coeficientes de cisalhamento
 
A decomposição por cisalhamento é uma técnica conhecida. O que torna a nossa implementação única são os **coeficientes escolhidos** para cada passo:
 
```c
// Passo A: coeficiente 2
int novo_x = (x + y * 2) % width;
 
// Passo B: coeficiente 3
int novo_y = (y + x * 3) % height;
```
 
Os coeficientes **2** e **3** controlam a intensidade do embaralhamento:
- Valores maiores causam deslocamentos mais agressivos, espalhando os pixels mais longe das suas posições originais
- Valores diferentes para cada passo (2 ≠ 3) evitam padrões simétricos no embaralhamento
- Ambos são coprimos com as dimensões típicas de imagem, maximizando a distribuição
#### Por que escolhemos o Arnold Cat Map
 
- É uma técnica clássica de criptografia de imagens, usada em artigos acadêmicos de segurança
- A decomposição por cisalhamento demonstra manipulação de coordenadas 2D com aritmética modular
- Combina perfeitamente com a confusão: as Camadas 1 e 2 destroem os valores, a Camada 3 destrói as posições
- É 100% reversível: os cisalhamentos inversos recuperam cada pixel exatamente
#### Inversa
 
Desfaz os cisalhamentos na **ordem inversa**. Se bagunçar foi A→B, desbagunçar é B⁻¹→A⁻¹:
 
```
Inversa do Passo B: orig_y = (y - x * 3) % height
Inversa do Passo A: orig_x = (x - y * 2) % width
```
 
Em C, o operador `%` pode retornar valores negativos para operandos negativos (`-7 % 5` retorna `-2` em vez de `3`). Para corrigir isso, usamos a fórmula:
 
```c
int orig_y = ((y - x * 3) % height + height) % height;
```
 
O `+ height` garante que o valor fique positivo antes do segundo `%`, produzindo o resultado matematicamente correto.
 
#### Implementação com ponteiros
 
A difusão trabalha pixel a pixel (não byte a byte como a confusão), usando ponteiros `Pixel *` com aritmética de ponteiros para acessar posições calculadas:
 
```c
void difusao_arnold(Pixel *entrada, Pixel *saida, int width, int height) {
  int tam = width * height;
  Pixel *temp = malloc(tam * sizeof(Pixel));
 
  // Passo A: cisalhamento horizontal
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int novo_x = (x + y * 2) % width;
 
      Pixel *p_in  = entrada + (y * width + x);
      Pixel *p_out = temp + (y * width + novo_x);
 
      *p_out = *p_in;
    }
  }
 
  // Passo B: cisalhamento vertical
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int novo_y = (y + x * 3) % height;
 
      Pixel *p_in  = temp + (y * width + x);
      Pixel *p_out = saida + (novo_y * width + x);
 
      *p_out = *p_in;
    }
  }
 
  free(temp);
}
```
 
A expressão `entrada + (y * width + x)` usa aritmética de ponteiros: o compilador calcula o endereço do pixel na posição (x, y) somando o deslocamento `y * width + x` ao ponteiro base. Como o ponteiro é do tipo `Pixel *`, cada unidade de deslocamento avança `sizeof(Pixel)` bytes (3 bytes), acessando diretamente o pixel correto sem precisar de casts ou indexação de array.
 
O buffer `temp` é necessário porque os dois cisalhamentos não podem ser feitos no mesmo array — o Passo A precisa terminar completamente antes que o Passo B leia seus resultados.
 
### Ordem das camadas
 
Na hora de bagunçar, aplicamos as 3 camadas em sequência. Na hora de desbagunçar, a ordem é **inversa**:
 
```
Bagunçar:      original → [1] rotação → [2] XOR → [3] Arnold → bagunçada
Desbagunçar:   bagunçada → [3⁻¹] Arnold inverso → [2⁻¹] XOR → [1⁻¹] rotação inversa → recuperada
```
 
As camadas de confusão (1 e 2) são aplicadas primeiro porque trabalham byte a byte — alterar os valores *antes* de embaralhar as posições garante que os padrões espaciais da imagem (bordas, gradientes) já estejam destruídos quando a difusão redistribui os pixels. Se a ordem fosse invertida (difusão primeiro), o Arnold Cat Map apenas moveria pixels intactos, e padrões locais ainda poderiam ser reconhecíveis.

## Estrutura do projeto

```
bagunceitor/
├── .vscode/               # Configurações do VS Code (debug, build)
├── include/
│   ├── stb_image.h        # Biblioteca para carregar imagens
│   └── stb_image_write.h  # Biblioteca para salvar imagens
├── main.c                 # Código principal com as implementações
├── Makefile               # Script de compilação multiplataforma
├── CMakeLists.txt         # Alternativa ao Makefile (CMake)
└── predio32.jpg           # Imagem de teste
```

