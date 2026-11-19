#include "semantic_analyzer.h"
#include <unordered_set>

namespace semantic {

void SemanticResult::printErrors(std::ostream& os) const {
    if (lexicalOrSyntaxFailed) {
        os << "=== ANÁLISIS SEMÁNTICO OMITIDO (ERROR LÉXICO O SINTÁCTICO PREVIO) ===\n";
        for (const auto& err : parseResult.errors) {
            os << "✗ " << err.format() << "\n";
        }
        os << "====================================================================\n";
        return;
    }

    if (!warnings.empty()) {
        os << "=== ADVERTENCIAS SEMÁNTICAS ===\n";
        for (const auto& w : warnings) {
            os << "⚠ " << w.format() << "\n";
        }
        os << "===============================\n";
    }

    if (errors.empty()) {
        os << "[OK] Consulta semánticamente válida.\n";
        return;
    }

    os << "=== ERRORES SEMÁNTICOS DETECTADOS ===\n";
    for (const auto& err : errors) {
        os << "✗ " << err.format() << "\n";
    }
    os << "=====================================\n";
}

void SemanticResult::printDecoratedAST(std::ostream& os) const {
    if (!ast) {
        os << "(AST no disponible)\n";
        return;
    }
    os << "=== AST DECORADO ===\n";
    ast->print(os, 0);
    os << "====================\n";
}

SemanticAnalyzer::SemanticAnalyzer(const CatalogManager& catalog, DuplicatePolicy duplicatePolicy)
    : catalog_(catalog), duplicatePolicy_(duplicatePolicy) {}

SemanticResult SemanticAnalyzer::analyzeQuery(const std::string& sqlQuery, bool verboseParser) const {
    SemanticResult result;
    ASTBuilder builder;
    ASTBuildResult buildRes = builder.build(sqlQuery, verboseParser);

    result.parseResult = std::move(buildRes.parseResult);
    if (!buildRes.success || !buildRes.ast) {
        result.success = false;
        result.lexicalOrSyntaxFailed = true;
        return result;
    }

    result.ast = std::move(buildRes.ast);
    result.success = analyzeAST(*result.ast, result.errors, result.warnings);
    return result;
}

bool SemanticAnalyzer::analyzeAST(SelectStmtNode& stmt,
                                  std::vector<SemanticError>& errors,
                                  std::vector<SemanticError>& warnings) const {
    size_t initialErrors = errors.size();

    // Regla 1: Existencia de tabla en cláusula FROM
    if (!catalog_.hasTable(stmt.tableName)) {
        errors.push_back({
            SemanticErrorType::TABLE_NOT_FOUND,
            "La tabla '" + stmt.tableName + "' no existe en el catálogo del sistema.",
            stmt.tableName,
            stmt.tableLine,
            stmt.tableColumn,
            false
        });
        return false;
    }

    // Reglas 2, 3 y 4: Proyección SELECT (expansión de *, existencia de columnas y duplicados)
    validateSelectClause(stmt, errors, warnings);

    // Reglas 2 y 5: Resolución de identificadores y verificación/coerción de tipos en WHERE
    if (stmt.whereClause) {
        DataType whereType = analyzeExpression(stmt.whereClause.get(), stmt.tableName, errors);
        if (whereType != DataType::UNKNOWN && whereType != DataType::BOOLEAN) {
            errors.push_back({
                SemanticErrorType::INVALID_LOGICAL_OPERAND,
                "La cláusula WHERE debe evaluar a BOOLEAN, pero se obtuvo " + dataTypeToString(whereType) + ".",
                "WHERE",
                stmt.whereClause->line,
                stmt.whereClause->column,
                false
            });
        }
    }

    return errors.size() == initialErrors;
}

void SemanticAnalyzer::validateSelectClause(SelectStmtNode& stmt,
                                            std::vector<SemanticError>& errors,
                                            std::vector<SemanticError>& warnings) const {
    // Regla 3: Expansión de comodín (*)
    if (stmt.isSelectStar) {
        stmt.projections.clear();
        std::vector<ColumnMetadata> orderedCols = catalog_.getOrderedColumns(stmt.tableName);
        for (const auto& meta : orderedCols) {
            auto colNode = std::make_unique<ColumnNode>(meta.name, stmt.starLine, stmt.starColumn, true);
            colNode->inferredType = meta.type;
            colNode->ordinalPosition = meta.ordinalPosition;
            stmt.projections.push_back(std::move(colNode));
        }
        return;
    }

    // Regla 2 (SELECT) + Regla 4 (Detección de duplicados)
    std::unordered_set<std::string> seenColumns;
    for (auto& colNode : stmt.projections) {
        if (!colNode) continue;

        const ColumnMetadata* meta = catalog_.getColumn(stmt.tableName, colNode->name);
        if (!meta) {
            errors.push_back({
                SemanticErrorType::COLUMN_NOT_FOUND,
                "La columna '" + colNode->name + "' no pertenece a la tabla '" + stmt.tableName + "'.",
                colNode->name,
                colNode->line,
                colNode->column,
                false
            });
            continue;
        }

        colNode->inferredType = meta->type;
        colNode->ordinalPosition = meta->ordinalPosition;

        std::string normalized = toLowerAscii(colNode->name);
        if (seenColumns.count(normalized) > 0) {
            SemanticError dupErr{
                SemanticErrorType::DUPLICATE_COLUMN,
                "La columna '" + colNode->name + "' está repetida en la cláusula SELECT.",
                colNode->name,
                colNode->line,
                colNode->column,
                duplicatePolicy_ == DuplicatePolicy::WARN
            };
            if (duplicatePolicy_ == DuplicatePolicy::WARN) {
                warnings.push_back(dupErr);
            } else {
                errors.push_back(dupErr);
            }
        } else {
            seenColumns.insert(normalized);
        }
    }
}

DataType SemanticAnalyzer::analyzeExpression(ASTNode* node,
                                             const std::string& tableName,
                                             std::vector<SemanticError>& errors) const {
    if (!node) {
        return DataType::UNKNOWN;
    }

    if (auto* col = dynamic_cast<ColumnNode*>(node)) {
        const ColumnMetadata* meta = catalog_.getColumn(tableName, col->name);
        if (!meta) {
            errors.push_back({
                SemanticErrorType::COLUMN_NOT_FOUND,
                "La columna '" + col->name + "' referenciada en WHERE no pertenece a la tabla '" + tableName + "'.",
                col->name,
                col->line,
                col->column,
                false
            });
            col->inferredType = DataType::UNKNOWN;
            return DataType::UNKNOWN;
        }
        col->inferredType = meta->type;
        col->ordinalPosition = meta->ordinalPosition;
        return col->inferredType;
    }

    if (auto* lit = dynamic_cast<LiteralNode*>(node)) {
        lit->inferredType = lit->literalType;
        return lit->inferredType;
    }

    if (auto* bin = dynamic_cast<BinaryExprNode*>(node)) {
        return analyzeBinaryExpr(bin, tableName, errors);
    }

    return DataType::UNKNOWN;
}

DataType SemanticAnalyzer::analyzeBinaryExpr(BinaryExprNode* bin,
                                             const std::string& tableName,
                                             std::vector<SemanticError>& errors) const {
    DataType leftType = analyzeExpression(bin->left.get(), tableName, errors);
    DataType rightType = analyzeExpression(bin->right.get(), tableName, errors);

    // Si alguno falló previamente (ej. columna inexistente), evitamos errores en cascada
    if (leftType == DataType::UNKNOWN || rightType == DataType::UNKNOWN) {
        bin->inferredType = DataType::UNKNOWN;
        return DataType::UNKNOWN;
    }

    switch (bin->category) {
        case BinaryExprNode::OpCategory::ARITHMETIC: {
            if (!isNumericType(leftType) || !isNumericType(rightType)) {
                errors.push_back({
                    SemanticErrorType::INVALID_ARITHMETIC_OPERAND,
                    "El operador aritmético '" + bin->op + "' requiere operandos numéricos (INT/FLOAT), pero recibió " +
                        dataTypeToString(leftType) + " y " + dataTypeToString(rightType) + ".",
                    bin->op,
                    bin->line,
                    bin->column,
                    false
                });
                bin->inferredType = DataType::UNKNOWN;
                return DataType::UNKNOWN;
            }
            if (leftType != rightType) {
                // Promoción numérica implícita INT -> FLOAT
                bin->hasImplicitCoercion = true;
                bin->inferredType = DataType::FLOAT;
            } else {
                bin->inferredType = leftType;
            }
            return bin->inferredType;
        }

        case BinaryExprNode::OpCategory::RELATIONAL: {
            // Caso 1: Numérico vs Numérico (permite promoción INT <-> FLOAT)
            if (isNumericType(leftType) && isNumericType(rightType)) {
                if (leftType != rightType) {
                    bin->hasImplicitCoercion = true;
                }
                bin->inferredType = DataType::BOOLEAN;
                return DataType::BOOLEAN;
            }

            // Caso 2: VARCHAR vs VARCHAR
            if (leftType == DataType::VARCHAR && rightType == DataType::VARCHAR) {
                bin->inferredType = DataType::BOOLEAN;
                return DataType::BOOLEAN;
            }

            // Caso 3: BOOLEAN vs BOOLEAN (solo igualdad/desigualdad)
            if (leftType == DataType::BOOLEAN && rightType == DataType::BOOLEAN) {
                if (bin->op == "=" || bin->op == "<>" || bin->op == "!=") {
                    bin->inferredType = DataType::BOOLEAN;
                    return DataType::BOOLEAN;
                }
                errors.push_back({
                    SemanticErrorType::TYPE_MISMATCH,
                    "El operador relacional de orden '" + bin->op + "' no es válido para operandos de tipo BOOLEAN.",
                    bin->op,
                    bin->line,
                    bin->column,
                    false
                });
                bin->inferredType = DataType::UNKNOWN;
                return DataType::UNKNOWN;
            }

            // Caso 4: Tipos incompatibles
            errors.push_back({
                SemanticErrorType::TYPE_MISMATCH,
                "Incompatibilidad de tipos en comparación '" + bin->op + "': no se puede comparar " +
                    dataTypeToString(leftType) + " con " + dataTypeToString(rightType) + ".",
                bin->op,
                bin->line,
                bin->column,
                false
            });
            bin->inferredType = DataType::UNKNOWN;
            return DataType::UNKNOWN;
        }

        case BinaryExprNode::OpCategory::LOGICAL: {
            if (leftType != DataType::BOOLEAN || rightType != DataType::BOOLEAN) {
                errors.push_back({
                    SemanticErrorType::INVALID_LOGICAL_OPERAND,
                    "El operador lógico '" + bin->op + "' requiere operandos de tipo BOOLEAN, pero recibió " +
                        dataTypeToString(leftType) + " y " + dataTypeToString(rightType) + ".",
                    bin->op,
                    bin->line,
                    bin->column,
                    false
                });
                bin->inferredType = DataType::UNKNOWN;
                return DataType::UNKNOWN;
            }
            bin->inferredType = DataType::BOOLEAN;
            return DataType::BOOLEAN;
        }
    }

    bin->inferredType = DataType::UNKNOWN;
    return DataType::UNKNOWN;
}

} // namespace semantic
