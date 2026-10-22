#pragma once
#include <string>
#include <vector>
#include "token.h"

class Lexer {
public:
    Lexer(const std::string& source);

    // Recorre toda la entrada y devuelve los tokens (el último es END_OF_FILE)
    std::vector<Token> tokenize();

    // Mensajes de los errores léxicos encontrados
    const std::vector<std::string>& getErrors() const;

private:
    std::string source;
    int pos;      // posición actual en el texto
    int line;     // línea actual (empieza en 1)
    int column;   // columna actual (empieza en 1)
    std::vector<Token> tokens;
    std::vector<std::string> errors;

    // Manejo de caracteres
    bool isAtEnd() const;
    char peek() const;      // carácter actual, sin avanzar
    char peekNext() const;  // el siguiente, sin avanzar
    char advance();         // devuelve el carácter actual y avanza

    // Reconocimiento de tokens
    void skipWhitespace();
    void scanToken();
    void readIdentifier(int startLine, int startColumn);
    void readNumber(int startLine, int startColumn);
    void readString(int startLine, int startColumn);
    void readSymbol(int startLine, int startColumn);

    // Ayudas para guardar tokens y errores
    void addToken(TokenType type, const std::string& lexeme, int startLine, int startColumn);
    void addError(const std::string& message, int startLine, int startColumn);
};
