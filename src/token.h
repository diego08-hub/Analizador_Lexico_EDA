#pragma once
#include <string>

// Todos los tipos de token que reconoce el analizador léxico
enum class TokenType {
    // Palabras reservadas
    SELECT_KW,
    FROM_KW,
    WHERE_KW,
    AND_KW,
    OR_KW,

    // Nombres de tablas, columnas, etc.
    IDENTIFIER,

    // Literales
    INTEGER,   // 18
    DECIMAL,   // 3.14
    STRING,    // 'hola'

    // Operadores relacionales
    EQUAL,         // =
    NOT_EQUAL,     // <> y !=
    LESS,          // <
    GREATER,       // >
    LESS_EQUAL,    // <=
    GREATER_EQUAL, // >=

    // Signos de puntuación
    COMMA,      // ,
    SEMICOLON,  // ;
    STAR,

    // No quedan más caracteres
    END_OF_FILE,

    //Error lexico
    ERROR
};

// Un token guarda su tipo, el texto original (lexema) y dónde empieza
struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
    std::string message; 
};

// Convierte el tipo a texto para poder imprimirlo
inline std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::SELECT_KW:     return "SELECT_KW";
        case TokenType::FROM_KW:       return "FROM_KW";
        case TokenType::WHERE_KW:      return "WHERE_KW";
        case TokenType::AND_KW:        return "AND_KW";
        case TokenType::OR_KW:         return "OR_KW";
        case TokenType::IDENTIFIER:    return "IDENTIFIER";
        case TokenType::INTEGER:       return "INTEGER";
        case TokenType::DECIMAL:       return "DECIMAL";
        case TokenType::STRING:        return "STRING";
        case TokenType::EQUAL:         return "EQUAL";
        case TokenType::NOT_EQUAL:     return "NOT_EQUAL";
        case TokenType::LESS:          return "LESS";
        case TokenType::GREATER:       return "GREATER";
        case TokenType::LESS_EQUAL:    return "LESS_EQUAL";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::COMMA:         return "COMMA";
        case TokenType::SEMICOLON:     return "SEMICOLON";
        case TokenType::STAR:          return "STAR";
        case TokenType::END_OF_FILE:   return "END_OF_FILE";
        case TokenType::ERROR:         return "ERROR";
    }
    return "UNKNOWN";
}
