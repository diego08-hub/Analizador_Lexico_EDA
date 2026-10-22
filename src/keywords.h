#pragma once
#include <cctype>
#include <string>
#include <unordered_map>
#include "token.h"

// Devuelve una copia de la palabra en mayúsculas
inline std::string toUpper(const std::string& s) {
    std::string result = s;
    for (char& c : result) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return result;
}

// Si la palabra es reservada devuelve su tipo; si no, IDENTIFIER.
// El lexema original no se modifica: solo se busca con una copia en mayúsculas.
inline TokenType lookupKeyword(const std::string& word) {
    // Para agregar una palabra reservada nueva, añade una línea aquí
    // (la clave va en MAYÚSCULAS).
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"SELECT", TokenType::SELECT_KW},
        {"FROM",   TokenType::FROM_KW},
        {"WHERE",  TokenType::WHERE_KW},
        {"AND",    TokenType::AND_KW},
        {"OR",     TokenType::OR_KW}
    };

    auto it = keywords.find(toUpper(word));
    if (it != keywords.end()) {
        return it->second;
    }
    return TokenType::IDENTIFIER;
}