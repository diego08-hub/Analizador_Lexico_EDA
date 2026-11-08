#pragma once

#include <vector>
#include <map>
#include <set>
#include <memory>
#include "types.h"

namespace parser_ll1 {

class ParseTable {
public:
    ParseTable();

    // Devuelve la regla a aplicar para el par (NoTerminal, TokenType) o nullptr si celda vacía
    const ProductionRule* getRule(NonTerminal nt, TokenType terminal) const;

    // Devuelve los tokens terminales válidos para un No Terminal (fila no vacía)
    std::vector<TokenType> getExpectedTokens(NonTerminal nt) const;

    // Obtener todas las reglas registradas
    const std::vector<ProductionRule>& getRules() const { return rules_; }

    // Imprime la tabla de parseo en formato legible
    void printTable() const;

private:
    void initRules();
    void initTable();
    void addEntry(NonTerminal nt, TokenType terminal, int ruleId);

    std::vector<ProductionRule> rules_;
    // Mapeo (NonTerminal, TokenType) -> ruleId
    std::map<std::pair<NonTerminal, TokenType>, int> table_;
};

} // namespace parser_ll1
