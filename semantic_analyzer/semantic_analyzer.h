#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include "ast.h"
#include "ast_builder.h"
#include "catalog.h"
#include "semantic_types.h"

namespace semantic {

// Política para columnas repetidas explícitamente en SELECT
enum class DuplicatePolicy {
    REJECT_ERROR, // Reporta error semántico y marca la consulta como inválida
    WARN          // Reporta advertencia pero permite continuar
};

// Resultado completo del pipeline (Lexer -> Parser -> AST -> SemanticAnalyzer)
struct SemanticResult {
    bool success = false;
    bool lexicalOrSyntaxFailed = false;
    parser_ll1::ParseResult parseResult;
    std::unique_ptr<SelectStmtNode> ast;
    std::vector<SemanticError> errors;
    std::vector<SemanticError> warnings;

    void printErrors(std::ostream& os) const;
    void printDecoratedAST(std::ostream& os) const;
};

// Verificador semántico sobre el AST contra el Catálogo del Sistema
class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(const CatalogManager& catalog,
                              DuplicatePolicy duplicatePolicy = DuplicatePolicy::REJECT_ERROR);

    // Ejecuta el pipeline completo: Lexer -> Parser LL(1) -> AST -> Validación Semántica.
    // Garantiza que el análisis semántico NO se inicia si hay errores léxicos o sintácticos.
    SemanticResult analyzeQuery(const std::string& sqlQuery, bool verboseParser = false) const;

    // Valida y decora un AST ya construido
    bool analyzeAST(SelectStmtNode& stmt,
                    std::vector<SemanticError>& errors,
                    std::vector<SemanticError>& warnings) const;

private:
    const CatalogManager& catalog_;
    DuplicatePolicy duplicatePolicy_;

    void validateSelectClause(SelectStmtNode& stmt,
                              std::vector<SemanticError>& errors,
                              std::vector<SemanticError>& warnings) const;

    DataType analyzeExpression(ASTNode* node,
                               const std::string& tableName,
                               std::vector<SemanticError>& errors) const;

    DataType analyzeBinaryExpr(BinaryExprNode* bin,
                               const std::string& tableName,
                               std::vector<SemanticError>& errors) const;
};

} // namespace semantic
