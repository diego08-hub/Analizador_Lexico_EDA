# Analizador Léxico, Sintáctico LL(1) y Semántico SQL (C++)

Fase 2 del proyecto de servidor de base de datos (curso Estructuras de Datos Avanzadas):
- **Laboratorio 4:** Analizador Léxico (scanner de tokens).
- **Laboratorio 5:** Analizador Sintáctico Predictivo No Recursivo LL(1) basado en pila.
- **Laboratorio 6:** Analizador Semántico, Catálogo de Metadatos (Tabla de Símbolos) y Construcción del AST decorado.

## Integrantes

- Diego Canqui Ecos
- Diego Mauricio Villanueva Flores

## Compilar, ejecutar y probar

Requiere `g++` con soporte C++17 y `make`.

```bash
make all          # compila ./lexer, ./parser_cli, ./semantic_cli, ./test_lexer, ./test_grammar, ./test_parser, ./test_semantic y ./test_catalog
make test         # compila y corre todas las suites (léxico, FIRST/FOLLOW, sintáctico, semántico y catálogo)
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

### Ejecutar Analizador Semántico (Práctica 6)
```bash
# Analizar una consulta con el esquema por defecto en memoria
./semantic_cli "SELECT id, nombre, salario FROM empleados WHERE salario > 2000.0;"

# Analizar usando un archivo de catálogo externo (CSV)
./semantic_cli --catalog semantic_analyzer/schema.csv "SELECT * FROM proyectos;"

# Modo interactivo semántico (muestra el AST decorado para consultas válidas)
./semantic_cli
```

## Estructura del Proyecto

| Archivo / Carpeta | Responsabilidad |
|---|---|
| [src/token.h](src/token.h) | Modelo de tokens: `TokenType`, `Token` (tipo, lexema, línea, columna, mensaje) |
| [src/keywords.h](src/keywords.h) | Tabla hash de palabras reservadas |
| [src/lexer.h](src/lexer.h), [src/lexer.cpp](src/lexer.cpp) | Reconocedor léxico (clase `Lexer`) |
| [src/main.cpp](src/main.cpp) | Punto de entrada del analizador léxico |
| [parser_ll1/grammar.h](parser_ll1/grammar.h), [parser_ll1/grammar.cpp](parser_ll1/grammar.cpp) | Formalización de la gramática SQL y cálculo algorítmico de FIRST y FOLLOW |
| [parser_ll1/types.h](parser_ll1/types.h) | Símbolos terminales/no terminales, producciones y errores |
| [parser_ll1/parse_table.h](parser_ll1/parse_table.h), [parse_table.cpp](parser_ll1/parse_table.cpp) | Matriz de Parseo LL(1) $M[A, a]$ y reglas gramaticales |
| [parser_ll1/tracer.h](parser_ll1/tracer.h) | Trazador formateador de la pila, entrada restante y acciones |
| [parser_ll1/parser.h](parser_ll1/parser.h), [parser.cpp](parser_ll1/parser.cpp) | Motor LL(1) iterativo basado en pila (`std::vector<Symbol>`) |
| [parser_ll1/main.cpp](parser_ll1/main.cpp) | CLI interactivo y analizador de archivos para el parser |
| [tests/test_lexer.cpp](tests/test_lexer.cpp) | Batería de 46 pruebas léxicas y generación de reporte |
| [tests/test_grammar.cpp](tests/test_grammar.cpp) | Pruebas de verificación de conjuntos FIRST/FOLLOW |
| [tests/test_parser.cpp](tests/test_parser.cpp) | Batería de 30 pruebas sintácticas (válidas, complejas e inválidas) |
| [tests/test_semantic.cpp](tests/test_semantic.cpp) | Batería de 19 pruebas de verificación semántica, coerción y errores |
| [tests/test_catalog.cpp](tests/test_catalog.cpp) | Batería de 35 pruebas unitarias para CatalogManager y AST (Persona 1) |
| [semantic_analyzer/semantic_types.h](semantic_analyzer/semantic_types.h) | Tipos de datos SQL (`DataType`), errores semánticos y metadatos de entidades |
| [semantic_analyzer/catalog.h](semantic_analyzer/catalog.h), [catalog.cpp](semantic_analyzer/catalog.cpp) | Catálogo del sistema (Tabla de Símbolos en $O(1)$) y carga desde CSV |
| [semantic_analyzer/schema.csv](semantic_analyzer/schema.csv) | Esquema con 3 tablas (`empleados`, `departamentos`, `proyectos`) |
| [semantic_analyzer/ast.h](semantic_analyzer/ast.h), [ast.cpp](semantic_analyzer/ast.cpp) | Jerarquía polimórfica del AST y visualizador de AST decorado |
| [semantic_analyzer/ast_builder.h](semantic_analyzer/ast_builder.h), [ast_builder.cpp](semantic_analyzer/ast_builder.cpp) | Construcción del AST mediante Traducción Dirigida por la Sintaxis (SDT) |
| [semantic_analyzer/semantic_analyzer.h](semantic_analyzer/semantic_analyzer.h), [semantic_analyzer.cpp](semantic_analyzer.cpp) | Verificador semántico (alcance, duplicados, comodín `*` y coerción de tipos) |
| [semantic_analyzer/main.cpp](semantic_analyzer/main.cpp) | CLI del analizador semántico (`semantic_cli`) |
| [docs/Gramatica_LL1.md](docs/Gramatica_LL1.md) | Documentación de la gramática formal y conjuntos FIRST/FOLLOW |
| [docs/Esquema_Traduccion.md](docs/Esquema_Traduccion.md) | Documentación formal del SDT, atributos y diagramas de clases del AST y Catálogo |
| [docs/Reporte_Pruebas.md](docs/Reporte_Pruebas.md) | Evidencia de ejecución de pruebas léxicas |
| [docs/Reporte_Pruebas_Parser.md](docs/Reporte_Pruebas_Parser.md) | Evidencia y traza paso a paso de pruebas sintácticas |

### Uso desde otro módulo (p. ej. el parser)

```cpp
#include "src/lexer.h"
#include "parser_ll1/parser.h"

