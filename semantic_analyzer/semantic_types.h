#pragma once

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace semantic {

// Tipos de datos soportados por el catálogo y el verificador semántico
enum class DataType {
    INT,
    FLOAT,
    VARCHAR,
    BOOLEAN,
    UNKNOWN
};

inline std::string toLowerAscii(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

inline std::string dataTypeToString(DataType type) {
    switch (type) {
        case DataType::INT:     return "INT";
        case DataType::FLOAT:   return "FLOAT";
        case DataType::VARCHAR: return "VARCHAR";
        case DataType::BOOLEAN: return "BOOLEAN";
        case DataType::UNKNOWN: return "UNKNOWN";
    }
    return "UNKNOWN";
}

inline DataType stringToDataType(const std::string& raw) {
    std::string lower = toLowerAscii(raw);
    if (lower == "int" || lower == "integer") return DataType::INT;
    if (lower == "float" || lower == "decimal" || lower == "double") return DataType::FLOAT;
    if (lower == "varchar" || lower == "string" || lower == "text") return DataType::VARCHAR;
    if (lower == "boolean" || lower == "bool") return DataType::BOOLEAN;
    return DataType::UNKNOWN;
}

inline bool isNumericType(DataType type) {
    return type == DataType::INT || type == DataType::FLOAT;
}

// Metadatos de una columna (atributo) en el catálogo
struct ColumnMetadata {
    std::string name;
    std::string tableName;
    int ordinalPosition = 0; // 1-based ordinal position in table schema
    DataType type = DataType::UNKNOWN;
    bool isPrimaryKey = false;
    bool isNullable = true;
};

// Metadatos de una tabla en el catálogo
struct TableMetadata {
    std::string name;
    std::string storagePath; // Ruta o identificador lógico de almacenamiento
    std::vector<ColumnMetadata> orderedColumns;
    std::unordered_map<std::string, size_t> columnIndexByName; // clave en minúsculas -> índice en orderedColumns

    void addColumn(const ColumnMetadata& col) {
        ColumnMetadata copy = col;
        if (copy.tableName.empty()) {
            copy.tableName = name;
        }
        if (copy.ordinalPosition <= 0) {
            copy.ordinalPosition = static_cast<int>(orderedColumns.size()) + 1;
        }
        columnIndexByName[toLowerAscii(copy.name)] = orderedColumns.size();
        orderedColumns.push_back(copy);
    }
};

// Categorías de errores y advertencias semánticas
enum class SemanticErrorType {
    TABLE_NOT_FOUND,
    COLUMN_NOT_FOUND,
    DUPLICATE_COLUMN,
    TYPE_MISMATCH,
    INVALID_LOGICAL_OPERAND,
    INVALID_ARITHMETIC_OPERAND,
    LEXICAL_OR_SYNTAX_ERROR
};

inline std::string semanticErrorTypeToString(SemanticErrorType type) {
    switch (type) {
        case SemanticErrorType::TABLE_NOT_FOUND:            return "TABLE_NOT_FOUND";
        case SemanticErrorType::COLUMN_NOT_FOUND:           return "COLUMN_NOT_FOUND";
        case SemanticErrorType::DUPLICATE_COLUMN:           return "DUPLICATE_COLUMN";
        case SemanticErrorType::TYPE_MISMATCH:              return "TYPE_MISMATCH";
        case SemanticErrorType::INVALID_LOGICAL_OPERAND:    return "INVALID_LOGICAL_OPERAND";
        case SemanticErrorType::INVALID_ARITHMETIC_OPERAND: return "INVALID_ARITHMETIC_OPERAND";
        case SemanticErrorType::LEXICAL_OR_SYNTAX_ERROR:    return "LEXICAL_OR_SYNTAX_ERROR";
    }
    return "UNKNOWN_ERROR";
}

// Reporte estructurado de error o advertencia semántica
struct SemanticError {
    SemanticErrorType type = SemanticErrorType::TYPE_MISMATCH;
    std::string message;
    std::string context; // Identificador o expresión conflictiva
    int line = 0;
    int column = 0;
    bool isWarning = false;

    std::string format() const {
        std::ostringstream oss;
        oss << (isWarning ? "Advertencia semántica" : "Error semántico")
            << " [" << semanticErrorTypeToString(type) << "]";
        if (line > 0 || column > 0) {
            oss << " (línea " << line << ", columna " << column << ")";
        }
        if (!context.empty()) {
            oss << " en '" << context << "'";
        }
        oss << ": " << message;
        return oss.str();
    }
};

} // namespace semantic
