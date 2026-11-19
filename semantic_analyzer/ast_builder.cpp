#include "ast_builder.h"
#include <stdexcept>

namespace semantic {

ASTBuildResult ASTBuilder::build(const std::string& sqlQuery, bool verboseParser) const {
    Lexer lexer(sqlQuery);
    std::vector<Token> tokens = lexer.tokenize();
    return buildFromTokens(tokens, verboseParser);
}

ASTBuildResult ASTBuilder::buildFromTokens(const std::vector<Token>& tokens, bool verboseParser) const {
    ASTBuildResult result;
    parser_ll1::Parser parser;
    result.parseResult = parser.parseTokens(tokens, verboseParser);

    // Requisito 4.1.2: No iniciar análisis semántico ni construir AST si hay errores léxicos o sintácticos
    if (!result.parseResult.success) {
        result.success = false;
        result.ast = nullptr;
        return result;
    }

    try {
        SDTTranslator translator(tokens);
        result.ast = translator.parseConsulta();
        result.success = (result.ast != nullptr);
    } catch (const std::exception&) {
        result.success = false;
        result.ast = nullptr;
    }

    return result;
}

const Token& ASTBuilder::SDTTranslator::current() const {
    if (pos_ < tokens_.size()) {
        return tokens_[pos_];
    }
    return tokens_.back();
}

const Token& ASTBuilder::SDTTranslator::consume(TokenType expected) {
    const Token& tok = current();
    if (tok.type != expected) {
        throw std::runtime_error("Token inesperado durante SDT: se esperaba " +
                                 tokenTypeToString(expected) + " y se encontró " +
                                 tokenTypeToString(tok.type));
    }
    pos_++;
    return tok;
}

// CONSULTA -> SELECT_KW COLUMNA FROM_KW IDENTIFIER CONDICION SEMICOLON
std::unique_ptr<SelectStmtNode> ASTBuilder::SDTTranslator::parseConsulta() {
    const Token& selectTok = consume(TokenType::SELECT_KW);
    auto stmt = std::make_unique<SelectStmtNode>(selectTok.line, selectTok.column);

    parseColumna(*stmt);

    consume(TokenType::FROM_KW);
    const Token& tableTok = consume(TokenType::IDENTIFIER);
    stmt->tableName = tableTok.lexeme;
    stmt->tableLine = tableTok.line;
    stmt->tableColumn = tableTok.column;

    stmt->whereClause = parseCondicion();

    consume(TokenType::SEMICOLON);
    return stmt;
}

// COLUMNA -> STAR | ID
void ASTBuilder::SDTTranslator::parseColumna(SelectStmtNode& stmt) {
    if (current().type == TokenType::STAR) {
        const Token& starTok = consume(TokenType::STAR);
        stmt.isSelectStar = true;
        stmt.starLine = starTok.line;
        stmt.starColumn = starTok.column;
    } else {
        stmt.isSelectStar = false;
        parseIdList(stmt);
    }
}

// ID -> IDENTIFIER ID_P
void ASTBuilder::SDTTranslator::parseIdList(SelectStmtNode& stmt) {
    const Token& idTok = consume(TokenType::IDENTIFIER);
    stmt.projections.push_back(
        std::make_unique<ColumnNode>(idTok.lexeme, idTok.line, idTok.column, false));
    parseIdListPrima(stmt);
}

// ID_P -> COMMA ID | ε
void ASTBuilder::SDTTranslator::parseIdListPrima(SelectStmtNode& stmt) {
    if (current().type == TokenType::COMMA) {
        consume(TokenType::COMMA);
        parseIdList(stmt);
    }
}

// CONDICION -> WHERE_KW EXPR_COND | ε
std::unique_ptr<ASTNode> ASTBuilder::SDTTranslator::parseCondicion() {
    if (current().type == TokenType::WHERE_KW) {
        consume(TokenType::WHERE_KW);
        return parseExprCond();
    }
    return nullptr;
}

// EXPR_COND -> EXPR_AND { EXPR_COND'.h = EXPR_AND.n } EXPR_COND'
std::unique_ptr<ASTNode> ASTBuilder::SDTTranslator::parseExprCond() {
    auto left = parseExprAnd();
    return parseExprCondPrima(std::move(left));
}

// EXPR_COND' -> OR_KW EXPR_AND { h = BinaryExpr(OR, h, EXPR_AND.n) } EXPR_COND' | ε
std::unique_ptr<ASTNode> ASTBuilder::SDTTranslator::parseExprCondPrima(std::unique_ptr<ASTNode> inherited) {
    if (current().type == TokenType::OR_KW) {
        const Token& opTok = consume(TokenType::OR_KW);
        auto rhs = parseExprAnd();
        auto combined = std::make_unique<BinaryExprNode>(
            BinaryExprNode::OpCategory::LOGICAL,
            "OR",
            std::move(inherited),
            std::move(rhs),
            opTok.line,
            opTok.column);
        return parseExprCondPrima(std::move(combined));
    }
    return inherited;
}

// EXPR_AND -> PRED { EXPR_AND'.h = PRED.n } EXPR_AND'
std::unique_ptr<ASTNode> ASTBuilder::SDTTranslator::parseExprAnd() {
    auto left = parsePred();
    return parseExprAndPrima(std::move(left));
}

// EXPR_AND' -> AND_KW PRED { h = BinaryExpr(AND, h, PRED.n) } EXPR_AND' | ε
std::unique_ptr<ASTNode> ASTBuilder::SDTTranslator::parseExprAndPrima(std::unique_ptr<ASTNode> inherited) {
    if (current().type == TokenType::AND_KW) {
        const Token& opTok = consume(TokenType::AND_KW);
        auto rhs = parsePred();
        auto combined = std::make_unique<BinaryExprNode>(
            BinaryExprNode::OpCategory::LOGICAL,
            "AND",
            std::move(inherited),
            std::move(rhs),
            opTok.line,
            opTok.column);
        return parseExprAndPrima(std::move(combined));
    }
    return inherited;
}

// PRED -> VAL OP_COMP VAL
std::unique_ptr<ASTNode> ASTBuilder::SDTTranslator::parsePred() {
    auto lhs = parseVal();
    Token opTok = parseOpComp();
    auto rhs = parseVal();
    return std::make_unique<BinaryExprNode>(
        BinaryExprNode::OpCategory::RELATIONAL,
        opTok.lexeme,
        std::move(lhs),
        std::move(rhs),
        opTok.line,
        opTok.column);
}

// VAL -> IDENTIFIER | STRING | EXPR_NUM
std::unique_ptr<ASTNode> ASTBuilder::SDTTranslator::parseVal() {
    if (current().type == TokenType::IDENTIFIER) {
        const Token& idTok = consume(TokenType::IDENTIFIER);
        return std::make_unique<ColumnNode>(idTok.lexeme, idTok.line, idTok.column, false);
    }
    if (current().type == TokenType::STRING) {
        const Token& strTok = consume(TokenType::STRING);
        return std::make_unique<LiteralNode>(strTok.lexeme, DataType::VARCHAR, strTok.line, strTok.column);
    }
    return parseExprNum();
}

// OP_COMP -> = | <> | != | < | > | <= | >=
Token ASTBuilder::SDTTranslator::parseOpComp() {
    switch (current().type) {
        case TokenType::EQUAL:
        case TokenType::NOT_EQUAL:
        case TokenType::LESS:
        case TokenType::GREATER:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER_EQUAL: {
            Token tok = current();
            pos_++;
            return tok;
        }
        default:
            throw std::runtime_error("Operador relacional inesperado en SDT");
    }
}

// EXPR_NUM -> TERM { EXPR_NUM'.h = TERM.n } EXPR_NUM'
std::unique_ptr<ASTNode> ASTBuilder::SDTTranslator::parseExprNum() {
    auto left = parseTerm();
    return parseExprNumPrima(std::move(left));
}

// EXPR_NUM' -> (+ | -) TERM { h = BinaryExpr(op, h, TERM.n) } EXPR_NUM' | ε
std::unique_ptr<ASTNode> ASTBuilder::SDTTranslator::parseExprNumPrima(std::unique_ptr<ASTNode> inherited) {
    if (current().type == TokenType::PLUS || current().type == TokenType::MINUS) {
        const Token& opTok = current();
        pos_++;
        auto rhs = parseTerm();
        auto combined = std::make_unique<BinaryExprNode>(
            BinaryExprNode::OpCategory::ARITHMETIC,
            opTok.lexeme,
            std::move(inherited),
            std::move(rhs),
            opTok.line,
            opTok.column);
        return parseExprNumPrima(std::move(combined));
    }
    return inherited;
}

// TERM -> FACTOR { TERM'.h = FACTOR.n } TERM'
std::unique_ptr<ASTNode> ASTBuilder::SDTTranslator::parseTerm() {
    auto left = parseFactor();
    return parseTermPrima(std::move(left));
}

// TERM' -> (* | /) FACTOR { h = BinaryExpr(op, h, FACTOR.n) } TERM' | ε
std::unique_ptr<ASTNode> ASTBuilder::SDTTranslator::parseTermPrima(std::unique_ptr<ASTNode> inherited) {
    if (current().type == TokenType::STAR || current().type == TokenType::SLASH) {
        const Token& opTok = current();
        pos_++;
        auto rhs = parseFactor();
        auto combined = std::make_unique<BinaryExprNode>(
            BinaryExprNode::OpCategory::ARITHMETIC,
            opTok.lexeme,
            std::move(inherited),
            std::move(rhs),
            opTok.line,
            opTok.column);
        return parseTermPrima(std::move(combined));
    }
    return inherited;
}

// FACTOR -> LPAREN EXPR_NUM RPAREN | INTEGER | DECIMAL
std::unique_ptr<ASTNode> ASTBuilder::SDTTranslator::parseFactor() {
    if (current().type == TokenType::LPAREN) {
        consume(TokenType::LPAREN);
        auto inner = parseExprNum();
        consume(TokenType::RPAREN);
        return inner;
    }
    if (current().type == TokenType::INTEGER) {
        const Token& intTok = consume(TokenType::INTEGER);
        return std::make_unique<LiteralNode>(intTok.lexeme, DataType::INT, intTok.line, intTok.column);
    }
    const Token& decTok = consume(TokenType::DECIMAL);
    return std::make_unique<LiteralNode>(decTok.lexeme, DataType::FLOAT, decTok.line, decTok.column);
}

} // namespace semantic
