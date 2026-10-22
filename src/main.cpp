#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "lexer.h"

// Lee todo el texto de un archivo o de la entrada estándar
std::string readInput(int argc, char* argv[]) {
    std::stringstream buffer;

    if (argc > 1) {
        std::ifstream file(argv[1]);
        if (!file) {
            std::cerr << "No se pudo abrir el archivo: " << argv[1] << "\n";
            return "";
        }
        buffer << file.rdbuf();
    } else {
        buffer << std::cin.rdbuf();
    }

    return buffer.str();
}

int main(int argc, char* argv[]) {
    std::string input = readInput(argc, argv);

    Lexer lexer(input);
    std::vector<Token> tokens = lexer.tokenize();

    // Tabla de tokens
    std::cout << std::left
              << std::setw(16) << "TIPO"
              << std::setw(20) << "LEXEMA"
              << "POSICIÓN\n";
    for (const Token& t : tokens) {
        std::cout << std::setw(16) << tokenTypeToString(t.type)
                  << std::setw(20) << ("\"" + t.lexeme + "\"")
                  << t.line << ":" << t.column << "\n";
    }

    // Errores léxicos (si hubo)
    if (!lexer.getErrors().empty()) {
        std::cout << "\nErrores léxicos:\n";
        for (const std::string& e : lexer.getErrors()) {
            std::cout << "  " << e << "\n";
        }
    }

    return 0;
}
