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
 
### Ordem das camadas
 
Na hora de bagunçar, aplicamos Camada 1 primeiro e Camada 2 depois. Na hora de desbagunçar, a ordem é **inversa** — Camada 2 primeiro e Camada 1 depois:
 
```
Bagunçar:      original → [1] rotação → [2] XOR → bagunçada
Desbagunçar:   bagunçada → [2⁻¹] XOR → [1⁻¹] rotação inversa → recuperada
```

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

