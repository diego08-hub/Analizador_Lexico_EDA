# Analizador Léxico y Sintáctico Predictivo LL(1) SQL (C++)

Fase 2 del proyecto de servidor de base de datos (curso Estructuras de Datos Avanzadas):
- **Laboratorio 4:** Analizador Léxico (scanner de tokens).
- **Laboratorio 5:** Analizador Sintáctico Predictivo No Recursivo LL(1) basado en pila.

## Integrantes

- Diego Canqui Ecos
- Diego Mauricio Villanueva Flores

## Compilar, ejecutar y probar

Requiere `g++` con soporte C++17 y `make`.

```bash
make all          # compila ./lexer, ./parser_cli, ./test_lexer y ./test_parser
make test         # compila y corre ambas suites de pruebas (46 léxicas + 30 sintácticas)
make clean        # borra los binarios generados
```

### Ejecutar Analizador Léxico (Práctica 4)
```bash
./lexer consulta.sql                              # lee desde un archivo
echo "SELECT a FROM t WHERE x >= 1.5;" | ./lexer  # lee desde la entrada estándar
```

### Ejecutar Analizador Sintáctico LL(1) (Práctica 5)
```bash
./parser_cli consulta.sql       # analiza y muestra la traza de la pila paso a paso
./parser_cli                    # inicia modo interactivo por consola (SQL> ...)
./parser_cli --table            # visualiza la lista de producciones y reglas registradas
```

## Estructura del Proyecto

| Archivo / Carpeta | Responsabilidad |
|---|---|
| [src/token.h](src/token.h) | Modelo de tokens: `TokenType`, `Token` (tipo, lexema, línea, columna, mensaje) |
| [src/keywords.h](src/keywords.h) | Tabla hash de palabras reservadas |
| [src/lexer.h](src/lexer.h), [src/lexer.cpp](src/lexer.cpp) | Reconocedor léxico (clase `Lexer`) |
| [src/main.cpp](src/main.cpp) | Punto de entrada del analizador léxico |
| [parser_ll1/types.h](parser_ll1/types.h) | Símbolos terminales/no terminales, producciones y errores |
| [parser_ll1/parse_table.h](parser_ll1/parse_table.h), [parse_table.cpp](parser_ll1/parse_table.cpp) | Matriz de Parseo LL(1) $M[A, a]$ y reglas gramaticales |
| [parser_ll1/tracer.h](parser_ll1/tracer.h) | Trazador formateador de la pila, entrada restante y acciones |
| [parser_ll1/parser.h](parser_ll1/parser.h), [parser.cpp](parser_ll1/parser.cpp) | Motor LL(1) iterativo basado en pila (`std::vector<Symbol>`) |
| [parser_ll1/main.cpp](parser_ll1/main.cpp) | CLI interactivo y analizador de archivos para el parser |
| [tests/test_lexer.cpp](tests/test_lexer.cpp) | Batería de 46 pruebas léxicas y generación de reporte |
| [tests/test_parser.cpp](tests/test_parser.cpp) | Batería de 30 pruebas sintácticas (válidas, complejas e inválidas) |
| [docs/Reporte_Pruebas.md](docs/Reporte_Pruebas.md) | Evidencia de ejecución de pruebas léxicas |
| [docs/Reporte_Pruebas_Parser.md](docs/Reporte_Pruebas_Parser.md) | Evidencia y traza paso a paso de pruebas sintácticas |

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
