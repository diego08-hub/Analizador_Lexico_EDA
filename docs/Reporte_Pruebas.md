# Reporte de pruebas del analizador lexico

Generado automaticamente por `make test`.

**Resultado:** 46 pruebas OK, 0 fallidas

Formato de token: `TIPO(lexema)@linea:columna`

## 1. Sentencia valida

| Prueba | Consulta | Tokens encontrados | Errores reportados | Resultado |
|---|---|---|---|---|
| select completo | `SELECT a, b FROM t WHERE x >= 10 AND y = 'hi' OR z < 2.5;` | `SELECT_KW(SELECT)@1:1  IDENTIFIER(a)@1:8  COMMA(,)@1:9  IDENTIFIER(b)@1:11  FROM_KW(FROM)@1:13  IDENTIFIER(t)@1:18  WHERE_KW(WHERE)@1:20  IDENTIFIER(x)@1:26  GREATER_EQUAL(>=)@1:28  INTEGER(10)@1:31  AND_KW(AND)@1:34  IDENTIFIER(y)@1:38  EQUAL(=)@1:40  STRING('hi')@1:42  OR_KW(OR)@1:47  IDENTIFIER(z)@1:50  LESS(<)@1:52  DECIMAL(2.5)@1:54  SEMICOLON(;)@1:57  EOF` |  | OK |
| select * | `SELECT * FROM t` | `SELECT_KW(SELECT)@1:1  STAR(*)@1:8  FROM_KW(FROM)@1:10  IDENTIFIER(t)@1:15  EOF` |  | OK |

## 2. Operadores compuestos

| Prueba | Consulta | Tokens encontrados | Errores reportados | Resultado |
|---|---|---|---|---|
| >= | `a>=b` | `IDENTIFIER(a)@1:1  GREATER_EQUAL(>=)@1:2  IDENTIFIER(b)@1:4  EOF` |  | OK |
| <= | `a<=b` | `IDENTIFIER(a)@1:1  LESS_EQUAL(<=)@1:2  IDENTIFIER(b)@1:4  EOF` |  | OK |
| <> | `a<>b` | `IDENTIFIER(a)@1:1  NOT_EQUAL(<>)@1:2  IDENTIFIER(b)@1:4  EOF` |  | OK |
| != | `a!=b` | `IDENTIFIER(a)@1:1  NOT_EQUAL(!=)@1:2  IDENTIFIER(b)@1:4  EOF` |  | OK |
| simples | `= < >` | `EQUAL(=)@1:1  LESS(<)@1:3  GREATER(>)@1:5  EOF` |  | OK |
| lexema <> | `<>` | `NOT_EQUAL(<>)@1:1  EOF` |  | OK |
| < = separados | `< =` | `LESS(<)@1:1  EQUAL(=)@1:3  EOF` |  | OK |
| >> | `>>` | `GREATER(>)@1:1  GREATER(>)@1:2  EOF` |  | OK |

## 3. Literales

| Prueba | Consulta | Tokens encontrados | Errores reportados | Resultado |
|---|---|---|---|---|
| entero | `42` | `INTEGER(42)@1:1  EOF` |  | OK |
| decimal | `3.14` | `DECIMAL(3.14)@1:1  EOF` |  | OK |
| lexema decimal | `3.14` | `DECIMAL(3.14)@1:1  EOF` |  | OK |
| cadena | `'hola mundo'` | `STRING('hola mundo')@1:1  EOF` |  | OK |
| cadena incluye comillas | `'hola'` | `STRING('hola')@1:1  EOF` |  | OK |
| cadena vacia | `''` | `STRING('')@1:1  EOF` |  | OK |
| cadena con simbolos | `'a<=b, c;'` | `STRING('a<=b, c;')@1:1  EOF` |  | OK |

## 4. Palabras reservadas e identificadores

| Prueba | Consulta | Tokens encontrados | Errores reportados | Resultado |
|---|---|---|---|---|
| keywords mayusculas | `SELECT FROM WHERE AND OR` | `SELECT_KW(SELECT)@1:1  FROM_KW(FROM)@1:8  WHERE_KW(WHERE)@1:13  AND_KW(AND)@1:19  OR_KW(OR)@1:23  EOF` |  | OK |
| keywords minusculas/mixtas | `select From wHeRe and Or` | `SELECT_KW(select)@1:1  FROM_KW(From)@1:8  WHERE_KW(wHeRe)@1:13  AND_KW(and)@1:19  OR_KW(Or)@1:23  EOF` |  | OK |
| lexema original conservado | `select` | `SELECT_KW(select)@1:1  EOF` |  | OK |
| prefijo de keyword es identificador | `selected fromage android order` | `IDENTIFIER(selected)@1:1  IDENTIFIER(fromage)@1:10  IDENTIFIER(android)@1:18  IDENTIFIER(order)@1:26  EOF` |  | OK |
| identificadores con _ y digitos | `_id col1 a_b2` | `IDENTIFIER(_id)@1:1  IDENTIFIER(col1)@1:5  IDENTIFIER(a_b2)@1:10  EOF` |  | OK |

