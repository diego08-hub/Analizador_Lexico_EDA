#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "catalog.h"
#include "semantic_analyzer.h"

namespace {

void printUsage(const char* prog) {
    std::cout << "Uso:\n"
              << "  " << prog << " \"SELECT ... ;\"\n"
              << "  " << prog << " -f <archivo.sql>\n"
              << "  " << prog << " --catalog <schema.csv> \"SELECT ... ;\"\n"
              << "  " << prog << " (modo interactivo)\n";
}

int runQuery(const semantic::SemanticAnalyzer& analyzer, const std::string& query) {
    std::cout << "\nConsulta: " << query << "\n";
    semantic::SemanticResult result = analyzer.analyzeQuery(query, false);

    if (result.success) {
        result.printErrors(std::cout);
        result.printDecoratedAST(std::cout);
        return 0;
    } else {
        result.printErrors(std::cout);
        return 1;
    }
}

} // namespace

int main(int argc, char* argv[]) {
    semantic::CatalogManager catalog;
    std::string catalogPath;
    std::string sqlFilePath;
    std::string inlineQuery;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--catalog" && i + 1 < argc) {
            catalogPath = argv[++i];
        } else if (arg == "-f" && i + 1 < argc) {
            sqlFilePath = argv[++i];
        } else {
            inlineQuery = arg;
        }
    }

    if (!catalogPath.empty()) {
        if (!catalog.loadFromCSV(catalogPath)) {
            std::cerr << "Error: no se pudo cargar el catálogo desde '" << catalogPath << "'.\n";
            return 1;
        }
    } else {
        catalog.loadDefaultSchema();
    }

    semantic::SemanticAnalyzer analyzer(catalog, semantic::DuplicatePolicy::REJECT_ERROR);

    if (!sqlFilePath.empty()) {
        std::ifstream file(sqlFilePath);
        if (!file.is_open()) {
            std::cerr << "Error: no se pudo abrir el archivo '" << sqlFilePath << "'.\n";
            return 1;
        }
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return runQuery(analyzer, buffer.str());
    }

    if (!inlineQuery.empty()) {
        return runQuery(analyzer, inlineQuery);
    }

    // Modo interactivo
    std::cout << "=== Analizador Semántico SQL (Laboratorio 6) ===\n";
    std::cout << "Escribe una consulta SQL (terminada en ';') o 'exit' para salir.\n";
    std::string line;
    while (true) {
        std::cout << "\nsql> ";
        if (!std::getline(std::cin, line)) {
            break;
        }
        if (line == "exit" || line == "quit") {
            break;
        }
        if (line.empty()) {
            continue;
        }
        runQuery(analyzer, line);
    }

    return 0;
}
