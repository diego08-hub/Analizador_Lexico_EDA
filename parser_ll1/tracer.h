#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <stack>
#include <sstream>
#include "types.h"

namespace parser_ll1 {

class Tracer {
public:
    static std::string formatStack(const std::vector<Symbol>& stackSymbols) {
        std::ostringstream oss;
        for (const auto& sym : stackSymbols) {
            oss << sym.toString() << " ";
        }
        std::string s = oss.str();
        if (!s.empty() && s.back() == ' ') s.pop_back();
        return s;
    }

    static std::string formatInput(const std::vector<Token>& tokens, size_t currentIndex, size_t maxTokens = 4) {
        std::ostringstream oss;
        size_t count = 0;
        for (size_t i = currentIndex; i < tokens.size(); ++i) {
            if (count >= maxTokens) {
                oss << "...";
                break;
            }
            if (count > 0) oss << " ";
            if (tokens[i].type == TokenType::END_OF_FILE) {
                oss << "$";
            } else {
                oss << tokens[i].lexeme;
            }
            count++;
        }
        return oss.str();
    }

    static void printHeader() {
        std::cout << "\n"
                  << "+-------+----------------------------------------+-----------------------------+------------------------------------------------+\n"
                  << "| Paso  | Pila (base -> cima)                    | Entrada restante            | Acción / Regla aplicada                        |\n"
                  << "+-------+----------------------------------------+-----------------------------+------------------------------------------------+\n";
    }

    static void printRow(int step, const std::string& stackStr, const std::string& inputStr, const std::string& actionStr) {
        auto pad = [](const std::string& str, size_t width) -> std::string {
            if (str.length() > width) {
                return str.substr(0, width - 3) + "...";
            }
            return str + std::string(width - str.length(), ' ');
        };

        std::cout << "| " << std::setw(5) << step << " | "
                  << pad(stackStr, 38) << " | "
                  << pad(inputStr, 27) << " | "
                  << pad(actionStr, 46) << " |\n";
    }

    static void printFooter() {
        std::cout << "+-------+----------------------------------------+-----------------------------+------------------------------------------------+\n\n";
    }
};

} // namespace parser_ll1