parser_ll1::Parser parser;
auto resultado = parser.parse("SELECT id FROM tabla ;", /*verbose=*/true);
if (resultado.success) {
    // Consulta válida
}
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
| Aritméticos y puntuación | `COMMA`, `SEMICOLON`, `STAR`, `PLUS`, `MINUS`, `SLASH`, `LPAREN`, `RPAREN` | `,` `;` `*` `+` `-` `/` `(` `)` |
| Fin de entrada | `END_OF_FILE` | vacío |
| Error léxico | `ERROR` | texto problemático, con mensaje en `Token::message` |

## Pruebas

```bash
make test
```

Ejecuta las cinco suites de pruebas del proyecto:
1. **Léxico:** 46 comprobaciones agrupadas en los 7 escenarios del laboratorio (reporte generado en [docs/Reporte_Pruebas.md](docs/Reporte_Pruebas.md)).
2. **Gramática y FIRST/FOLLOW:** Verificación de conjuntos de selección matemática.
3. **Parser LL(1):** 30 pruebas para sentencias válidas, proyecciones múltiples, condiciones compuestas, expresiones aritméticas y detección de errores sintácticos (reporte generado en [docs/Reporte_Pruebas_Parser.md](docs/Reporte_Pruebas_Parser.md)).
4. **Catálogo y AST (Persona 1):** 35 pruebas unitarias para `CatalogManager` (búsquedas $O(1)$, case-insensitivity, carga de `schema.csv`, orden ordinal) y manipulación del AST.
5. **Analizador Semántico (Persona 2 e Integración):** 19 pruebas de integración cubriendo las 5 reglas obligatorias, expansión de `SELECT *`, coerción de tipos en `WHERE`, prevención de fallas de memoria y visualización del AST decorado.
