# Analizador Léxico SQL (C++)

Primera parte de la Fase 2 del proyecto de servidor de base de datos (curso Estructuras de Datos Avanzadas, Laboratorio 4). Convierte una sentencia `SELECT` en una secuencia ordenada de tokens, lista para el analizador sintáctico. No ejecuta consultas ni valida que existan tablas o columnas.

## Compilar, ejecutar y probar

Requiere `g++` con soporte C++17 y `make`.

```bash
make              # compila el ejecutable ./lexer
make test         # compila y corre las pruebas (./test_lexer)
make clean        # borra los binarios

./lexer consulta.sql                      # lee desde un archivo
echo "SELECT a FROM t WHERE x >= 1.5;" | ./lexer   # lee desde la entrada estándar
```

Salida de ejemplo:

```
TIPO            LEXEMA              POSICIÓN
SELECT_KW       "SELECT"            1:1
IDENTIFIER      "a"                 1:8
FROM_KW         "FROM"              1:10
...
END_OF_FILE     ""                  1:31
```

La posición es `línea:columna`, ambas empezando en 1. Los errores léxicos se listan además al final.

## Estructura

| Archivo | Responsabilidad |
|---|---|
| [src/token.h](src/token.h) | Modelo de tokens: `TokenType`, `Token` (tipo, lexema, línea, columna, mensaje) |
| [src/keywords.h](src/keywords.h) | Tabla de palabras reservadas |
| [src/lexer.h](src/lexer.h), [src/lexer.cpp](src/lexer.cpp) | Reconocedor léxico (clase `Lexer`) |
| [src/main.cpp](src/main.cpp) | Lectura de entrada e interfaz de salida |
| [tests/test_lexer.cpp](tests/test_lexer.cpp) | Pruebas unitarias y generación del reporte |

### Uso desde otro módulo (p. ej. el parser)

```cpp
#include "lexer.h"
Lexer lexer(sql);
std::vector<Token> tokens = lexer.tokenize();   // el último siempre es END_OF_FILE
const auto& errores = lexer.getErrors();        // mensajes legibles
```

## Especificación de tokens

| Categoría | Token(s) | Lexema |
|---|---|---|
| Palabras reservadas | `SELECT_KW`, `FROM_KW`, `WHERE_KW`, `AND_KW`, `OR_KW` | SELECT, FROM, WHERE, AND, OR |
| Identificador | `IDENTIFIER` | `[A-Za-z_][A-Za-z0-9_]*` |
| Entero | `INTEGER` | `[0-9]+` |
| Decimal | `DECIMAL` | `[0-9]+\.[0-9]+` |
| Cadena | `STRING` | `'...'` (el lexema incluye las comillas) |
| Relacionales | `EQUAL`, `NOT_EQUAL`, `LESS`, `GREATER`, `LESS_EQUAL`, `GREATER_EQUAL` | `=`, `<>` y `!=`, `<`, `>`, `<=`, `>=` |
| Puntuación | `COMMA`, `SEMICOLON`, `STAR` | `,` `;` `*` |
| Fin de entrada | `END_OF_FILE` | vacío |
| Error léxico | `ERROR` | texto problemático, con mensaje en `Token::message` |

## Reglas léxicas

- **Mayúsculas/minúsculas:** las palabras reservadas no distinguen mayúsculas (`select` = `SELECT`). El lexema conserva el texto original. Los identificadores conservan su capitalización.
- **Identificadores:** empiezan con letra o `_`, y continúan con letras, dígitos o `_`. Una palabra reservada solo se reconoce si coincide completa (`selected` es identificador).
- **Espacios en blanco:** espacios, tabulaciones y saltos de línea (`\n`, `\r`) se ignoran. Solo sirven para separar tokens y para calcular posiciones.
- **Operadores:** se reconoce siempre el más largo posible (`>=` es un token, no `>` y `=`). `<>` y `!=` producen el mismo tipo, `NOT_EQUAL`.
- **Números:** un decimal exige dígitos a ambos lados del punto (`1.5`). `1.` es un error léxico.
- **Cadenas:** delimitadas por comillas simples, sin saltos de línea dentro. No hay secuencias de escape.
- **Posición:** línea y columna del primer carácter del token. El token `END_OF_FILE` marca la posición final.

## Manejo de errores

Un error nunca se clasifica como token válido. Se emite un token `ERROR` en el punto donde ocurrió (para que el parser sepa dónde), y se guarda un mensaje `Error léxico en línea L, columna C: ...`. El análisis continúa después del error.

| Entrada | Resultado |
|---|---|
| `@`, `#`, `$`, etc. | `carácter no válido` |
| `!` sin `=` | `carácter no válido '!'` |
| `'abc` (sin cerrar, hasta fin de línea o de entrada) | `cadena sin cerrar` |
| `1.` | `número decimal mal formado` |
| `.` suelto (p. ej. `t.col`) | `carácter no válido '.'` |

## Decisiones y limitaciones documentadas

- No se admite la notación `tabla.columna` (el `.` solo es válido dentro de un decimal).
- `12abc` se separa en `INTEGER(12)` e `IDENTIFIER(abc)`. Es el parser quien rechazará esa secuencia.
- No se admiten comentarios SQL, comillas dobles ni secuencias de escape en cadenas.
- Se agregó el token `STAR` (`*`) para soportar `SELECT *`.

## Pruebas

```bash
make test
```

Compila y ejecuta 46 comprobaciones agrupadas en los 7 escenarios del laboratorio: sentencia válida, operadores compuestos, literales, palabras reservadas vs. identificadores, espacios en blanco, errores léxicos y casos límite (entrada vacía, sentencias incompletas, tokens al final de la entrada).

En consola muestra cada prueba como `[OK]` o `[FALLO]` y un resumen. Además **genera el reporte [docs/Reporte_Pruebas.md](docs/Reporte_Pruebas.md)**, con una tabla por escenario que indica, para cada prueba, la consulta de entrada, los tokens encontrados (`TIPO(lexema)@línea:columna`), los errores léxicos reportados y el resultado. Se regenera en cada ejecución y sirve como evidencia de lo que se probó.