## 5. Espacios en blanco

| Prueba | Consulta | Tokens encontrados | Errores reportados | Resultado |
|---|---|---|---|---|
| espacios, tabs y saltos | `SELECT \t a\n\n FROM\r\n t` | `SELECT_KW(SELECT)@1:1  IDENTIFIER(a)@1:10  FROM_KW(FROM)@3:2  IDENTIFIER(t)@4:2  EOF` |  | OK |
| sin espacios | `a,b;` | `IDENTIFIER(a)@1:1  COMMA(,)@1:2  IDENTIFIER(b)@1:3  SEMICOLON(;)@1:4  EOF` |  | OK |
| posicion token 1 | `SELECT a` | `SELECT_KW(SELECT)@1:1  IDENTIFIER(a)@1:8  EOF` |  | OK |
| posicion token 2 | `SELECT a` | `SELECT_KW(SELECT)@1:1  IDENTIFIER(a)@1:8  EOF` |  | OK |
| posicion tras salto de linea | `SELECT\n  a` | `SELECT_KW(SELECT)@1:1  IDENTIFIER(a)@2:3  EOF` |  | OK |
| posicion tras tab | `\ta` | `IDENTIFIER(a)@1:2  EOF` |  | OK |

## 6. Errores lexicos

| Prueba | Consulta | Tokens encontrados | Errores reportados | Resultado |
|---|---|---|---|---|
| caracter invalido | `a @ b` | `IDENTIFIER(a)@1:1  ERROR(@)@1:3  IDENTIFIER(b)@1:5  EOF` | Error léxico en línea 1, columna 3: carácter no válido '@' | OK |
| mensaje de error con posicion | `a @ b` | `IDENTIFIER(a)@1:1  ERROR(@)@1:3  IDENTIFIER(b)@1:5  EOF` | Error léxico en línea 1, columna 3: carácter no válido '@' | OK |
| cadena sin cerrar | `x = 'abc` | `IDENTIFIER(x)@1:1  EQUAL(=)@1:3  ERROR('abc)@1:5  EOF` | Error léxico en línea 1, columna 5: cadena sin cerrar | OK |
| cadena sin cerrar reporta 1 error | `x = 'abc` | `IDENTIFIER(x)@1:1  EQUAL(=)@1:3  ERROR('abc)@1:5  EOF` | Error léxico en línea 1, columna 5: cadena sin cerrar | OK |
| cadena sin cerrar no cruza lineas | `x = 'abc\ny` | `IDENTIFIER(x)@1:1  EQUAL(=)@1:3  ERROR('abc)@1:5  IDENTIFIER(y)@2:1  EOF` | Error léxico en línea 1, columna 5: cadena sin cerrar | OK |
| ! solo es error | `!` | `ERROR(!)@1:1  EOF` | Error léxico en línea 1, columna 1: carácter no válido '!' | OK |
| decimal mal formado | `1.` | `ERROR(1.)@1:1  EOF` | Error léxico en línea 1, columna 1: número decimal mal formado '1.' | OK |
| varios errores consecutivos | `# $ @` | `ERROR(#)@1:1  ERROR($)@1:3  ERROR(@)@1:5  EOF` | Error léxico en línea 1, columna 1: carácter no válido '#'<br>Error léxico en línea 1, columna 3: carácter no válido '$'<br>Error léxico en línea 1, columna 5: carácter no válido '@' | OK |
| entrada valida no genera errores | `SELECT a FROM t` | `SELECT_KW(SELECT)@1:1  IDENTIFIER(a)@1:8  FROM_KW(FROM)@1:10  IDENTIFIER(t)@1:15  EOF` |  | OK |

## 7. Casos limite

| Prueba | Consulta | Tokens encontrados | Errores reportados | Resultado |
|---|---|---|---|---|
| entrada vacia | `` | `EOF` |  | OK |
| solo espacios | `  \n\t ` | `EOF` |  | OK |
| sentencia incompleta | `SELECT` | `SELECT_KW(SELECT)@1:1  EOF` |  | OK |
| sentencia incompleta 2 | `SELECT a FROM WHERE` | `SELECT_KW(SELECT)@1:1  IDENTIFIER(a)@1:8  FROM_KW(FROM)@1:10  WHERE_KW(WHERE)@1:15  EOF` |  | OK |
| token pegado al final | `a=1` | `IDENTIFIER(a)@1:1  EQUAL(=)@1:2  INTEGER(1)@1:3  EOF` |  | OK |
| operador al final | `a<` | `IDENTIFIER(a)@1:1  LESS(<)@1:2  EOF` |  | OK |
| cadena al final | `x='a'` | `IDENTIFIER(x)@1:1  EQUAL(=)@1:2  STRING('a')@1:3  EOF` |  | OK |
| EOF posicion | `a\nb` | `IDENTIFIER(a)@1:1  IDENTIFIER(b)@2:1  EOF` |  | OK |
| tokenize repetido no duplica tokens | `SELECT a` | `SELECT_KW(SELECT)@1:1  IDENTIFIER(a)@1:8  EOF` |  | OK |
