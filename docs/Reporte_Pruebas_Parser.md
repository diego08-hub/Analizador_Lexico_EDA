# Reporte de Pruebas Unitarias — Analizador Sintáctico LL(1) SQL

**Asignatura:** Estructuras de Datos Avanzadas  
**Práctica:** Laboratorio 5 — Analizador Sintáctico Predictivo LL(1)  
**Resultado global:** 30/30 pruebas aprobadas (100% de éxito)  

## 1. Tabla Resumen de Ejecución

| # | Categoría | Consulta SQL | Esperado | Estado | Observación / Diagnóstico |
|---|---|---|---|---|---|
| 1 | Consultas Válidas Simples | `SELECT * FROM usuarios ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 2 | Consultas Válidas Simples | `SELECT nombre FROM alumnos ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 3 | Consultas Válidas Simples | `SELECT   *    FROM   clientes    ;   ` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 4 | Proyecciones Múltiples | `SELECT id, nombre FROM personas ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 5 | Proyecciones Múltiples | `SELECT id, nombre, edad, email FROM usuarios ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 6 | Cláusula WHERE Relacional | `SELECT * FROM productos WHERE precio = 100 ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 7 | Cláusula WHERE Relacional | `SELECT * FROM users WHERE status <> 'INACTIVO' ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 8 | Cláusula WHERE Relacional | `SELECT id FROM empleados WHERE salario > 2500 ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 9 | Cláusula WHERE Relacional | `SELECT * FROM medidas WHERE peso < 12.5 ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 10 | Cláusula WHERE Relacional | `SELECT nombre FROM alumnos WHERE edad >= 18 ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 11 | Cláusula WHERE Relacional | `SELECT nombre FROM stock WHERE cantidad <= 0 ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 12 | Operadores Lógicos | `SELECT * FROM empleados WHERE edad > 21 AND salario > 1500 ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 13 | Operadores Lógicos | `SELECT * FROM roles WHERE nivel = 1 OR nivel = 2 ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 14 | Operadores Lógicos | `SELECT * FROM clientes WHERE saldo > 0 AND activo = 'SI' OR vip = 1 ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 15 | Operadores Lógicos | `SELECT * FROM tabla WHERE colA = colB ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 16 | Expresiones Aritméticas | `SELECT * FROM datos WHERE total = 10 + 20 ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 17 | Expresiones Aritméticas | `SELECT * FROM datos WHERE balance = 50 - 5 * 2 ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 18 | Expresiones Aritméticas | `SELECT * FROM datos WHERE ratio = 100.0 / 4.0 ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 19 | Expresiones Aritméticas | `SELECT * FROM datos WHERE res = ( 10 + 5 ) * 2 ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 20 | Expresiones Aritméticas | `SELECT * FROM t WHERE val >= 100 + 5 / ( 10 - 5 ) ;` | Válida | ✅ Pasó | Sentencia aceptada formalmente |
| 21 | Detección de Errores | `SELECT id usuarios ;` | Inválida | ✅ Pasó | Error sintáctico [línea 1, columna 11]: se encontró 'usuarios' (IDENTIFIER). Se esperaba uno de los siguientes: [FROM_KW, COMMA]. Detalle: No existe regla de producción en la tabla LL(1) para el símbolo no terminal 'ID_P'. |
| 22 | Detección de Errores | `SELECT * FROM usuarios` | Inválida | ✅ Pasó | Error sintáctico [línea 1, columna 23]: se encontró '' (END_OF_FILE). Se esperaba uno de los siguientes: [WHERE_KW, SEMICOLON]. Detalle: No existe regla de producción en la tabla LL(1) para el símbolo no terminal 'CONDICION'. |
| 23 | Detección de Errores | `FROM usuarios SELECT * ;` | Inválida | ✅ Pasó | Error sintáctico [línea 1, columna 1]: se encontró 'FROM' (FROM_KW). Se esperaba uno de los siguientes: [SELECT_KW]. Detalle: No existe regla de producción en la tabla LL(1) para el símbolo no terminal 'CONSULTA'. |
| 24 | Detección de Errores | `SELECT * FROM tabla WHERE ;` | Inválida | ✅ Pasó | Error sintáctico [línea 1, columna 27]: se encontró ';' (SEMICOLON). Se esperaba uno de los siguientes: [IDENTIFIER, INTEGER, DECIMAL, STRING, LPAREN]. Detalle: No existe regla de producción en la tabla LL(1) para el símbolo no terminal 'EXPR_COND'. |
| 25 | Detección de Errores | `SELECT * FROM tabla WHERE edad > ;` | Inválida | ✅ Pasó | Error sintáctico [línea 1, columna 34]: se encontró ';' (SEMICOLON). Se esperaba uno de los siguientes: [IDENTIFIER, INTEGER, DECIMAL, STRING, LPAREN]. Detalle: No existe regla de producción en la tabla LL(1) para el símbolo no terminal 'VAL'. |
| 26 | Detección de Errores | `SELECT id, nombre, FROM tabla ;` | Inválida | ✅ Pasó | Error sintáctico [línea 1, columna 20]: se encontró 'FROM' (FROM_KW). Se esperaba uno de los siguientes: [IDENTIFIER]. Detalle: No existe regla de producción en la tabla LL(1) para el símbolo no terminal 'ID'. |
| 27 | Detección de Errores | `SELECT * FROM t WHERE x = ( 10 + 5 ;` | Inválida | ✅ Pasó | Error sintáctico [línea 1, columna 36]: se encontró ';' (SEMICOLON). Se esperaba uno de los siguientes: [RPAREN]. Detalle: Fallo de emparejamiento (match) en la cima de la pila. |
| 28 | Detección de Errores | `SELECT * FROM ;` | Inválida | ✅ Pasó | Error sintáctico [línea 1, columna 15]: se encontró ';' (SEMICOLON). Se esperaba uno de los siguientes: [IDENTIFIER]. Detalle: Fallo de emparejamiento (match) en la cima de la pila. |
| 29 | Detección de Errores | `` | Inválida | ✅ Pasó | Error sintáctico [línea 1, columna 1]: se encontró '' (END_OF_FILE). Se esperaba uno de los siguientes: [SELECT_KW]. Detalle: No existe regla de producción en la tabla LL(1) para el símbolo no terminal 'CONSULTA'. |
| 30 | Detección de Errores | `SELECT FROM tabla ;` | Inválida | ✅ Pasó | Error sintáctico [línea 1, columna 8]: se encontró 'FROM' (FROM_KW). Se esperaba uno de los siguientes: [IDENTIFIER, STAR]. Detalle: No existe regla de producción en la tabla LL(1) para el símbolo no terminal 'COLUMNA'. |

