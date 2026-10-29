# Informe – Laboratorio 4: Analizador Léxico SQL

**Curso:** Estructuras de Datos Avanzadas · **Lenguaje:** C++17
**Integrantes:** Diego Canqui Ecos, Diego Mauricio Villanueva Flores
**Repositorio:** https://github.com/diego08-hub/Analizador_Lexico_EDA

## 1. Introducción
Se implementó el analizador léxico de un subconjunto SQL de sentencias `SELECT`. Recibe el texto de la consulta y produce una secuencia ordenada de tokens con tipo, lexema y posición (línea y columna). No ejecuta consultas ni valida tablas o columnas.

## 2. Arquitectura
Módulo independiente del resto del motor, con responsabilidades separadas:

| Componente | Archivo | Función |
|---|---|---|
| Modelo de tokens | `src/token.h` | `TokenType`, `Token`, conversión a texto |
| Tabla de palabras reservadas | `src/keywords.h` | `lookupKeyword` (búsqueda sin distinguir mayúsculas) |
| Reconocedor léxico | `src/lexer.h/.cpp` | clase `Lexer` |
| Entrada y salida | `src/main.cpp` | lectura desde archivo o stdin; tabla de tokens y errores |
| Pruebas | `tests/test_lexer.cpp` | 46 comprobaciones |

### Algoritmo
`tokenize()` recorre la entrada una sola vez, de izquierda a derecha: omite espacios (`skipWhitespace`) y decide por el primer carácter con una lectura de un carácter hacia adelante (`peek`/`peekNext`):

- letra o `_` → `readIdentifier` (luego consulta la tabla de reservadas);
- dígito → `readNumber` (entero o decimal);
- `'` → `readString`;
- otro → `readSymbol` (operadores y puntuación; los de dos caracteres se resuelven mirando el siguiente, lo que garantiza que `>=`, `<=`, `<>`, `!=` no se dividan).

`advance()` actualiza línea y columna (columna se reinicia tras `\n`). Cada token guarda la posición de su primer carácter. Al final se agrega `END_OF_FILE`. Complejidad O(n) en tiempo y espacio.

## 3. Modelo de tokens
Ver la tabla completa y las reglas léxicas (mayúsculas/minúsculas, identificadores, espacios, cadenas, caracteres no admitidos) en el [README](../README.md). Cada `Token` contiene: `type`, `lexeme`, `line`, `column` y `message` (solo para errores).

## 4. Manejo de errores
Los errores léxicos (carácter no válido, cadena sin cerrar, decimal mal formado) generan un token `ERROR` en el lugar donde ocurren y un mensaje `Error léxico en línea L, columna C: …`. Nunca se clasifican como tokens válidos y el análisis continúa, de modo que se reportan todos los errores en una pasada.

## 5. Pruebas y resultados
Ejecución: `make test`. Resultado: **46 pruebas OK, 0 fallidas**. El detalle de cada prueba (consulta, tokens encontrados y errores) se genera automáticamente en [Reporte_Pruebas.md](Reporte_Pruebas.md).

| # | Escenario | Casos verificados |
|---|---|---|
| 1 | Sentencia válida | SELECT/FROM/WHERE completo; `SELECT *` |
| 2 | Operadores compuestos | `>=`, `<=`, `<>`, `!=` como un solo token; `< =` separados; `>>` |
| 3 | Literales | entero, decimal, cadena, cadena vacía, cadena con símbolos |
| 4 | Reservadas vs. identificadores | mayúsculas/minúsculas, lexema original conservado, `selected`/`fromage`, `_id` |
| 5 | Espacios en blanco | espacios, tabs, `\n`, `\r\n`; posiciones tras salto y tab |
| 6 | Errores léxicos | `@`, `!` solo, `1.`, cadena sin cerrar (no cruza líneas), múltiples errores, mensaje con posición |
| 7 | Casos límite | entrada vacía, solo espacios, sentencias incompletas, token al final, posición de EOF, `tokenize` repetido |

Ejemplo de ejecución del programa:

```
$ echo "SELECT a FROM t WHERE x >= 1.5 AND y <> 'hi';" | ./lexer
SELECT_KW "SELECT" 1:1 · IDENTIFIER "a" 1:8 · FROM_KW "FROM" 1:10 · IDENTIFIER "t" 1:15 ·
WHERE_KW "WHERE" 1:17 · IDENTIFIER "x" 1:23 · GREATER_EQUAL ">=" 1:25 · DECIMAL "1.5" 1:28 ·
AND_KW "AND" 1:32 · IDENTIFIER "y" 1:36 · NOT_EQUAL "<>" 1:38 · STRING "'hi'" 1:41 ·
SEMICOLON ";" 1:45 · END_OF_FILE "" 2:1
```

## 6. Limitaciones
No se admite `tabla.columna`, comentarios, comillas dobles ni escapes en cadenas. `12abc` se separa en entero e identificador; el analizador sintáctico lo rechazará.

## 7. Integración con el analizador sintáctico
La API es `Lexer(sql).tokenize()` → `std::vector<Token>` terminado en `END_OF_FILE`, más `getErrors()`. El parser puede consumir el vector secuencialmente.

## 8. Conclusiones
El analizador reconoce todas las categorías pedidas, mantiene el orden y la posición de los tokens, detecta errores sin clasificarlos como válidos, y está cubierto por pruebas de casos válidos, inválidos y límite. Su diseño modular facilita la integración con las siguientes etapas.
