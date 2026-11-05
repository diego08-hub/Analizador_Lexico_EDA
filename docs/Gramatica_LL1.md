# Gramática LL(1) del subconjunto SQL

La gramática parte de la propuesta del equipo y nombra los terminales con los
valores de `TokenType` del lexer. Una producción con lado derecho vacío
representa epsilon (`ε`).

```text
CONSULTA          -> SELECT_KW COLUMNA FROM_KW IDENTIFIER CONDICION SEMICOLON
COLUMNA           -> STAR | LISTA_ID
LISTA_ID          -> IDENTIFIER LISTA_ID_PRIMA
LISTA_ID_PRIMA    -> COMMA LISTA_ID | ε
CONDICION         -> WHERE_KW EXPR_OR | ε
EXPR_OR           -> EXPR_AND EXPR_OR_PRIMA
EXPR_OR_PRIMA     -> OR_KW EXPR_AND EXPR_OR_PRIMA | ε
EXPR_AND          -> PREDICADO EXPR_AND_PRIMA
EXPR_AND_PRIMA    -> AND_KW PREDICADO EXPR_AND_PRIMA | ε
PREDICADO         -> VALOR OP_COMP VALOR
VALOR             -> IDENTIFIER | STRING | EXPR_NUM
OP_COMP           -> EQUAL | NOT_EQUAL | LESS | GREATER | LESS_EQUAL | GREATER_EQUAL
EXPR_NUM           -> TERM EXPR_NUM_PRIMA
EXPR_NUM_PRIMA     -> PLUS TERM EXPR_NUM_PRIMA | MINUS TERM EXPR_NUM_PRIMA | ε
TERM               -> FACTOR TERM_PRIMA
TERM_PRIMA         -> STAR FACTOR TERM_PRIMA | SLASH FACTOR TERM_PRIMA | ε
FACTOR             -> LPAREN EXPR_NUM RPAREN | INTEGER | DECIMAL
```

La factorización de la lista de identificadores evita la alternativa original
`ID -> id | id, ID`, que comparte prefijo. Las colas `EXPR_OR_PRIMA`,
`EXPR_AND_PRIMA`, `EXPR_NUM_PRIMA` y `TERM_PRIMA` eliminan la recursión
izquierda de las expresiones. La separación entre `EXPR_OR` y `EXPR_AND`
establece que `AND` tiene mayor precedencia que `OR`; en aritmética, `*` y `/`
preceden a `+` y `-`.

Los operadores de comparación están representados por los tokens del lexer:
`<>` y `!=` llegan ambos como `NOT_EQUAL`. Los números se representan como
`INTEGER` o `DECIMAL`, ya que el lexer no produce un único token `NUM`.

## Conjuntos FIRST

| No terminal | FIRST |
|---|---|
| `CONSULTA` | `{SELECT_KW}` |
| `COLUMNA` | `{STAR, IDENTIFIER}` |
| `LISTA_ID` | `{IDENTIFIER}` |
| `LISTA_ID_PRIMA` | `{COMMA, ε}` |
| `CONDICION` | `{WHERE_KW, ε}` |
| `EXPR_OR` | `{IDENTIFIER, STRING, LPAREN, INTEGER, DECIMAL}` |
| `EXPR_OR_PRIMA` | `{OR_KW, ε}` |
| `EXPR_AND` | `{IDENTIFIER, STRING, LPAREN, INTEGER, DECIMAL}` |
| `EXPR_AND_PRIMA` | `{AND_KW, ε}` |
| `PREDICADO` | `{IDENTIFIER, STRING, LPAREN, INTEGER, DECIMAL}` |
| `VALOR` | `{IDENTIFIER, STRING, LPAREN, INTEGER, DECIMAL}` |
| `OP_COMP` | `{EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL}` |
| `EXPR_NUM` | `{LPAREN, INTEGER, DECIMAL}` |
| `EXPR_NUM_PRIMA` | `{PLUS, MINUS, ε}` |
| `TERM` | `{LPAREN, INTEGER, DECIMAL}` |
| `TERM_PRIMA` | `{STAR, SLASH, ε}` |
| `FACTOR` | `{LPAREN, INTEGER, DECIMAL}` |

