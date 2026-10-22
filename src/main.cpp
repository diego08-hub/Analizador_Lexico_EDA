#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "lexer.h"

// Lee todo el texto de un archivo o de la entrada estándar.
// Devuelve false si no se pudo abrir el archivo.
bool readInput(int argc, char* argv[], std::string& output) {
    std::stringstream buffer;

    if (argc > 1) {
        std::ifstream file(argv[1]);
        if (!file) {
            std::cerr << "No se pudo abrir el archivo: " << argv[1] << "\n";
            return false;
        }
        buffer << file.rdbuf();
    } else {
        buffer << std::cin.rdbuf();
    }

    output = buffer.str();
    return true;
}

int main(int argc, char* argv[]) {
    std::string input;
    if (!readInput(argc, argv, input)) {
        return 1;
    }

    Lexer lexer(input);
    std::vector<Token> tokens = lexer.tokenize();

    // Tabla de tokens
    std::cout << std::left
              << std::setw(16) << "TIPO"
              << std::setw(20) << "LEXEMA"
              << "POSICION\n";
    for (const Token& t : tokens) {
        std::cout << std::setw(16) << tokenTypeToString(t.type)
                  << std::setw(20) << ("\"" + t.lexeme + "\"")
                  << std::setw(8) << (std::to_string(t.line) + ":" + std::to_string(t.column));

        // Los tokens ERROR explican qué pasó
        if (t.type == TokenType::ERROR) {
            std::cout << t.message;
        }
        std::cout << "\n";
    }

    // Resumen de errores léxicos (si hubo)
    if (!lexer.getErrors().empty()) {
        std::cout << "\nErrores léxicos (" << lexer.getErrors().size() << "):\n";
        for (const std::string& e : lexer.getErrors()) {
            std::cout << "  " << e << "\n";
        }
    }

    return 0;
}echo "SELECT @a FROM t;" | ./lexer