#pragma once

#include <string>
#include <vector>
#include <variant>
#include <sstream>
#include "../src/token.h"

namespace parser_ll1 {

// Símbolos No Terminales de la gramática SQL LL(1)
enum class NonTerminal {
    CONSULTA,
    COLUMNA,
    ID,
    ID_P,
    CONDICION,
    EXPR_COND,
    EXPR_COND_P,
    EXPR_AND,
    EXPR_AND_P,
    PRED,
    VAL,
    OP_COMP,
    EXPR_NUM,
    EXPR_NUM_P,
    TERM,
    TERM_P,
    FACTOR
};

inline std::string nonTerminalToString(NonTerminal nt) {
    switch (nt) {
        case NonTerminal::CONSULTA:    return "CONSULTA";
        case NonTerminal::COLUMNA:     return "COLUMNA";
        case NonTerminal::ID:          return "ID";
        case NonTerminal::ID_P:        return "ID_P";
        case NonTerminal::CONDICION:   return "CONDICION";
        case NonTerminal::EXPR_COND:   return "EXPR_COND";
        case NonTerminal::EXPR_COND_P: return "EXPR_COND'";
        case NonTerminal::EXPR_AND:    return "EXPR_AND";
        case NonTerminal::EXPR_AND_P:  return "EXPR_AND'";
        case NonTerminal::PRED:        return "PRED";
        case NonTerminal::VAL:         return "VAL";
        case NonTerminal::OP_COMP:     return "OP_COMP";
        case NonTerminal::EXPR_NUM:    return "EXPR_NUM";
        case NonTerminal::EXPR_NUM_P:  return "EXPR_NUM'";
        case NonTerminal::TERM:        return "TERM";
        case NonTerminal::TERM_P:      return "TERM'";
        case NonTerminal::FACTOR:      return "FACTOR";
    }
    return "UNKNOWN_NT";
}

// Representación de un símbolo en la pila (Terminal o No Terminal)
struct Symbol {
    bool isTerminal;
    TokenType terminal;
    NonTerminal nonTerminal;

    static Symbol makeTerminal(TokenType t) {
        Symbol s;
        s.isTerminal = true;
        s.terminal = t;
        s.nonTerminal = NonTerminal::CONSULTA;
        return s;
    }

    static Symbol makeNonTerminal(NonTerminal nt) {
        Symbol s;
        s.isTerminal = false;
        s.terminal = TokenType::ERROR;
        s.nonTerminal = nt;
        return s;
    }

    bool operator==(const Symbol& other) const {
        if (isTerminal != other.isTerminal) return false;
        if (isTerminal) return terminal == other.terminal;
        return nonTerminal == other.nonTerminal;
    }

    std::string toString() const {
        if (isTerminal) {
            if (terminal == TokenType::END_OF_FILE) return "$";
            return tokenTypeToString(terminal);
        }
        return nonTerminalToString(nonTerminal);
    }
};

// Regla de producción: A -> X1 X2 ... Xn
struct ProductionRule {
    int id;
    NonTerminal lhs;
    std::vector<Symbol> rhs; // Vacío representa λ (epsilon)

    std::string toString() const {
        std::ostringstream oss;
        oss << "Regla " << id << ": " << nonTerminalToString(lhs) << " -> ";
        if (rhs.empty()) {
            oss << "λ";
        } else {
            for (size_t i = 0; i < rhs.size(); ++i) {
                if (i > 0) oss << " ";
                oss << rhs[i].toString();
            }
        }
        return oss.str();
    }
};

// Detalle de un error sintáctico
struct SyntaxError {
    int line;
    int column;
    std::string foundLexeme;
    TokenType foundType;
    std::vector<TokenType> expectedTokens;
    std::string message;

    std::string format() const {
        std::ostringstream oss;
        oss << "Error sintáctico [línea " << line << ", columna " << column << "]: "
            << "se encontró '" << foundLexeme << "' (" << tokenTypeToString(foundType) << ").";
        if (!expectedTokens.empty()) {
            oss << " Se esperaba uno de los siguientes: [";
            for (size_t i = 0; i < expectedTokens.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << tokenTypeToString(expectedTokens[i]);
            }
            oss << "].";
        }
        if (!message.empty()) {
            oss << " Detalle: " << message;
        }
        return oss.str();
    }
};

// Registro de un paso en la traza de ejecución
struct TraceStep {
    int stepNumber;
    std::string stackStr;
    std::string inputStr;
    std::string actionStr;
};

// Resultado final del análisis sintáctico
struct ParseResult {
    bool success = false;
    std::vector<TraceStep> trace;
    std::vector<SyntaxError> errors;

    void printTrace() const;
    void printErrors() const;
};

} // namespace parser_ll1
