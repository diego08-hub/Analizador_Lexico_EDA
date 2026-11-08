#include "parser.h"
#include "tracer.h"
#include <iostream>

namespace parser_ll1 {

void ParseResult::printTrace() const {
    if (trace.empty()) {
        std::cout << "(No hay pasos de traza registrados)\n";
        return;
    }
    Tracer::printHeader();
    for (const auto& step : trace) {
        Tracer::printRow(step.stepNumber, step.stackStr, step.inputStr, step.actionStr);
    }
    Tracer::printFooter();
}

void ParseResult::printErrors() const {
    if (errors.empty()) {
        std::cout << "[OK] No se encontraron errores sintácticos.\n";
        return;
    }
    std::cout << "\n=== DETECCIÓN DE ERRORES SINTÁCTICOS ===\n";
    for (const auto& err : errors) {
        std::cout << "✗ " << err.format() << "\n";
    }
    std::cout << "========================================\n\n";
}

Parser::Parser() : table_() {}

Parser::Parser(const ParseTable& table) : table_(table) {}

ParseResult Parser::parse(const std::string& sqlQuery, bool verbose) {
    Lexer lexer(sqlQuery);
    std::vector<Token> tokens = lexer.tokenize();
    return parseTokens(tokens, verbose);
}

ParseResult Parser::parseTokens(const std::vector<Token>& tokens, bool verbose) {
    ParseResult result;
    result.success = false;

    // Verificar si hay errores léxicos previos en los tokens
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::ERROR) {
            SyntaxError err;
            err.line = tok.line;
            err.column = tok.column;
            err.foundLexeme = tok.lexeme;
            err.foundType = tok.type;
            err.message = "Error léxico previo: " + tok.message;
            result.errors.push_back(err);
            if (verbose) {
                result.printErrors();
            }
            return result;
        }
    }

    if (tokens.empty()) {
        SyntaxError err;
        err.line = 1;
        err.column = 1;
        err.foundLexeme = "";
        err.foundType = TokenType::END_OF_FILE;
        err.message = "Entrada vacía sin tokens para analizar.";
        result.errors.push_back(err);
        return result;
    }

    // Inicialización de la pila sintáctica: Base [$], Cima [CONSULTA]
    // Usamos std::vector para poder imprimir el contenido de la pila en orden
    std::vector<Symbol> stack;
    stack.push_back(Symbol::makeTerminal(TokenType::END_OF_FILE)); // $
    stack.push_back(Symbol::makeNonTerminal(NonTerminal::CONSULTA)); // Símbolo inicial

    size_t tokenIndex = 0;
    int stepCount = 0;

    if (verbose) {
        Tracer::printHeader();
    }

    while (!stack.empty()) {
        stepCount++;
        Symbol top = stack.back();

        // Obtener token actual (lookahead)
        Token currentToken = (tokenIndex < tokens.size()) ? tokens[tokenIndex] : tokens.back();

        std::string stackStr = Tracer::formatStack(stack);
        std::string inputStr = Tracer::formatInput(tokens, tokenIndex);
        std::string actionStr = "";

        // Caso 1: La cima de la pila es un TERMINAL
        if (top.isTerminal) {
            if (top.terminal == currentToken.type) {
                if (top.terminal == TokenType::END_OF_FILE) {
                    actionStr = "Aceptar ($ == $)";
                    result.trace.push_back({stepCount, stackStr, inputStr, actionStr});
                    if (verbose) {
                        Tracer::printRow(stepCount, stackStr, inputStr, actionStr);
                    }
                    stack.pop_back();
                    result.success = true;
                    break;
                } else {
                    actionStr = "Emparejar terminal '" + currentToken.lexeme + "'";
                    result.trace.push_back({stepCount, stackStr, inputStr, actionStr});
                    if (verbose) {
                        Tracer::printRow(stepCount, stackStr, inputStr, actionStr);
                    }
                    stack.pop_back();
                    tokenIndex++;
                }
            } else {
                // Fallo de match
                actionStr = "Error: se esperaba '" + tokenTypeToString(top.terminal) + "'";
                result.trace.push_back({stepCount, stackStr, inputStr, actionStr});
                if (verbose) {
                    Tracer::printRow(stepCount, stackStr, inputStr, actionStr);
                }

                SyntaxError err;
                err.line = currentToken.line;
                err.column = currentToken.column;
                err.foundLexeme = currentToken.lexeme;
                err.foundType = currentToken.type;
                err.expectedTokens = {top.terminal};
                err.message = "Fallo de emparejamiento (match) en la cima de la pila.";
                result.errors.push_back(err);
                break;
            }
        }
        // Caso 2: La cima de la pila es un NO TERMINAL
        else {
            const ProductionRule* rule = table_.getRule(top.nonTerminal, currentToken.type);
            if (rule != nullptr) {
                actionStr = rule->toString();
                result.trace.push_back({stepCount, stackStr, inputStr, actionStr});
                if (verbose) {
                    Tracer::printRow(stepCount, stackStr, inputStr, actionStr);
                }

                stack.pop_back(); // Desapilar el No Terminal

                // Apilar el consecuente (RHS) en orden inverso
                const auto& rhs = rule->rhs;
                for (auto it = rhs.rbegin(); it != rhs.rend(); ++it) {
                    stack.push_back(*it);
                }
            } else {
                // Celda vacía en la tabla M[A, a]
                actionStr = "Error: entrada no válida para " + nonTerminalToString(top.nonTerminal);
                result.trace.push_back({stepCount, stackStr, inputStr, actionStr});
                if (verbose) {
                    Tracer::printRow(stepCount, stackStr, inputStr, actionStr);
                }

                SyntaxError err;
                err.line = currentToken.line;
                err.column = currentToken.column;
                err.foundLexeme = currentToken.lexeme;
                err.foundType = currentToken.type;
                err.expectedTokens = table_.getExpectedTokens(top.nonTerminal);
                err.message = "No existe regla de producción en la tabla LL(1) para el símbolo no terminal '" +
                              nonTerminalToString(top.nonTerminal) + "'.";
                result.errors.push_back(err);
                break;
            }
        }
    }

    if (verbose) {
        Tracer::printFooter();
        if (result.success) {
            std::cout << ">>> RESULTADO: SENTENCIA SQL VÁLIDA (ACEPTADA) <<<\n\n";
        } else {
            result.printErrors();
        }
    }

    return result;
}

} // namespace parser_ll1
