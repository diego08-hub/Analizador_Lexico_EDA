#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include "../semantic_analyzer/ast.h"
#include "../semantic_analyzer/catalog.h"
#include "../semantic_analyzer/semantic_types.h"

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

} // namespace

int main() {
    std::cout << "=== BATERÍA DE PRUEBAS: CATÁLOGO Y AST (PERSONA 1 - LAB 6) ===\n\n";

    CatalogManager catalog;

    // 1. Estado inicial
    {
        check(catalog.tableCount() == 0, "1. Catalogo inicia vacio");
        check(!catalog.hasTable("empleados"), "1b. hasTable retorna false cuando vacio");
    }

    // 2. Carga desde archivo CSV
    {
        bool loaded = catalog.loadFromCSV("semantic_analyzer/schema.csv");
        check(loaded, "2. Carga exitosa desde semantic_analyzer/schema.csv");
        check(catalog.tableCount() == 3, "2b. Carga exactamente 3 tablas (empleados, departamentos, proyectos)");
    }

    // 3. Verificación de existencia de tablas (Case-Insensitive)
    {
        check(catalog.hasTable("empleados"), "3a. hasTable('empleados') minusculas");
        check(catalog.hasTable("EMPLEADOS"), "3b. hasTable('EMPLEADOS') mayusculas");
        check(catalog.hasTable("Departamentos"), "3c. hasTable('Departamentos') mixto");
        check(catalog.hasTable("PROYECTOS"), "3d. hasTable('PROYECTOS') mayusculas");
        check(!catalog.hasTable("clientes"), "3e. hasTable('clientes') inexistente retorna false");
    }

    // 4. Verificación de metadatos de tabla y almacenamiento
    {
        const TableMetadata* tbl = catalog.getTable("empleados");
        check(tbl != nullptr, "4a. getTable('empleados') no es nulo");
        if (tbl) {
            check(tbl->name == "empleados", "4b. Nombre de tabla preservado");
            check(tbl->storagePath == "data/empleados.tbl", "4c. storagePath coincide con CSV");
            check(tbl->orderedColumns.size() == 6, "4d. Tabla empleados tiene 6 columnas");
        }
    }

    // 5. Verificación de columnas, tipos, PKs y posición ordinal
    {
        const ColumnMetadata* colId = catalog.getColumn("empleados", "id");
        check(colId != nullptr, "5a. Columna 'id' encontrada en 'empleados'");
        if (colId) {
            check(colId->ordinalPosition == 1, "5b. id tiene posicion ordinal 1");
            check(colId->type == DataType::INT, "5c. id es de tipo INT");
            check(colId->isPrimaryKey == true, "5d. id es Primary Key");
            check(colId->isNullable == false, "5e. id no es nullable");
        }

        const ColumnMetadata* colNom = catalog.getColumn("empleados", "NOMBRE");
        check(colNom != nullptr && colNom->type == DataType::VARCHAR,
              "5f. Columna 'NOMBRE' case-insensitive es VARCHAR");

        const ColumnMetadata* colSal = catalog.getColumn("empleados", "salario");
        check(colSal != nullptr && colSal->type == DataType::FLOAT,
              "5g. Columna 'salario' es FLOAT");

        const ColumnMetadata* colAct = catalog.getColumn("empleados", "activo");
        check(colAct != nullptr && colAct->type == DataType::BOOLEAN,
              "5h. Columna 'activo' es BOOLEAN");
    }

    // 6. Verificación de columnas inexistentes
    {
        check(!catalog.hasColumn("empleados", "direccion"), "6a. hasColumn para columna no existente");
        check(catalog.getColumn("empleados", "telefono") == nullptr, "6b. getColumn para columna inexistente es null");
        check(catalog.getColumn("tabla_fantasma", "id") == nullptr, "6c. getColumn en tabla no existente es null");
    }

    // 7. Columnas ordenadas para expansion SELECT *
    {
        auto cols = catalog.getOrderedColumns("departamentos");
        check(cols.size() == 4, "7a. departamentos tiene 4 columnas ordenadas");
        bool strictlyOrdered = true;
        for (size_t i = 0; i < cols.size(); ++i) {
            if (cols[i].ordinalPosition != static_cast<int>(i + 1)) {
                strictlyOrdered = false;
                break;
            }
        }
        check(strictlyOrdered, "7b. getOrderedColumns devuelve orden ordinal estricto 1..N");
        if (cols.size() == 4) {
            check(cols[0].name == "id" && cols[1].name == "nombre" &&
                  cols[2].name == "presupuesto" && cols[3].name == "activo",
                  "7c. Nombres y orden de atributos correctos en departamentos");
        }
    }

    // 8. Construcción directa de AST y verificación de nodos
    {
        auto selectStmt = std::make_unique<SelectStmtNode>(1, 1);
        selectStmt->tableName = "empleados";
        selectStmt->tableLine = 1;
        selectStmt->tableColumn = 15;

        // Proyecciones
        auto col1 = std::make_unique<ColumnNode>("id", 1, 8);
        col1->inferredType = DataType::INT;
        col1->ordinalPosition = 1;
        selectStmt->projections.push_back(std::move(col1));

        auto col2 = std::make_unique<ColumnNode>("salario", 1, 12);
        col2->inferredType = DataType::FLOAT;
        col2->ordinalPosition = 4;
        selectStmt->projections.push_back(std::move(col2));

        // Clausula WHERE: salario > 2000.0
        auto leftCol = std::make_unique<ColumnNode>("salario", 1, 30);
        leftCol->inferredType = DataType::FLOAT;
        leftCol->ordinalPosition = 4;

        auto rightLit = std::make_unique<LiteralNode>("2000.0", DataType::FLOAT, 1, 40);

        auto binExpr = std::make_unique<BinaryExprNode>(
            BinaryExprNode::OpCategory::RELATIONAL, ">", std::move(leftCol), std::move(rightLit), 1, 38
        );
        binExpr->inferredType = DataType::BOOLEAN;

        selectStmt->whereClause = std::move(binExpr);

        check(selectStmt->projections.size() == 2, "8a. AST SelectStmtNode contiene 2 proyecciones");
        check(selectStmt->tableName == "empleados", "8b. AST SelectStmtNode tiene tabla 'empleados'");
        check(selectStmt->whereClause != nullptr, "8c. AST SelectStmtNode tiene clausula WHERE");
        check(selectStmt->whereClause->inferredType == DataType::BOOLEAN, "8d. Clausula WHERE evaluada como BOOLEAN");

        // 9. Impresión del AST Decorado
        std::ostringstream oss;
        selectStmt->print(oss, 0);
        std::string astStr = oss.str();
        check(!astStr.empty(), "9a. AST::print genera texto no vacio");
        check(astStr.find("SelectStmtNode") != std::string::npos, "9b. AST contiene SelectStmtNode");
        check(astStr.find("ColumnNode('id')") != std::string::npos, "9c. AST contiene ColumnNode('id')");
        check(astStr.find("type=FLOAT") != std::string::npos, "9d. AST contiene tipo decorado FLOAT");
    }

    std::cout << "\n=== RESUMEN PERSONA 1: " << passedTests << " aprobadas, " << failedTests << " fallidas ===\n";
    return failedTests == 0 ? 0 : 1;
}
