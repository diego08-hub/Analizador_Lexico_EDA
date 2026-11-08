#include "grammar.h"

#include <cstddef>
#include <stdexcept>

namespace {

void validateGrammar(const Grammar& grammar) {
    if (grammar.startSymbol.empty() ||
        grammar.nonTerminals.count(grammar.startSymbol) == 0) {
        throw std::invalid_argument("El símbolo inicial debe ser un no terminal declarado.");
    }
    if (grammar.productions.empty()) {
        throw std::invalid_argument("La gramática debe tener al menos una producción.");
    }
    if (grammar.terminals.count(END_MARKER) != 0 ||
        grammar.nonTerminals.count(END_MARKER) != 0 ||
        grammar.terminals.count(EPSILON) != 0 ||
        grammar.nonTerminals.count(EPSILON) != 0) {
        throw std::invalid_argument("El marcador de fin y epsilon son símbolos reservados.");
    }
    for (const std::string& symbol : grammar.nonTerminals) {
        if (symbol.empty() || grammar.terminals.count(symbol) != 0) {
            throw std::invalid_argument("Los terminales y no terminales deben ser distintos y no vacíos.");
        }
    }
    for (const std::string& symbol : grammar.terminals) {
        if (symbol.empty()) {
            throw std::invalid_argument("Los terminales no pueden estar vacíos.");
        }
    }
    for (const Production& production : grammar.productions) {
        if (grammar.nonTerminals.count(production.lhs) == 0) {
            throw std::invalid_argument("La parte izquierda de cada producción debe ser un no terminal.");
        }
        for (const std::string& symbol : production.rhs) {
            if (grammar.nonTerminals.count(symbol) == 0 &&
                grammar.terminals.count(symbol) == 0) {
                throw std::invalid_argument("La producción contiene un símbolo no declarado: " + symbol);
            }
        }
    }
}

SymbolSet firstOfSequence(const std::vector<std::string>& sequence,
                          const Grammar& grammar,
                          const GrammarSets& firstSets) {
    SymbolSet result;
    bool sequenceIsNullable = true;

    for (const std::string& symbol : sequence) {
        if (grammar.terminals.count(symbol) != 0) {
            result.insert(symbol);
            sequenceIsNullable = false;
            break;
        }

        const SymbolSet& symbolFirst = firstSets.at(symbol);
        for (const std::string& firstSymbol : symbolFirst) {
            if (firstSymbol != EPSILON) {
                result.insert(firstSymbol);
            }
        }
        if (symbolFirst.count(EPSILON) == 0) {
            sequenceIsNullable = false;
            break;
        }
    }

    if (sequenceIsNullable) {
        result.insert(EPSILON);
    }
    return result;
}

}  // namespace

