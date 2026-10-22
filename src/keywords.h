#pragma once
#include <string>
#include <unordered_map>
#include "token.h"

// Tabla de palabras reservadas: palabra (en minúscula) -> tipo de token.
// Para agregar una palabra nueva, solo se añade una línea aquí.
const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"select", TokenType::SELECT_KW},
    {"from",   TokenType::FROM_KW},
    {"where",  TokenType::WHERE_KW},
    {"and",    TokenType::AND_KW},
    {"or",     TokenType::OR_KW},
};