## 2. Ejemplo Detallado de Traza Paso a Paso (Pila)

A continuación se muestra la traza de ejecución generada por el algoritmo predictivo basado en pila para la consulta:

```sql
SELECT nombre, edad FROM alumnos WHERE edad >= 18 + 5 ;
```

| Paso | Pila (base -> cima) | Entrada restante | Acción / Regla |
|:---:|:---|:---|:---|
| 1 | `$ CONSULTA` | `SELECT nombre , edad...` | Regla 1: CONSULTA -> SELECT_KW COLUMNA FROM_KW IDENTIFIER CONDICION SEMICOLON |
| 2 | `$ SEMICOLON CONDICION IDENTIFIER FROM_KW COLUMNA SELECT_KW` | `SELECT nombre , edad...` | Emparejar terminal 'SELECT' |
| 3 | `$ SEMICOLON CONDICION IDENTIFIER FROM_KW COLUMNA` | `nombre , edad FROM...` | Regla 3: COLUMNA -> ID |
| 4 | `$ SEMICOLON CONDICION IDENTIFIER FROM_KW ID` | `nombre , edad FROM...` | Regla 4: ID -> IDENTIFIER ID_P |
| 5 | `$ SEMICOLON CONDICION IDENTIFIER FROM_KW ID_P IDENTIFIER` | `nombre , edad FROM...` | Emparejar terminal 'nombre' |
| 6 | `$ SEMICOLON CONDICION IDENTIFIER FROM_KW ID_P` | `, edad FROM alumnos...` | Regla 5: ID_P -> COMMA ID |
| 7 | `$ SEMICOLON CONDICION IDENTIFIER FROM_KW ID COMMA` | `, edad FROM alumnos...` | Emparejar terminal ',' |
| 8 | `$ SEMICOLON CONDICION IDENTIFIER FROM_KW ID` | `edad FROM alumnos WHERE...` | Regla 4: ID -> IDENTIFIER ID_P |
| 9 | `$ SEMICOLON CONDICION IDENTIFIER FROM_KW ID_P IDENTIFIER` | `edad FROM alumnos WHERE...` | Emparejar terminal 'edad' |
| 10 | `$ SEMICOLON CONDICION IDENTIFIER FROM_KW ID_P` | `FROM alumnos WHERE edad...` | Regla 6: ID_P -> λ |
| 11 | `$ SEMICOLON CONDICION IDENTIFIER FROM_KW` | `FROM alumnos WHERE edad...` | Emparejar terminal 'FROM' |
| 12 | `$ SEMICOLON CONDICION IDENTIFIER` | `alumnos WHERE edad >=...` | Emparejar terminal 'alumnos' |
| 13 | `$ SEMICOLON CONDICION` | `WHERE edad >= 18...` | Regla 7: CONDICION -> WHERE_KW EXPR_COND |
| 14 | `$ SEMICOLON EXPR_COND WHERE_KW` | `WHERE edad >= 18...` | Emparejar terminal 'WHERE' |
| 15 | `$ SEMICOLON EXPR_COND` | `edad >= 18 +...` | Regla 9: EXPR_COND -> EXPR_AND EXPR_COND' |
| 16 | `$ SEMICOLON EXPR_COND' EXPR_AND` | `edad >= 18 +...` | Regla 12: EXPR_AND -> PRED EXPR_AND' |
| 17 | `$ SEMICOLON EXPR_COND' EXPR_AND' PRED` | `edad >= 18 +...` | Regla 15: PRED -> VAL OP_COMP VAL |
| 18 | `$ SEMICOLON EXPR_COND' EXPR_AND' VAL OP_COMP VAL` | `edad >= 18 +...` | Regla 16: VAL -> IDENTIFIER |
| 19 | `$ SEMICOLON EXPR_COND' EXPR_AND' VAL OP_COMP IDENTIFIER` | `edad >= 18 +...` | Emparejar terminal 'edad' |
| 20 | `$ SEMICOLON EXPR_COND' EXPR_AND' VAL OP_COMP` | `>= 18 + 5...` | Regla 24: OP_COMP -> GREATER_EQUAL |
| 21 | `$ SEMICOLON EXPR_COND' EXPR_AND' VAL GREATER_EQUAL` | `>= 18 + 5...` | Emparejar terminal '>=' |
| 22 | `$ SEMICOLON EXPR_COND' EXPR_AND' VAL` | `18 + 5 ;...` | Regla 18: VAL -> EXPR_NUM |
| 23 | `$ SEMICOLON EXPR_COND' EXPR_AND' EXPR_NUM` | `18 + 5 ;...` | Regla 25: EXPR_NUM -> TERM EXPR_NUM' |
| 24 | `$ SEMICOLON EXPR_COND' EXPR_AND' EXPR_NUM' TERM` | `18 + 5 ;...` | Regla 29: TERM -> FACTOR TERM' |
| 25 | `$ SEMICOLON EXPR_COND' EXPR_AND' EXPR_NUM' TERM' FACTOR` | `18 + 5 ;...` | Regla 34: FACTOR -> INTEGER |
| 26 | `$ SEMICOLON EXPR_COND' EXPR_AND' EXPR_NUM' TERM' INTEGER` | `18 + 5 ;...` | Emparejar terminal '18' |
| 27 | `$ SEMICOLON EXPR_COND' EXPR_AND' EXPR_NUM' TERM'` | `+ 5 ; $` | Regla 32: TERM' -> λ |
| 28 | `$ SEMICOLON EXPR_COND' EXPR_AND' EXPR_NUM'` | `+ 5 ; $` | Regla 26: EXPR_NUM' -> PLUS TERM EXPR_NUM' |
| 29 | `$ SEMICOLON EXPR_COND' EXPR_AND' EXPR_NUM' TERM PLUS` | `+ 5 ; $` | Emparejar terminal '+' |
| 30 | `$ SEMICOLON EXPR_COND' EXPR_AND' EXPR_NUM' TERM` | `5 ; $` | Regla 29: TERM -> FACTOR TERM' |
| 31 | `$ SEMICOLON EXPR_COND' EXPR_AND' EXPR_NUM' TERM' FACTOR` | `5 ; $` | Regla 34: FACTOR -> INTEGER |
| 32 | `$ SEMICOLON EXPR_COND' EXPR_AND' EXPR_NUM' TERM' INTEGER` | `5 ; $` | Emparejar terminal '5' |
| 33 | `$ SEMICOLON EXPR_COND' EXPR_AND' EXPR_NUM' TERM'` | `; $` | Regla 32: TERM' -> λ |
| 34 | `$ SEMICOLON EXPR_COND' EXPR_AND' EXPR_NUM'` | `; $` | Regla 28: EXPR_NUM' -> λ |
| 35 | `$ SEMICOLON EXPR_COND' EXPR_AND'` | `; $` | Regla 14: EXPR_AND' -> λ |
| 36 | `$ SEMICOLON EXPR_COND'` | `; $` | Regla 11: EXPR_COND' -> λ |
| 37 | `$ SEMICOLON` | `; $` | Emparejar terminal ';' |
| 38 | `$` | `$` | Aceptar ($ == $) |

## 3. Ejemplo de Diagnóstico de Error Sintáctico

Para la consulta errónea `SELECT id usuarios ;` (omisión obligatoria de la palabra clave `FROM`):

```text
Error sintáctico [línea 1, columna 11]: se encontró 'usuarios' (IDENTIFIER). Se esperaba uno de los siguientes: [FROM_KW, COMMA]. Detalle: No existe regla de producción en la tabla LL(1) para el símbolo no terminal 'ID_P'.
```
