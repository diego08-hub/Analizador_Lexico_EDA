#include "lexer.h"
#include "keywords.h"
#include <cctype>

Lexer::Lexer(const std::string& source)
    : source(source), pos(0), line(1), column(1) {}

const std::vector<std::string>& Lexer::getErrors() const {
    return errors;
}

// ---------- Manejo de caracteres ----------

bool Lexer::isAtEnd() const {
    return pos >= (int)source.size();
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[pos];
}

char Lexer::peekNext() const {
    if (pos + 1 >= (int)source.size()) return '\0';
    return source[pos + 1];
}

// Avanza un carácter y actualiza línea y columna
char Lexer::advance() {
    char c = source[pos];
    pos++;
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

// ---------- Programa principal del lexer ----------

std::vector<Token> Lexer::tokenize() {
    while (true) {
        skipWhitespace();
        if (isAtEnd()) break;
        scanToken();
    }
    addToken(TokenType::END_OF_FILE, "", line, column);
    return tokens;
}

// Ignora espacios, tabs y saltos de línea
void Lexer::skipWhitespace() {
    while (!isAtEnd() && std::isspace((unsigned char)peek())) {
        advance();
    }
}

// Mira el primer carácter y decide qué tipo de token leer
void Lexer::scanToken() {
    int startLine = line;
    int startColumn = column;
    char c = peek();

    if (std::isalpha((unsigned char)c) || c == '_') {
        readIdentifier(startLine, startColumn);
    } else if (std::isdigit((unsigned char)c)) {
        readNumber(startLine, startColumn);
    } else if (c == '\'') {
        readString(startLine, startColumn);
    } else {
        readSymbol(startLine, startColumn);
    }
}

// ---------- Lectura de cada tipo de token ----------

// Lee una palabra y decide si es palabra reservada o identificador
void Lexer::readIdentifier(int startLine, int startColumn) {
    std::string text;
    while (std::isalnum((unsigned char)peek()) || peek() == '_') {
        text += advance();
    }

    // Se busca en minúscula para que SELECT, select y Select sean iguales
    std::string lower;
    for (char c : text) {
        lower += (char)std::tolower((unsigned char)c);
    }

    auto found = KEYWORDS.find(lower);
    if (found != KEYWORDS.end()) {
        addToken(found->second, text, startLine, startColumn);
    } else {
        addToken(TokenType::IDENTIFIER, text, startLine, startColumn);
    }
}

// Lee un entero (18) o un decimal (3.14)
void Lexer::readNumber(int startLine, int startColumn) {
    std::string text;
    while (std::isdigit((unsigned char)peek())) {
        text += advance();
    }

    // Si hay un punto, debe haber al menos un dígito después
    if (peek() == '.') {
        if (!std::isdigit((unsigned char)peekNext())) {
            text += advance();  // consume el punto para no leerlo dos veces
            addError("número decimal mal formado '" + text + "'", startLine, startColumn);
            return;
        }
        text += advance();  // el punto
        while (std::isdigit((unsigned char)peek())) {
            text += advance();
        }
        addToken(TokenType::DECIMAL, text, startLine, startColumn);
        return;
    }

    addToken(TokenType::INTEGER, text, startLine, startColumn);
}

// Lee una cadena entre comillas simples: 'hola'
void Lexer::readString(int startLine, int startColumn) {
    std::string text;
    text += advance();  // la comilla de apertura

    while (!isAtEnd() && peek() != '\'' && peek() != '\n') {
        text += advance();
    }

    // Si no encontramos la comilla de cierre, es un error
    if (peek() != '\'') {
        addError("cadena sin cerrar", startLine, startColumn);
        return;
    }

    text += advance();  // la comilla de cierre
    addToken(TokenType::STRING, text, startLine, startColumn);
}

// Lee operadores, comas, punto y coma. Si no reconoce el carácter, es error
void Lexer::readSymbol(int startLine, int startColumn) {
    char c = advance();

    switch (c) {
        case ',': addToken(TokenType::COMMA, ",", startLine, startColumn); break;
        case ';': addToken(TokenType::SEMICOLON, ";", startLine, startColumn); break;
        case '=': addToken(TokenType::EQUAL, "=", startLine, startColumn); break;

        // Los operadores de dos caracteres se revisan mirando el siguiente
        case '<':
            if (peek() == '=') {
                advance();
                addToken(TokenType::LESS_EQUAL, "<=", startLine, startColumn);
            } else if (peek() == '>') {
                advance();
                addToken(TokenType::NOT_EQUAL, "<>", startLine, startColumn);
            } else {
                addToken(TokenType::LESS, "<", startLine, startColumn);
            }
            break;

        case '>':
            if (peek() == '=') {
                advance();
                addToken(TokenType::GREATER_EQUAL, ">=", startLine, startColumn);
            } else {
                addToken(TokenType::GREATER, ">", startLine, startColumn);
            }
            break;

        case '!':
            if (peek() == '=') {
                advance();
                addToken(TokenType::NOT_EQUAL, "!=", startLine, startColumn);
            } else {
                addError("carácter no válido '!'", startLine, startColumn);
            }
            break;

        default:
            addError(std::string("carácter no válido '") + c + "'", startLine, startColumn);
            break;
    }
}

// ---------- Ayudas ----------

void Lexer::addToken(TokenType type, const std::string& lexeme, int startLine, int startColumn) {
    tokens.push_back(Token{type, lexeme, startLine, startColumn});
}

void Lexer::addError(const std::string& message, int startLine, int startColumn) {
    errors.push_back("Error léxico en línea " + std::to_string(startLine) +
                     ", columna " + std::to_string(startColumn) + ": " + message);
}
