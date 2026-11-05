#include "../parser_ll1/grammar.h"

#include <initializer_list>
#include <iostream>
#include <string>

namespace {

bool expectSet(const GrammarSets& sets,
               const std::string& nonTerminal,
               std::initializer_list<std::string> expected) {
    const SymbolSet expectedSet(expected);
    const auto found = sets.find(nonTerminal);
    if (found == sets.end() || found->second != expectedSet) {
        std::cerr << "[FALLO] " << nonTerminal << " no coincide.\n";
        if (found != sets.end()) {
            std::cerr << "  Obtenido:";
            for (const std::string& symbol : found->second) {
                std::cerr << " " << symbol;
            }
            std::cerr << "\n";
        }
        return false;
    }
    return true;
}

}  // namespace

int main() {
    const Grammar& grammar = sqlGrammar();
    const GrammarSets first = calculateFirstSets(grammar);
    const GrammarSets follow = calculateFollowSets(grammar, first);
    int failures = 0;

    failures += !expectSet(first, "CONSULTA", {"SELECT_KW"});
    failures += !expectSet(first, "COLUMNA", {"STAR", "IDENTIFIER"});
    failures += !expectSet(first, "LISTA_ID", {"IDENTIFIER"});
    failures += !expectSet(first, "LISTA_ID_PRIMA", {"COMMA", EPSILON});
    failures += !expectSet(first, "CONDICION", {"WHERE_KW", EPSILON});
    failures += !expectSet(first, "EXPR_OR", {
        "IDENTIFIER", "STRING", "LPAREN", "INTEGER", "DECIMAL"
    });
    failures += !expectSet(first, "EXPR_OR_PRIMA", {"OR_KW", EPSILON});
    failures += !expectSet(first, "EXPR_AND", {
        "IDENTIFIER", "STRING", "LPAREN", "INTEGER", "DECIMAL"
    });
    failures += !expectSet(first, "EXPR_AND_PRIMA", {"AND_KW", EPSILON});
    failures += !expectSet(first, "PREDICADO", {
        "IDENTIFIER", "STRING", "LPAREN", "INTEGER", "DECIMAL"
    });
    failures += !expectSet(first, "VALOR", {
        "IDENTIFIER", "STRING", "LPAREN", "INTEGER", "DECIMAL"
    });
    failures += !expectSet(first, "EXPR_NUM_PRIMA", {"PLUS", "MINUS", EPSILON});
    failures += !expectSet(first, "TERM_PRIMA", {"STAR", "SLASH", EPSILON});
    failures += !expectSet(first, "EXPR_NUM", {"LPAREN", "INTEGER", "DECIMAL"});
    failures += !expectSet(first, "TERM", {"LPAREN", "INTEGER", "DECIMAL"});
    failures += !expectSet(first, "FACTOR", {"LPAREN", "INTEGER", "DECIMAL"});
    failures += !expectSet(first, "OP_COMP", {
        "EQUAL", "NOT_EQUAL", "LESS", "GREATER", "LESS_EQUAL", "GREATER_EQUAL"
    });

    failures += !expectSet(follow, "CONSULTA", {END_MARKER});
    failures += !expectSet(follow, "COLUMNA", {"FROM_KW"});
    failures += !expectSet(follow, "LISTA_ID", {"FROM_KW"});
    failures += !expectSet(follow, "LISTA_ID_PRIMA", {"FROM_KW"});
    failures += !expectSet(follow, "CONDICION", {"SEMICOLON"});
    failures += !expectSet(follow, "EXPR_OR", {"SEMICOLON"});
    failures += !expectSet(follow, "EXPR_OR_PRIMA", {"SEMICOLON"});
    failures += !expectSet(follow, "EXPR_AND", {"OR_KW", "SEMICOLON"});
    failures += !expectSet(follow, "EXPR_AND_PRIMA", {"OR_KW", "SEMICOLON"});
    failures += !expectSet(follow, "PREDICADO", {"AND_KW", "OR_KW", "SEMICOLON"});
    failures += !expectSet(follow, "VALOR", {
        "EQUAL", "NOT_EQUAL", "LESS", "GREATER", "LESS_EQUAL", "GREATER_EQUAL",
        "AND_KW", "OR_KW", "SEMICOLON"
    });
    failures += !expectSet(follow, "OP_COMP", {
        "IDENTIFIER", "STRING", "LPAREN", "INTEGER", "DECIMAL"
    });
    failures += !expectSet(follow, "EXPR_NUM", {
        "EQUAL", "NOT_EQUAL", "LESS", "GREATER", "LESS_EQUAL", "GREATER_EQUAL",
        "AND_KW", "OR_KW", "SEMICOLON", "RPAREN"
    });
    failures += !expectSet(follow, "EXPR_NUM_PRIMA", {
        "EQUAL", "NOT_EQUAL", "LESS", "GREATER", "LESS_EQUAL", "GREATER_EQUAL",
        "AND_KW", "OR_KW", "SEMICOLON", "RPAREN"
    });
    failures += !expectSet(follow, "TERM", {
        "PLUS", "MINUS", "EQUAL", "NOT_EQUAL", "LESS", "GREATER",
        "LESS_EQUAL", "GREATER_EQUAL", "AND_KW", "OR_KW", "SEMICOLON", "RPAREN"
    });
    failures += !expectSet(follow, "TERM_PRIMA", {
        "PLUS", "MINUS", "EQUAL", "NOT_EQUAL", "LESS", "GREATER",
        "LESS_EQUAL", "GREATER_EQUAL", "AND_KW", "OR_KW", "SEMICOLON", "RPAREN"
    });
    failures += !expectSet(follow, "FACTOR", {
        "STAR", "SLASH", "PLUS", "MINUS", "EQUAL", "NOT_EQUAL", "LESS",
        "GREATER", "LESS_EQUAL", "GREATER_EQUAL", "AND_KW", "OR_KW",
        "SEMICOLON", "RPAREN"
    });

    if (failures != 0) {
        std::cerr << failures << " comprobación(es) fallaron.\n";
        return 1;
    }
    std::cout << "Todos los conjuntos FIRST y FOLLOW coinciden con la gramática SQL.\n";
    return 0;
}