const Grammar& sqlGrammar() {
    static const Grammar grammar{
        "CONSULTA",
        {
            "CONSULTA", "COLUMNA", "LISTA_ID", "LISTA_ID_PRIMA",
            "CONDICION", "EXPR_OR", "EXPR_OR_PRIMA", "EXPR_AND",
            "EXPR_AND_PRIMA", "PREDICADO", "VALOR", "OP_COMP",
            "EXPR_NUM", "EXPR_NUM_PRIMA", "TERM", "TERM_PRIMA", "FACTOR"
        },
        {
            "SELECT_KW", "FROM_KW", "WHERE_KW", "AND_KW", "OR_KW",
            "IDENTIFIER", "INTEGER", "DECIMAL", "STRING", "EQUAL",
            "NOT_EQUAL", "LESS", "GREATER", "LESS_EQUAL", "GREATER_EQUAL",
            "COMMA", "SEMICOLON", "STAR", "PLUS", "MINUS", "SLASH",
            "LPAREN", "RPAREN"
        },
        {
            {"CONSULTA", {"SELECT_KW", "COLUMNA", "FROM_KW", "IDENTIFIER",
                          "CONDICION", "SEMICOLON"}},
            {"COLUMNA", {"STAR"}},
            {"COLUMNA", {"LISTA_ID"}},
            {"LISTA_ID", {"IDENTIFIER", "LISTA_ID_PRIMA"}},
            {"LISTA_ID_PRIMA", {"COMMA", "LISTA_ID"}},
            {"LISTA_ID_PRIMA", {}},
            {"CONDICION", {"WHERE_KW", "EXPR_OR"}},
            {"CONDICION", {}},
            {"EXPR_OR", {"EXPR_AND", "EXPR_OR_PRIMA"}},
            {"EXPR_OR_PRIMA", {"OR_KW", "EXPR_AND", "EXPR_OR_PRIMA"}},
            {"EXPR_OR_PRIMA", {}},
            {"EXPR_AND", {"PREDICADO", "EXPR_AND_PRIMA"}},
            {"EXPR_AND_PRIMA", {"AND_KW", "PREDICADO", "EXPR_AND_PRIMA"}},
            {"EXPR_AND_PRIMA", {}},
            {"PREDICADO", {"VALOR", "OP_COMP", "VALOR"}},
            {"VALOR", {"IDENTIFIER"}},
            {"VALOR", {"STRING"}},
            {"VALOR", {"EXPR_NUM"}},
            {"OP_COMP", {"EQUAL"}},
            {"OP_COMP", {"NOT_EQUAL"}},
            {"OP_COMP", {"LESS"}},
            {"OP_COMP", {"GREATER"}},
            {"OP_COMP", {"LESS_EQUAL"}},
            {"OP_COMP", {"GREATER_EQUAL"}},
            {"EXPR_NUM", {"TERM", "EXPR_NUM_PRIMA"}},
            {"EXPR_NUM_PRIMA", {"PLUS", "TERM", "EXPR_NUM_PRIMA"}},
            {"EXPR_NUM_PRIMA", {"MINUS", "TERM", "EXPR_NUM_PRIMA"}},
            {"EXPR_NUM_PRIMA", {}},
            {"TERM", {"FACTOR", "TERM_PRIMA"}},
            {"TERM_PRIMA", {"STAR", "FACTOR", "TERM_PRIMA"}},
            {"TERM_PRIMA", {"SLASH", "FACTOR", "TERM_PRIMA"}},
            {"TERM_PRIMA", {}},
            {"FACTOR", {"LPAREN", "EXPR_NUM", "RPAREN"}},
            {"FACTOR", {"INTEGER"}},
            {"FACTOR", {"DECIMAL"}}
        }
    };
    return grammar;
}

GrammarSets calculateFirstSets(const Grammar& grammar) {
    validateGrammar(grammar);

    GrammarSets firstSets;
    for (const std::string& nonTerminal : grammar.nonTerminals) {
        firstSets.emplace(nonTerminal, SymbolSet{});
    }

    bool changed;
    do {
        changed = false;
        for (const Production& production : grammar.productions) {
            SymbolSet productionFirst = firstOfSequence(
                production.rhs, grammar, firstSets);
            SymbolSet& lhsFirst = firstSets.at(production.lhs);
            const std::size_t oldSize = lhsFirst.size();
            lhsFirst.insert(productionFirst.begin(), productionFirst.end());
            changed = changed || lhsFirst.size() != oldSize;
        }
    } while (changed);

    return firstSets;
}

GrammarSets calculateFollowSets(const Grammar& grammar, const GrammarSets& firstSets) {
    validateGrammar(grammar);

    if (firstSets.size() != grammar.nonTerminals.size()) {
        throw std::invalid_argument("Los conjuntos FIRST no corresponden a esta gramática.");
    }

    GrammarSets followSets;
    for (const std::string& nonTerminal : grammar.nonTerminals) {
        if (firstSets.count(nonTerminal) == 0) {
            throw std::invalid_argument("Falta FIRST para el no terminal: " + nonTerminal);
        }
        followSets.emplace(nonTerminal, SymbolSet{});
    }
    followSets.at(grammar.startSymbol).insert(END_MARKER);

    bool changed;
    do {
        changed = false;
        for (const Production& production : grammar.productions) {
            for (std::size_t i = 0; i < production.rhs.size(); ++i) {
                const std::string& symbol = production.rhs[i];
                if (grammar.nonTerminals.count(symbol) == 0) {
                    continue;
                }

                std::vector<std::string> suffix(
                    production.rhs.begin() + static_cast<std::ptrdiff_t>(i + 1),
                    production.rhs.end());
                SymbolSet suffixFirst = firstOfSequence(suffix, grammar, firstSets);
                SymbolSet& symbolFollow = followSets.at(symbol);
                const std::size_t oldSize = symbolFollow.size();

                for (const std::string& firstSymbol : suffixFirst) {
                    if (firstSymbol != EPSILON) {
                        symbolFollow.insert(firstSymbol);
                    }
                }
                if (suffixFirst.count(EPSILON) != 0) {
                    const SymbolSet& lhsFollow = followSets.at(production.lhs);
                    symbolFollow.insert(lhsFollow.begin(), lhsFollow.end());
                }
                changed = changed || symbolFollow.size() != oldSize;
            }
        }
    } while (changed);

    return followSets;
}
