#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "parser.h"

void printUsage(const char* progName) {
    std::cout << "Uso:\n"
              << "  " << progName << " <archivo.sql>       Analiza el archivo SQL especificado y muestra la traza LL(1)\n"
              << "  " << progName << " --table             Muestra todas las reglas y la tabla de parseo LL(1)\n"
              << "  " << progName << "                     Modo interactivo por consola\n\n";
}

int main(int argc, char* argv[]) {
    parser_ll1::Parser parser;

    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        }

        if (arg == "--table") {
            parser.getTable().printTable();
            return 0;
        }

        // Leer archivo
        std::ifstream file(arg);
        if (!file.is_open()) {
            std::cerr << "Error: no se pudo abrir el archivo '" << arg << "'.\n";
            return 1;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string query = buffer.str();

        std::cout << "=================================================================\n";
        std::cout << " Analizador Sintáctico Predictivo LL(1) SQL - Práctica 5\n";
        std::cout << "=================================================================\n";
        std::cout << "Archivo a analizar: " << arg << "\n";
        std::cout << "Contenido de la consulta:\n" << query << "\n";

        auto result = parser.parse(query, true);
        return result.success ? 0 : 2;
    }

    // Modo interactivo
    std::cout << "=================================================================\n";
    std::cout << " Analizador Sintáctico Predictivo LL(1) SQL - Modo Interactivo\n";
    std::cout << " (Escriba 'salir' o 'exit' para terminar, o '--table' para ver reglas)\n";
    std::cout << "=================================================================\n\n";

    std::string line;
    while (true) {
        std::cout << "SQL> ";
        if (!std::getline(std::cin, line)) break;

        if (line.empty()) continue;
        if (line == "salir" || line == "exit" || line == "quit") break;
        if (line == "--table") {
            parser.getTable().printTable();
            continue;
        }

        parser.parse(line, true);
    }

    std::cout << "Programa finalizado.\n";
    return 0;
}