## Conjuntos FOLLOW

`$` representa el fin de la entrada, equivalente al token `END_OF_FILE`.

| No terminal | FOLLOW |
|---|---|
| `CONSULTA` | `{$}` |
| `COLUMNA` | `{FROM_KW}` |
| `LISTA_ID` | `{FROM_KW}` |
| `LISTA_ID_PRIMA` | `{FROM_KW}` |
| `CONDICION` | `{SEMICOLON}` |
| `EXPR_OR` | `{SEMICOLON}` |
| `EXPR_OR_PRIMA` | `{SEMICOLON}` |
| `EXPR_AND` | `{OR_KW, SEMICOLON}` |
| `EXPR_AND_PRIMA` | `{OR_KW, SEMICOLON}` |
| `PREDICADO` | `{AND_KW, OR_KW, SEMICOLON}` |
| `VALOR` | `{EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, AND_KW, OR_KW, SEMICOLON}` |
| `OP_COMP` | `{IDENTIFIER, STRING, LPAREN, INTEGER, DECIMAL}` |
| `EXPR_NUM` | `{EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, AND_KW, OR_KW, SEMICOLON, RPAREN}` |
| `EXPR_NUM_PRIMA` | `{EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, AND_KW, OR_KW, SEMICOLON, RPAREN}` |
| `TERM` | `{PLUS, MINUS, EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, AND_KW, OR_KW, SEMICOLON, RPAREN}` |
| `TERM_PRIMA` | `{PLUS, MINUS, EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, AND_KW, OR_KW, SEMICOLON, RPAREN}` |
| `FACTOR` | `{STAR, SLASH, PLUS, MINUS, EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, AND_KW, OR_KW, SEMICOLON, RPAREN}` |

Cada `ε` en FIRST habilita la alternativa vacía; en esos casos se comprueba
también la intersección con FOLLOW para construir la tabla LL(1). El algoritmo
de `parser_ll1/grammar.cpp` calcula estos conjuntos por punto fijo, en lugar de
mantenerlos duplicados como valores escritos a mano.

## Comprobación LL(1)

Las alternativas de `COLUMNA`, `VALOR` y `FACTOR` tienen FIRST disjuntos.
Para las producciones con alternativa vacía, el terminal de continuación no
coincide con FIRST de la alternativa no vacía:

| No terminal | FIRST de alternativa no vacía | FOLLOW | Intersección |
|---|---|---|---|
| `LISTA_ID_PRIMA` | `{COMMA}` | `{FROM_KW}` | `∅` |
| `CONDICION` | `{WHERE_KW}` | `{SEMICOLON}` | `∅` |
| `EXPR_OR_PRIMA` | `{OR_KW}` | `{SEMICOLON}` | `∅` |
| `EXPR_AND_PRIMA` | `{AND_KW}` | `{OR_KW, SEMICOLON}` | `∅` |
| `EXPR_NUM_PRIMA` | `{PLUS, MINUS}` | `{EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, AND_KW, OR_KW, SEMICOLON, RPAREN}` | `∅` |
| `TERM_PRIMA` | `{STAR, SLASH}` | `{PLUS, MINUS, EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, AND_KW, OR_KW, SEMICOLON, RPAREN}` | `∅` |

Así, las alternativas de esta gramática no generan conflictos LL(1).

## Alcance actual

- La proyección admite `*` o una lista separada por comas.
- La tabla es un `IDENTIFIER`.
- `WHERE` es opcional; sin esa cláusula, la consulta termina después de la tabla.
- Los operandos aceptan identificadores, cadenas o expresiones aritméticas.
- Las expresiones aritméticas admiten paréntesis, `+`, `-`, `*` y `/`.
- La gramática no incluye alias, nombres calificados (`tabla.columna`), `NULL`,
  operadores unarios ni paréntesis para agrupar condiciones lógicas.

El módulo `parser_ll1/grammar.cpp` calcula FIRST y FOLLOW por punto fijo a partir de
estas producciones. Las pruebas comparan conjuntos completos para no depender
del orden de impresión.
