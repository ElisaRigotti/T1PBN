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

