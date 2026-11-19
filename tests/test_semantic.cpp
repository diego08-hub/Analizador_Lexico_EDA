#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include "../semantic_analyzer/catalog.h"
#include "../semantic_analyzer/semantic_analyzer.h"

using namespace semantic;

namespace {

int passedTests = 0;
int failedTests = 0;

void check(bool condition, const std::string& testName, const std::string& detail = "") {
    if (condition) {
        std::cout << "[PASS] " << testName << "\n";
        passedTests++;
    } else {
        std::cerr << "[FAIL] " << testName;
        if (!detail.empty()) {
            std::cerr << " -> " << detail;
        }
        std::cerr << "\n";
        failedTests++;
    }
}

bool hasErrorType(const SemanticResult& res, SemanticErrorType expected) {
    for (const auto& e : res.errors) {
        if (e.type == expected) return true;
    }
    return false;
}

} // namespace

int main() {
    CatalogManager catalog;
    catalog.loadDefaultSchema();

    SemanticAnalyzer analyzer(catalog, DuplicatePolicy::REJECT_ERROR);
    SemanticAnalyzer warnAnalyzer(catalog, DuplicatePolicy::WARN);

    std::cout << "=== BATERÍA DE PRUEBAS: ANALIZADOR SEMÁNTICO (LAB 6) ===\n\n";

    // 1. Consulta válida con proyección específica
    {
        auto res = analyzer.analyzeQuery("SELECT nombre, salario FROM empleados;");
        check(res.success && !res.lexicalOrSyntaxFailed && res.errors.empty(),
              "1. Proyección específica válida");
        check(res.ast && res.ast->projections.size() == 2 &&
                  res.ast->projections[0]->inferredType == DataType::VARCHAR &&
                  res.ast->projections[1]->inferredType == DataType::FLOAT,
              "1b. Tipos inferidos en proyección (VARCHAR, FLOAT)");
    }

    // 2. Expansión de SELECT * en orden ordinal
    {
        auto res = analyzer.analyzeQuery("SELECT * FROM departamentos;");
        check(res.success && res.ast && res.ast->isSelectStar,
              "2. SELECT * válido en tabla departamentos");
        bool orderedOk = res.ast && res.ast->projections.size() == 4 &&
                         res.ast->projections[0]->name == "id" &&
                         res.ast->projections[0]->ordinalPosition == 1 &&
                         res.ast->projections[1]->name == "nombre" &&
                         res.ast->projections[2]->name == "presupuesto" &&
                         res.ast->projections[3]->name == "activo" &&
                         res.ast->projections[3]->inferredType == DataType::BOOLEAN;
        check(orderedOk, "2b. Expansión de * respeta orden ordinal y tipos del catálogo");
    }

    // 3. WHERE compuesto válido con AND, OR, aritmética y promoción INT -> FLOAT
    {
        auto res = analyzer.analyzeQuery(
            "SELECT codigo, titulo FROM proyectos WHERE horas_estimadas >= 40 AND costo < 1500.50 + 200 OR codigo = 10;");
        check(res.success && res.ast && res.ast->whereClause &&
                  res.ast->whereClause->inferredType == DataType::BOOLEAN,
              "3. WHERE compuesto con AND, OR, aritmética y coerción INT->FLOAT");
    }

    // 4. Promoción numérica INT vs FLOAT directa en comparación relacional
    {
        auto res = analyzer.analyzeQuery("SELECT id FROM empleados WHERE salario >= 3000;");
        auto* rel = dynamic_cast<BinaryExprNode*>(res.ast ? res.ast->whereClause.get() : nullptr);
        check(res.success && rel && rel->hasImplicitCoercion && rel->inferredType == DataType::BOOLEAN,
              "4. Coerción numérica implícita FLOAT >= INT marcada en AST");
    }

    // 5. Comparación válida de VARCHAR y BOOLEAN por igualdad + insensibilidad a mayúsculas
    {
        auto res = analyzer.analyzeQuery(
            "SELECT NOMBRE, ACTIVO FROM EMPLEADOS WHERE nombre = 'Carlos' AND activo = activo;");
        check(res.success && res.errors.empty(),
              "5. Comparación válida VARCHAR=VARCHAR y BOOLEAN=BOOLEAN (case-insensitive)");
    }

    // 6. Error: Tabla inexistente en FROM
    {
        auto res = analyzer.analyzeQuery("SELECT id FROM clientes;");
        check(!res.success && !res.lexicalOrSyntaxFailed &&
                  hasErrorType(res, SemanticErrorType::TABLE_NOT_FOUND) &&
                  res.errors[0].line == 1 && res.errors[0].column == 16,
              "6. Error TABLE_NOT_FOUND con línea y columna exactas");
    }

    // 7. Error: Columna inexistente en SELECT
    {
        auto res = analyzer.analyzeQuery("SELECT nombre, apellido FROM empleados;");
        check(!res.success && hasErrorType(res, SemanticErrorType::COLUMN_NOT_FOUND) &&
                  res.errors[0].context == "apellido" && res.errors[0].line == 1 && res.errors[0].column == 16,
              "7. Error COLUMN_NOT_FOUND en SELECT con contexto y posición");
    }

    // 8. Error: Columna inexistente en WHERE
    {
        auto res = analyzer.analyzeQuery("SELECT nombre FROM empleados WHERE telefono = '999';");
        check(!res.success && hasErrorType(res, SemanticErrorType::COLUMN_NOT_FOUND) &&
                  res.errors[0].context == "telefono",
              "8. Error COLUMN_NOT_FOUND en WHERE sin errores en cascada");
    }

    // 9. Detección de columnas duplicadas en SELECT (política REJECT_ERROR y WARN)
    {
        auto resErr = analyzer.analyzeQuery("SELECT nombre, edad, nombre FROM empleados;");
        check(!resErr.success && hasErrorType(resErr, SemanticErrorType::DUPLICATE_COLUMN),
              "9a. Detección de duplicados en SELECT (política REJECT_ERROR)");

        auto resWarn = warnAnalyzer.analyzeQuery("SELECT nombre, edad, nombre FROM empleados;");
        check(resWarn.success && resWarn.warnings.size() == 1 &&
                  resWarn.warnings[0].type == SemanticErrorType::DUPLICATE_COLUMN,
              "9b. Detección de duplicados en SELECT (política WARN)");
    }

    // 10. Error de incompatibilidad de tipos: INT vs VARCHAR (WHERE edad = 'veinte')
    {
        auto res = analyzer.analyzeQuery("SELECT nombre FROM empleados WHERE edad = 'veinte';");
        check(!res.success && hasErrorType(res, SemanticErrorType::TYPE_MISMATCH),
              "10. Error TYPE_MISMATCH: INT = VARCHAR");
    }

    // 11. Error de incompatibilidad de tipos: VARCHAR > INT (WHERE nombre > 100)
    {
        auto res = analyzer.analyzeQuery("SELECT id FROM empleados WHERE nombre > 100;");
        check(!res.success && hasErrorType(res, SemanticErrorType::TYPE_MISMATCH),
              "11. Error TYPE_MISMATCH: VARCHAR > INT");
    }

    // 12. Error de incompatibilidad de tipos con BOOLEAN
    {
        auto res1 = analyzer.analyzeQuery("SELECT id FROM empleados WHERE activo = 1;");
        check(!res1.success && hasErrorType(res1, SemanticErrorType::TYPE_MISMATCH),
              "12a. Error TYPE_MISMATCH: BOOLEAN = INT");

        auto res2 = analyzer.analyzeQuery("SELECT id FROM empleados WHERE activo > activo;");
        check(!res2.success && hasErrorType(res2, SemanticErrorType::TYPE_MISMATCH),
              "12b. Error TYPE_MISMATCH: operador de orden '>' sobre BOOLEAN");
    }

    // 13. No iniciar análisis semántico si hay error léxico o sintáctico
    {
        auto lexErr = analyzer.analyzeQuery("SELECT @ FROM empleados;");
        check(!lexErr.success && lexErr.lexicalOrSyntaxFailed && lexErr.ast == nullptr && lexErr.errors.empty(),
              "13a. Error léxico detiene el pipeline antes del análisis semántico");

        auto synErr = analyzer.analyzeQuery("SELECT nombre FROM empleados WHERE edad = ;");
        check(!synErr.success && synErr.lexicalOrSyntaxFailed && synErr.ast == nullptr && synErr.errors.empty(),
              "13b. Error sintáctico detiene el pipeline antes del análisis semántico");
    }

    // 14. Impresión del AST decorado
    {
        auto res = analyzer.analyzeQuery("SELECT * FROM empleados WHERE edad >= 25 AND salario > 2500.50 + 100;");
        std::ostringstream oss;
        res.printDecoratedAST(oss);
        std::string output = oss.str();
        check(res.success && output.find("SelectStmtNode") != std::string::npos &&
                  output.find("coercion=INT->FLOAT") != std::string::npos &&
                  output.find("from=*") != std::string::npos,
              "14. Impresión del AST decorado incluye nodos, tipos, expansión * y coerción");

        std::cout << "\n--- Ejemplo de salida de AST Decorado ---\n" << output << "\n";
    }

    std::cout << "=== RESUMEN: " << passedTests << " aprobadas, " << failedTests << " fallidas ===\n";
    return failedTests == 0 ? 0 : 1;
}
