#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../parser_ll1/parser.h"
#include "../src/lexer.h"
#include "ast.h"

namespace semantic {

// Resultado de la fase Lexer -> Parser LL(1) -> Construcción del AST (SDT)
struct ASTBuildResult {
    bool success = false;
    parser_ll1::ParseResult parseResult;
    std::unique_ptr<SelectStmtNode> ast;
};

// Constructor del AST mediante Traducción Dirigida por la Sintaxis (SDT)
// acoplada al flujo validado por el Lexer y Parser LL(1).
class ASTBuilder {
public:
    ASTBuilder() = default;

    // Ejecuta Lexer + Parser LL(1) y solo construye el AST si no hay errores léxicos/sintácticos
    ASTBuildResult build(const std::string& sqlQuery, bool verboseParser = false) const;

    // Versión a partir de una lista de tokens previamente generada
    ASTBuildResult buildFromTokens(const std::vector<Token>& tokens, bool verboseParser = false) const;

private:
    class SDTTranslator {
    public:
        explicit SDTTranslator(const std::vector<Token>& tokens)
            : tokens_(tokens), pos_(0) {}

        std::unique_ptr<SelectStmtNode> parseConsulta();

    private:
        const std::vector<Token>& tokens_;
        size_t pos_;

        const Token& current() const;
        const Token& consume(TokenType expected);

        void parseColumna(SelectStmtNode& stmt);
        void parseIdList(SelectStmtNode& stmt);
        void parseIdListPrima(SelectStmtNode& stmt);

        std::unique_ptr<ASTNode> parseCondicion();
        std::unique_ptr<ASTNode> parseExprCond();
        std::unique_ptr<ASTNode> parseExprCondPrima(std::unique_ptr<ASTNode> inherited);
        std::unique_ptr<ASTNode> parseExprAnd();
        std::unique_ptr<ASTNode> parseExprAndPrima(std::unique_ptr<ASTNode> inherited);
        std::unique_ptr<ASTNode> parsePred();
        std::unique_ptr<ASTNode> parseVal();
        Token parseOpComp();

        std::unique_ptr<ASTNode> parseExprNum();
        std::unique_ptr<ASTNode> parseExprNumPrima(std::unique_ptr<ASTNode> inherited);
        std::unique_ptr<ASTNode> parseTerm();
        std::unique_ptr<ASTNode> parseTermPrima(std::unique_ptr<ASTNode> inherited);
        std::unique_ptr<ASTNode> parseFactor();
    };
};

} // namespace semantic
