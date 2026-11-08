#include "parse_table.h"
#include <iostream>
#include <iomanip>

namespace parser_ll1 {

static Symbol T(TokenType t) { return Symbol::makeTerminal(t); }
static Symbol NT(NonTerminal nt) { return Symbol::makeNonTerminal(nt); }

ParseTable::ParseTable() {
    initRules();
    initTable();
}

void ParseTable::initRules() {
    rules_.clear();
    // Nota: El índice en el vector será id - 1
    // Regla 1: CONSULTA -> SELECT COLUMNA FROM id CONDICION ;
    rules_.push_back({1, NonTerminal::CONSULTA, {T(TokenType::SELECT_KW), NT(NonTerminal::COLUMNA), T(TokenType::FROM_KW), T(TokenType::IDENTIFIER), NT(NonTerminal::CONDICION), T(TokenType::SEMICOLON)}});

    // Regla 2: COLUMNA -> *
    rules_.push_back({2, NonTerminal::COLUMNA, {T(TokenType::STAR)}});

    // Regla 3: COLUMNA -> ID
    rules_.push_back({3, NonTerminal::COLUMNA, {NT(NonTerminal::ID)}});

    // Regla 4: ID -> id ID_P
    rules_.push_back({4, NonTerminal::ID, {T(TokenType::IDENTIFIER), NT(NonTerminal::ID_P)}});

    // Regla 5: ID_P -> , ID
    rules_.push_back({5, NonTerminal::ID_P, {T(TokenType::COMMA), NT(NonTerminal::ID)}});

    // Regla 6: ID_P -> λ
    rules_.push_back({6, NonTerminal::ID_P, {}});

    // Regla 7: CONDICION -> WHERE EXPR_COND
    rules_.push_back({7, NonTerminal::CONDICION, {T(TokenType::WHERE_KW), NT(NonTerminal::EXPR_COND)}});

    // Regla 8: CONDICION -> λ
    rules_.push_back({8, NonTerminal::CONDICION, {}});

    // Regla 9: EXPR_COND -> EXPR_AND EXPR_COND'
    rules_.push_back({9, NonTerminal::EXPR_COND, {NT(NonTerminal::EXPR_AND), NT(NonTerminal::EXPR_COND_P)}});

    // Regla 10: EXPR_COND' -> OR EXPR_AND EXPR_COND'
    rules_.push_back({10, NonTerminal::EXPR_COND_P, {T(TokenType::OR_KW), NT(NonTerminal::EXPR_AND), NT(NonTerminal::EXPR_COND_P)}});

    // Regla 11: EXPR_COND' -> λ
    rules_.push_back({11, NonTerminal::EXPR_COND_P, {}});

    // Regla 12: EXPR_AND -> PRED EXPR_AND'
    rules_.push_back({12, NonTerminal::EXPR_AND, {NT(NonTerminal::PRED), NT(NonTerminal::EXPR_AND_P)}});

    // Regla 13: EXPR_AND' -> AND PRED EXPR_AND'
    rules_.push_back({13, NonTerminal::EXPR_AND_P, {T(TokenType::AND_KW), NT(NonTerminal::PRED), NT(NonTerminal::EXPR_AND_P)}});

    // Regla 14: EXPR_AND' -> λ
    rules_.push_back({14, NonTerminal::EXPR_AND_P, {}});

    // Regla 15: PRED -> VAL OP_COMP VAL
    rules_.push_back({15, NonTerminal::PRED, {NT(NonTerminal::VAL), NT(NonTerminal::OP_COMP), NT(NonTerminal::VAL)}});

    // Regla 16: VAL -> id
    rules_.push_back({16, NonTerminal::VAL, {T(TokenType::IDENTIFIER)}});

    // Regla 17: VAL -> string
    rules_.push_back({17, NonTerminal::VAL, {T(TokenType::STRING)}});

    // Regla 18: VAL -> EXPR_NUM
    rules_.push_back({18, NonTerminal::VAL, {NT(NonTerminal::EXPR_NUM)}});

    // Reglas 19 a 24: OP_COMP
    rules_.push_back({19, NonTerminal::OP_COMP, {T(TokenType::EQUAL)}});
    rules_.push_back({20, NonTerminal::OP_COMP, {T(TokenType::NOT_EQUAL)}});
    rules_.push_back({21, NonTerminal::OP_COMP, {T(TokenType::LESS)}});
    rules_.push_back({22, NonTerminal::OP_COMP, {T(TokenType::GREATER)}});
    rules_.push_back({23, NonTerminal::OP_COMP, {T(TokenType::LESS_EQUAL)}});
    rules_.push_back({24, NonTerminal::OP_COMP, {T(TokenType::GREATER_EQUAL)}});

    // Regla 25: EXPR_NUM -> TERM EXPR_NUM'
    rules_.push_back({25, NonTerminal::EXPR_NUM, {NT(NonTerminal::TERM), NT(NonTerminal::EXPR_NUM_P)}});

    // Regla 26: EXPR_NUM' -> + TERM EXPR_NUM'
    rules_.push_back({26, NonTerminal::EXPR_NUM_P, {T(TokenType::PLUS), NT(NonTerminal::TERM), NT(NonTerminal::EXPR_NUM_P)}});

    // Regla 27: EXPR_NUM' -> - TERM EXPR_NUM'
    rules_.push_back({27, NonTerminal::EXPR_NUM_P, {T(TokenType::MINUS), NT(NonTerminal::TERM), NT(NonTerminal::EXPR_NUM_P)}});

    // Regla 28: EXPR_NUM' -> λ
    rules_.push_back({28, NonTerminal::EXPR_NUM_P, {}});

    // Regla 29: TERM -> FACTOR TERM'
    rules_.push_back({29, NonTerminal::TERM, {NT(NonTerminal::FACTOR), NT(NonTerminal::TERM_P)}});

    // Regla 30: TERM' -> * FACTOR TERM'
    rules_.push_back({30, NonTerminal::TERM_P, {T(TokenType::STAR), NT(NonTerminal::FACTOR), NT(NonTerminal::TERM_P)}});

    // Regla 31: TERM' -> / FACTOR TERM'
    rules_.push_back({31, NonTerminal::TERM_P, {T(TokenType::SLASH), NT(NonTerminal::FACTOR), NT(NonTerminal::TERM_P)}});

    // Regla 32: TERM' -> λ
    rules_.push_back({32, NonTerminal::TERM_P, {}});

    // Regla 33: FACTOR -> ( EXPR_NUM )
    rules_.push_back({33, NonTerminal::FACTOR, {T(TokenType::LPAREN), NT(NonTerminal::EXPR_NUM), T(TokenType::RPAREN)}});

    // Regla 34: FACTOR -> INTEGER
    rules_.push_back({34, NonTerminal::FACTOR, {T(TokenType::INTEGER)}});

    // Regla 35: FACTOR -> DECIMAL
    rules_.push_back({35, NonTerminal::FACTOR, {T(TokenType::DECIMAL)}});
}

void ParseTable::addEntry(NonTerminal nt, TokenType terminal, int ruleId) {
    table_[{nt, terminal}] = ruleId;
}

void ParseTable::initTable() {
    table_.clear();

    // 1. CONSULTA
    addEntry(NonTerminal::CONSULTA, TokenType::SELECT_KW, 1);

    // 2. COLUMNA
    addEntry(NonTerminal::COLUMNA, TokenType::STAR, 2);
    addEntry(NonTerminal::COLUMNA, TokenType::IDENTIFIER, 3);

    // 3. ID
    addEntry(NonTerminal::ID, TokenType::IDENTIFIER, 4);

    // 4. ID_P
    addEntry(NonTerminal::ID_P, TokenType::COMMA, 5);
    addEntry(NonTerminal::ID_P, TokenType::FROM_KW, 6); // Follow(ID_P)

    // 5. CONDICION
    addEntry(NonTerminal::CONDICION, TokenType::WHERE_KW, 7);
    addEntry(NonTerminal::CONDICION, TokenType::SEMICOLON, 8); // Follow(CONDICION)

    // 6. EXPR_COND
    for (TokenType t : {TokenType::IDENTIFIER, TokenType::STRING, TokenType::LPAREN, TokenType::INTEGER, TokenType::DECIMAL}) {
        addEntry(NonTerminal::EXPR_COND, t, 9);
    }

    // 7. EXPR_COND'
    addEntry(NonTerminal::EXPR_COND_P, TokenType::OR_KW, 10);
    addEntry(NonTerminal::EXPR_COND_P, TokenType::SEMICOLON, 11); // Follow(EXPR_COND')

    // 8. EXPR_AND
    for (TokenType t : {TokenType::IDENTIFIER, TokenType::STRING, TokenType::LPAREN, TokenType::INTEGER, TokenType::DECIMAL}) {
        addEntry(NonTerminal::EXPR_AND, t, 12);
    }

    // 9. EXPR_AND'
    addEntry(NonTerminal::EXPR_AND_P, TokenType::AND_KW, 13);
    addEntry(NonTerminal::EXPR_AND_P, TokenType::OR_KW, 14);     // Follow(EXPR_AND')
    addEntry(NonTerminal::EXPR_AND_P, TokenType::SEMICOLON, 14); // Follow(EXPR_AND')

    // 10. PRED
    for (TokenType t : {TokenType::IDENTIFIER, TokenType::STRING, TokenType::LPAREN, TokenType::INTEGER, TokenType::DECIMAL}) {
        addEntry(NonTerminal::PRED, t, 15);
    }

    // 11. VAL
    addEntry(NonTerminal::VAL, TokenType::IDENTIFIER, 16);
    addEntry(NonTerminal::VAL, TokenType::STRING, 17);
    addEntry(NonTerminal::VAL, TokenType::LPAREN, 18);
    addEntry(NonTerminal::VAL, TokenType::INTEGER, 18);
    addEntry(NonTerminal::VAL, TokenType::DECIMAL, 18);

    // 12. OP_COMP
    addEntry(NonTerminal::OP_COMP, TokenType::EQUAL, 19);
    addEntry(NonTerminal::OP_COMP, TokenType::NOT_EQUAL, 20);
    addEntry(NonTerminal::OP_COMP, TokenType::LESS, 21);
    addEntry(NonTerminal::OP_COMP, TokenType::GREATER, 22);
    addEntry(NonTerminal::OP_COMP, TokenType::LESS_EQUAL, 23);
    addEntry(NonTerminal::OP_COMP, TokenType::GREATER_EQUAL, 24);

    // 13. EXPR_NUM
    addEntry(NonTerminal::EXPR_NUM, TokenType::LPAREN, 25);
    addEntry(NonTerminal::EXPR_NUM, TokenType::INTEGER, 25);
    addEntry(NonTerminal::EXPR_NUM, TokenType::DECIMAL, 25);

    // 14. EXPR_NUM'
    addEntry(NonTerminal::EXPR_NUM_P, TokenType::PLUS, 26);
    addEntry(NonTerminal::EXPR_NUM_P, TokenType::MINUS, 27);
    // Follow(EXPR_NUM'):
    for (TokenType t : {TokenType::EQUAL, TokenType::NOT_EQUAL, TokenType::LESS, TokenType::GREATER,
                        TokenType::LESS_EQUAL, TokenType::GREATER_EQUAL, TokenType::AND_KW,
                        TokenType::OR_KW, TokenType::SEMICOLON, TokenType::RPAREN}) {
        addEntry(NonTerminal::EXPR_NUM_P, t, 28);
    }

    // 15. TERM
    addEntry(NonTerminal::TERM, TokenType::LPAREN, 29);
    addEntry(NonTerminal::TERM, TokenType::INTEGER, 29);
    addEntry(NonTerminal::TERM, TokenType::DECIMAL, 29);

    // 16. TERM'
    addEntry(NonTerminal::TERM_P, TokenType::STAR, 30);
    addEntry(NonTerminal::TERM_P, TokenType::SLASH, 31);
    // Follow(TERM'):
    for (TokenType t : {TokenType::PLUS, TokenType::MINUS, TokenType::EQUAL, TokenType::NOT_EQUAL,
                        TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQUAL,
                        TokenType::GREATER_EQUAL, TokenType::AND_KW, TokenType::OR_KW,
                        TokenType::SEMICOLON, TokenType::RPAREN}) {
        addEntry(NonTerminal::TERM_P, t, 32);
    }

    // 17. FACTOR
    addEntry(NonTerminal::FACTOR, TokenType::LPAREN, 33);
    addEntry(NonTerminal::FACTOR, TokenType::INTEGER, 34);
    addEntry(NonTerminal::FACTOR, TokenType::DECIMAL, 35);
}

const ProductionRule* ParseTable::getRule(NonTerminal nt, TokenType terminal) const {
    auto it = table_.find({nt, terminal});
    if (it != table_.end()) {
        int ruleId = it->second;
        if (ruleId >= 1 && ruleId <= static_cast<int>(rules_.size())) {
            return &rules_[ruleId - 1];
        }
    }
    return nullptr;
}

std::vector<TokenType> ParseTable::getExpectedTokens(NonTerminal nt) const {
    std::vector<TokenType> tokens;
    for (const auto& [key, ruleId] : table_) {
        if (key.first == nt) {
            tokens.push_back(key.second);
        }
    }
    return tokens;
}

void ParseTable::printTable() const {
    std::cout << "\n==================== TABLA DE PARSEO LL(1) ====================\n";
    for (const auto& rule : rules_) {
        std::cout << rule.toString() << "\n";
    }
    std::cout << "=================================================================\n\n";
}

} // namespace parser_ll1
